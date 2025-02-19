/*
 Copyright (C) 2024  Greg Smith

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
 
*/

#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "sys/param.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "main.h"
#include "CH422G.h"
#include "control.h"
#include "task_priorities.h"
#include "usb/usb_host.h"
#include "usb_comms.h"
#include "usb_tonex_one.h"
#include "leds.h"
#include "driver/i2c.h"
#include "SX1509.h"

#define FOOTSWITCH_TASK_STACK_SIZE          (3 * 1024)
#define FOOTSWITCH_SAMPLE_COUNT             5       // 20 msec per sample
#define BANK_MODE_BUTTONS                   4
#define BANK_MAXIMUM                        (MAX_PRESETS / BANK_MODE_BUTTONS)
#define BUTTON_FACTORY_RESET_TIME           500    // * 20 msec = 10 secs

enum FootswitchStates
{
    FOOTSWITCH_IDLE,
    FOOTSWITCH_WAIT_RELEASE_1,
    FOOTSWITCH_WAIT_RELEASE_2
};

enum FootswitchHandlers
{
    FOOTSWITCH_HANDLER_ONBOARD,
    FOOTSWITCH_HANDLER_EXTERNAL,
    FOOTSWITCH_HANDLER_MAX
};

static const char *TAG = "app_footswitches";

typedef struct
{
    uint8_t state;
    uint32_t sample_counter;
    uint8_t last_binary_val;
    uint8_t current_bank;
    uint8_t index_pending;
    uint8_t (*footswitch_reader)(uint8_t, uint8_t*);    
} tFootswitchHandler;

typedef struct
{
    tFootswitchHandler Handlers[FOOTSWITCH_HANDLER_MAX];
    uint8_t io_expander_ok;
} tFootswitchControl;

static tFootswitchControl FootswitchControl;
static SemaphoreHandle_t I2CMutexHandle;
static i2c_port_t i2cnum;

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  
* RETURN:      
* NOTES:       
*****************************************************************************/
static uint8_t read_footswitch_input_expander(uint8_t number, uint8_t* switch_state)
{
    uint8_t result = false;
    uint8_t level;
    uint8_t pin = 1;

    if (FootswitchControl.io_expander_ok)
    {       
        if (SX1509_digitalRead(pin, &level) == ESP_OK)
        {            
            // debug
            //ESP_LOGI(TAG, "Footswitch read %d", (int)level_mask);

            result = true;
            if (level == 0)
            {
                *switch_state = 0;
            }
            else
            {
                *switch_state = 1;
            }                
        }
    }

    return result;
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  
* RETURN:      
* NOTES:       
*****************************************************************************/
static uint8_t read_footswitch_input(uint8_t number, uint8_t* switch_state)
{
    uint8_t result = false;

#if CONFIG_TONEX_CONTROLLER_HARDWARE_PLATFORM_WAVESHARE_43B
    // display board uses I2C IO expander
    uint8_t value;

    if (CH422G_read_input(number, &value) == ESP_OK)
    {
        result = true;
        *switch_state = value;
    }
#else
    // other boards can use direct IO pin
    *switch_state = gpio_get_level(number);

    result = true;
#endif

    return result;
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  
* RETURN:      
* NOTES:       
*****************************************************************************/
static void footswitch_handle_dual_mode(tFootswitchHandler* handler)
{
    uint8_t value;   

    switch (handler->state)
    {
        case FOOTSWITCH_IDLE:
        default:
        {
            // read footswitches
            if (handler->footswitch_reader(FOOTSWITCH_1, &value)) 
            {
                if (value == 0)
                {
                    ESP_LOGI(TAG, "Footswitch 1 pressed");

                    // foot switch 1 pressed
                    control_request_preset_down();

                    // wait release	
                    handler->sample_counter = 0;
                    handler->state = FOOTSWITCH_WAIT_RELEASE_1;
                }
            }

            if (handler->state == FOOTSWITCH_IDLE)
            {
                if (handler->footswitch_reader(FOOTSWITCH_2, &value))
                {
                    if (value == 0)
                    {
                        ESP_LOGI(TAG, "Footswitch 2 pressed");

                        // foot switch 2 pressed, send event
                        control_request_preset_up();

                        // wait release	
                        handler->sample_counter = 0;
                        handler->state = FOOTSWITCH_WAIT_RELEASE_2;
                    }
                }
            }
        } break;

        case FOOTSWITCH_WAIT_RELEASE_1:
        {
            // read footswitch 1
            if (handler->footswitch_reader(FOOTSWITCH_1, &value))
            {
                if (value != 0)
                {
                    handler->sample_counter++;
                    if (handler->sample_counter == FOOTSWITCH_SAMPLE_COUNT)
                    {
                        // foot switch released
                        handler->state = FOOTSWITCH_IDLE;		
                    }
                }
                else
                {
                    // reset counter
                    handler->sample_counter = 0;
                }
            }
        } break;

        case FOOTSWITCH_WAIT_RELEASE_2:
        {
            // read footswitch 2
            if (handler->footswitch_reader(FOOTSWITCH_2, &value))
            {
                if (value != 0)
                {
                    handler->sample_counter++;
                    if (handler->sample_counter == FOOTSWITCH_SAMPLE_COUNT)
                    {
                        // foot switch released
                        handler->state = FOOTSWITCH_IDLE;
                    }                 
                }
                else
                {
                    // reset counter
                    handler->sample_counter = 0;
                }
            }
        } break;
    }
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  
* RETURN:      
* NOTES:       
*****************************************************************************/
static void footswitch_handle_quad_banked(tFootswitchHandler* handler)
{
    uint8_t value;
    uint8_t binary_val = 0;    

    // read all 4 switches (and swap so 1 is pressed)
    handler->footswitch_reader(FOOTSWITCH_1, &value);
    if (value == 0)
    {
        binary_val |= 1;
    }

    handler->footswitch_reader(FOOTSWITCH_2, &value);
    if (value == 0)
    {
        binary_val |= 2;
    }

    handler->footswitch_reader(FOOTSWITCH_3, &value);
    if (value == 0)
    {
        binary_val |= 4;
    }

    handler->footswitch_reader(FOOTSWITCH_4, &value);
    if (value == 0)
    {
        binary_val |= 8;
    }
    
    // handle state
    switch (handler->state)
    {
        case FOOTSWITCH_IDLE:
        {
            // any buttons pressed?
            if (binary_val != 0)
            {
                // check if A+B is pressed
                if (binary_val == 0x03)
                {
                    if (handler->current_bank > 0)
                    {
                        // bank down
                        handler->current_bank--;   
                        ESP_LOGI(TAG, "Footswitch banked down %d", handler->current_bank);
                    }

                    handler->state = FOOTSWITCH_WAIT_RELEASE_1;
                }
                // check if C+D is pressed
                else if (binary_val == 0x0C)
                {
                    if (handler->current_bank < BANK_MAXIMUM)
                    {
                        // bank up
                        handler->current_bank++;
                        ESP_LOGI(TAG, "Footswitch banked up %d", handler->current_bank);
                    }

                    handler->state = FOOTSWITCH_WAIT_RELEASE_1;
                }
                else
                {
                    // single button pressed, just store it. Preset select only happens on button release
                    handler->index_pending = binary_val;
                }
            }
            else
            {
                if (handler->index_pending != 0)
                {
                    uint8_t new_preset = handler->current_bank * BANK_MODE_BUTTONS;

                    // get the index from the bit set
                    if ((handler->index_pending & 0x01) != 0)
                    {
                        // nothing needed
                    }
                    else if ((handler->index_pending & 0x02) != 0)
                    {
                        new_preset += 1;
                    }
                    else if ((handler->index_pending & 0x04) != 0)
                    {
                        new_preset += 2;
                    }
                    else if ((handler->index_pending & 0x08) != 0)
                    {
                        new_preset += 3;
                    }
                    
                    // set the preset
                    control_request_preset_index(new_preset);
                    handler->index_pending = 0;

                    // give a little debounce time
                    vTaskDelay(pdMS_TO_TICKS(100));
                }
            }
        } break;

        case FOOTSWITCH_WAIT_RELEASE_1:
        {
            // check if all buttons released
            if (binary_val == 0)
            {
                handler->state = FOOTSWITCH_IDLE;
                handler->index_pending = 0;

                // give a little debounce time
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        } break;
    }
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  
* RETURN:      
* NOTES:       
*****************************************************************************/
static void footswitch_handle_quad_binary(tFootswitchHandler* handler)
{
    uint8_t value;
    uint8_t binary_val = 0;    

    // read all 4 switches (and swap so 1 is pressed)
    handler->footswitch_reader(FOOTSWITCH_1, &value);
    if (value == 0)
    {
        binary_val |= 1;
    }

    handler->footswitch_reader(FOOTSWITCH_2, &value);
    if (value == 0)
    {
        binary_val |= 2;
    }

    handler->footswitch_reader(FOOTSWITCH_3, &value);
    if (value == 0)
    {
        binary_val |= 4;
    }

    handler->footswitch_reader(FOOTSWITCH_4, &value);
    if (value == 0)
    {
        binary_val |= 8;
    }

    // has it changed?
    if (binary_val != handler->last_binary_val)
    {
        handler->last_binary_val = binary_val;

        // set preset
        control_request_preset_index(binary_val);

        ESP_LOGI(TAG, "Footswitch binary set preset %d", binary_val);
    }

    // wait a little longer, so we don't jump around presets while inputs are being set
    vTaskDelay(pdMS_TO_TICKS(180));
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  
* RETURN:      
* NOTES:       
*****************************************************************************/
void footswitch_task(void *arg)
{    
    uint8_t onboard_switch_mode;   
    uint8_t external_switch_mode;
    uint8_t value;
    uint32_t reset_timer = 0;

    ESP_LOGI(TAG, "Footswitch task start");

    // let things settle
    vTaskDelay(pdMS_TO_TICKS(1000));

#if CONFIG_TONEX_CONTROLLER_HARDWARE_PLATFORM_WAVESHARE_43B
    // 4.3B doesn't have enough IO, only supports dual mode
    onboard_switch_mode = FOOTSWITCH_MODE_DUAL_UP_DOWN;
#else
    // others, get the currently configured mode from web config
    onboard_switch_mode = control_get_config_footswitch_mode();
#endif


    // todo 
    external_switch_mode = FOOTSWITCH_MODE_QUAD_BANKED;


    // setup handler for onboard IO footswitches
    FootswitchControl.Handlers[FOOTSWITCH_HANDLER_ONBOARD].footswitch_reader = &read_footswitch_input;

    // setup handler for external IO Expander footswitches
    FootswitchControl.Handlers[FOOTSWITCH_HANDLER_EXTERNAL].footswitch_reader = &read_footswitch_input_expander;

    while (1)
    {
        // handle onboard IO foot switches (direct GPIO and IO expander on main PCB)
        switch (onboard_switch_mode) 
        {
            case FOOTSWITCH_MODE_DUAL_UP_DOWN:
            default:
            {
                // run dual mode next/previous
                footswitch_handle_dual_mode(&FootswitchControl.Handlers[FOOTSWITCH_HANDLER_ONBOARD]);
            } break;

            case FOOTSWITCH_MODE_QUAD_BANKED:
            {
                // run 4 switch with bank up/down
                footswitch_handle_quad_banked(&FootswitchControl.Handlers[FOOTSWITCH_HANDLER_ONBOARD]);
            } break;

            case FOOTSWITCH_MODE_QUAD_BINARY:
            {
                // run 4 switch binary mode
                footswitch_handle_quad_binary(&FootswitchControl.Handlers[FOOTSWITCH_HANDLER_ONBOARD]);
            } break;
        }

        // did we find an IO expander on boot?
        if (FootswitchControl.io_expander_ok)
        {
            switch (external_switch_mode) 
            {
                case FOOTSWITCH_MODE_DUAL_UP_DOWN:
                default:
                {
                    // run dual mode next/previous
                    footswitch_handle_dual_mode(&FootswitchControl.Handlers[FOOTSWITCH_HANDLER_EXTERNAL]);
                } break;
    
                case FOOTSWITCH_MODE_QUAD_BANKED:
                {
                    // run 4 switch with bank up/down
                    footswitch_handle_quad_banked(&FootswitchControl.Handlers[FOOTSWITCH_HANDLER_EXTERNAL]);
                } break;
    
                case FOOTSWITCH_MODE_QUAD_BINARY:
                {
                    // run 4 switch binary mode
                    footswitch_handle_quad_binary(&FootswitchControl.Handlers[FOOTSWITCH_HANDLER_EXTERNAL]);
                } break;
            }
        }

        // check for button held for data reset
        if (read_footswitch_input(FOOTSWITCH_1, &value))
        {
            if (value == 0)
            {        
                reset_timer++;

                if (reset_timer > BUTTON_FACTORY_RESET_TIME)
                {
                    ESP_LOGI(TAG, "Config Reset to default");  
                    control_set_default_config(); 

                    // save and reboot
                    control_save_user_data(1);
                }
            }
            else
            {
                reset_timer = 0;
            }
        }

        // handle leds from this task, to save wasting ram on another task for it
        leds_handle();

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  
* RETURN:      
* NOTES:       
*****************************************************************************/
void footswitches_init(i2c_port_t i2c_num, SemaphoreHandle_t I2CMutex)
{	
    memset((void*)&FootswitchControl, 0, sizeof(FootswitchControl));

    // save handles
    I2CMutexHandle = I2CMutex;
	i2cnum = i2c_num;

#if CONFIG_TONEX_CONTROLLER_GPIO_FOOTSWITCHES
    // init GPIO
    gpio_config_t gpio_config_struct;

    gpio_config_struct.pin_bit_mask = (((uint64_t)1 << FOOTSWITCH_1) | ((uint64_t)1 << FOOTSWITCH_2) | ((uint64_t)1 << FOOTSWITCH_3) | ((uint64_t)1 << FOOTSWITCH_4));
    gpio_config_struct.mode = GPIO_MODE_INPUT;
    gpio_config_struct.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config_struct.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config_struct.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&gpio_config_struct);
#endif

    // try to init I2C IO expander
    if (SX1509_Init(i2c_num, I2CMutex) == ESP_OK)
    {
        ESP_LOGI(TAG, "Found External IO Expander");

        // init all pins to inputs
        for (uint8_t pins = 0; pins < 8; pins++)
        {
            SX1509_gpioMode(1 << pins, EXPANDER_INPUT_PULLUP);
        }
        FootswitchControl.io_expander_ok = 1;
    }
    else
    {
        ESP_LOGI(TAG, "External IO Expander not found");
    }

    // init leds
    leds_init();

    // create task
    xTaskCreatePinnedToCore(footswitch_task, "FOOT", FOOTSWITCH_TASK_STACK_SIZE, NULL, FOOTSWITCH_TASK_PRIORITY, NULL, 1);
}
