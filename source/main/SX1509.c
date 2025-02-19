/*******************************************************
      SX1509 Procedures
*******************************************************/
//---------------------- Include Files ------------------------------------
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
#include "SX1509.h"

/* 
** Defines
*/
#define SX1509_IC2_ADDRESS           0x21
#define I2C_TIMEOUT_MS          	 10

// Class flags
#define SX1509_FLAG_PRESERVE_STATE   0x0001
#define SX1509_FLAG_DEVICE_PRESENT   0x1000
#define SX1509_FLAG_PINS_CONFD       0x2000
#define SX1509_FLAG_INITIALIZED      0x4000
#define SX1509_FLAG_FROM_BLOB        0x8000

#define SX1509_FLAG_SERIAL_MASK      0x000F  // Only these bits are serialized.


// These are the i2c register indicies. NOT their addresses.
enum
{
    SX1509_REG_INPUT_DISABLE,
    SX1509_REG_LONG_SLEW,
    SX1509_REG_LOW_DRIVE,
    SX1509_REG_PULLUP,
    SX1509_REG_PULLDOWN,
    SX1509_REG_OPEN_DRAIN,
    SX1509_REG_POLARITY,
    SX1509_REG_DIR,
    SX1509_REG_DATA,
    SX1509_REG_INTERRUPT_MASK,
    SX1509_REG_SENSE_HIGH,
    SX1509_REG_SENSE_LOW,
    SX1509_REG_INTERRUPT_SOURCE,
    SX1509_REG_EVENT_STATUS,
    SX1509_REG_LEVEL_SHIFTER,
    SX1509_REG_CLOCK,
    SX1509_REG_MISC,
    SX1509_REG_LED_DRIVER_ENABLE

    // more regs for keypad and led driver, don't need them
};


/*
** Static vars
*/
static uint16_t _flags = 0;
static uint8_t registers[31];
static const char *TAG = "app_SX1509";
static SemaphoreHandle_t I2CMutexHandle;
static i2c_port_t i2cnum;

// Real register addresses
static const uint8_t SX1509_REG_ADDR[18] = 
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11
};

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  none
* RETURN:      none
* NOTES:       none
*****************************************************************************/
static inline uint16_t _sx_flags(void) 
{ 
    return _flags;          
};

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  none
* RETURN:      none
* NOTES:       none
*****************************************************************************/
static inline uint8_t _sx_flag(uint16_t _flag) 
{ 
    return (_flags & _flag); 
};

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  none
* RETURN:      none
* NOTES:       none
*****************************************************************************/
static inline void _sx_clear_flag(uint16_t _flag) 
{ 
    _flags &= ~_flag;        
};

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  none
* RETURN:      none
* NOTES:       none
*****************************************************************************/
static inline void _sx_set_flag(uint16_t _flag) 
{   
    _flags |= _flag;         
};

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  none
* RETURN:      none
* NOTES:       none
*****************************************************************************/
static inline void _sx_set_flags(uint16_t _flag, uint8_t nu) 
{
    if (nu)
    {
        _flags |= _flag;
    }
    else
    {
        _flags &= ~_flag;
    }
};

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  none
* RETURN:      none
* NOTES:       none
*****************************************************************************/
static inline uint8_t initialized(void) 
{  
    return _sx_flag(SX1509_FLAG_INITIALIZED);  
};

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  none
* RETURN:      none
* NOTES:       none
*****************************************************************************/
static inline esp_err_t SX1509_ll_pin_init(void) 
{
    _sx_set_flag(SX1509_FLAG_PINS_CONFD);
    return ESP_OK;
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  none
* RETURN:      none
* NOTES:       none
*****************************************************************************/
static esp_err_t SX1509_write_register(uint8_t reg, uint8_t val) 
{
    esp_err_t ret = ESP_FAIL;
    uint8_t outbuffer[5];
  
    // build address at start of buffer
    outbuffer[0] = SX1509_REG_ADDR[reg];
    outbuffer[1] = val;

    // do the transfer
    if (xSemaphoreTake(I2CMutexHandle, pdMS_TO_TICKS(200)) == pdTRUE)
    {		
        if (i2c_master_write_to_device(i2cnum, SX1509_IC2_ADDRESS, outbuffer, 2, pdMS_TO_TICKS(I2C_TIMEOUT_MS)) != ESP_OK)
        {
            ESP_LOGE(TAG, "SX1509 write failed");
        }
        else
        {
            registers[reg] = val;
            ret = ESP_OK;
        }
        xSemaphoreGive(I2CMutexHandle);
    }

    return ret;
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  none
* RETURN:      none
* NOTES:       none
*****************************************************************************/
static esp_err_t SX1509_write_registers(uint8_t reg, uint8_t* buf, uint8_t len) 
{
    esp_err_t ret = ESP_FAIL;    
    uint8_t outbuffer[255];
  
    // build address at start of buffer
    outbuffer[0] = SX1509_REG_ADDR[reg];
    
    for (uint8_t i = 0; i < len; i++) 
    {
        outbuffer[i + 1] = buf[i];
    }

    // do the transfer
    if (xSemaphoreTake(I2CMutexHandle, pdMS_TO_TICKS(200)) == pdTRUE)
    {		
        if (i2c_master_write_to_device(i2cnum, SX1509_IC2_ADDRESS, outbuffer, len + 1, pdMS_TO_TICKS(I2C_TIMEOUT_MS)) != ESP_OK)
        {
            ESP_LOGE(TAG, "SX1509 writes failed");
        }
        else
        {
            ret = ESP_OK; 
        }
        xSemaphoreGive(I2CMutexHandle);
    }

    return ret;
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  none
* RETURN:      none
* NOTES:       none
*****************************************************************************/
static esp_err_t SX1509_read_register(uint8_t reg, uint8_t len) 
{  
    esp_err_t ret = ESP_FAIL;
    uint8_t outbuffer[5];
    uint8_t inbuffer[255];

    outbuffer[0] = SX1509_REG_ADDR[reg];

    // do the transfer
    if (xSemaphoreTake(I2CMutexHandle, pdMS_TO_TICKS(200)) == pdTRUE)
    {		
        if (i2c_master_write_read_device(i2cnum, SX1509_IC2_ADDRESS, outbuffer, 1, inbuffer, len, pdMS_TO_TICKS(I2C_TIMEOUT_MS)) != ESP_OK)
        {
            ESP_LOGE(TAG, "SX1509_read_register failed");
        }
        else
        {
            ret = ESP_OK;

            for (uint8_t i = 0; i < len; i++) 
            {
                registers[reg + i] = inbuffer[i];
            }
        }

        xSemaphoreGive(I2CMutexHandle);
    }

    return ret;
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  none
* RETURN:      none
* NOTES:       none
*****************************************************************************/
esp_err_t SX1509_Init(i2c_port_t i2c_num, SemaphoreHandle_t I2CMutex)
{ 
    esp_err_t ret = ESP_FAIL;
    uint8_t i;
    
    // save handles
    I2CMutexHandle = I2CMutex;
    i2cnum = i2c_num;

    for (i = 0; i < sizeof(registers); i++) 
    {
        registers[i] = 0;
    }

    // check if chip is present by reading
    ret = SX1509_read_register(SX1509_REG_INPUT_DISABLE, 1);

    if (ret == ESP_OK)
    {
        _sx_clear_flag(SX1509_FLAG_INITIALIZED);
        if (!_sx_flag(SX1509_FLAG_PINS_CONFD)) 
        {
            SX1509_ll_pin_init();
        }
    
        // load default config  
        ret = SX1509_reset();
    
        _sx_set_flags(SX1509_FLAG_INITIALIZED | SX1509_FLAG_DEVICE_PRESENT, (0 == ret));

        if (ret == ESP_OK)
        {
            ESP_LOGI(TAG, "SX1509 init OK\n");
        }    
    }
    else
    {
        ESP_LOGI(TAG, "SX1509 not detected\n");
    }

    return ret; 
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  none
* RETURN:      none
* NOTES:       none
*****************************************************************************/
esp_err_t SX1509_reset(void) 
{
    esp_err_t ret = ESP_FAIL;

    for (uint8_t i = 0; i < sizeof(registers); i++) 
    {
        registers[i] = 0;
    }
  
    // Steamroll the registers with the default values.
    uint8_t vals[18] = 
    {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
        0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00
    };

    ret = SX1509_write_registers(SX1509_REG_INPUT_DISABLE, &vals[0], 18);

    if (ret == ESP_OK) 
    {  
        ret = SX1509_refresh();   
    }
    else
    {
        ESP_LOGE(TAG,  "SX1509 reset failed\n");
    }

    return ret;
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  none
* RETURN:      none
* NOTES:       none
*****************************************************************************/
esp_err_t SX1509_refresh(void) 
{
    esp_err_t ret;

    ret = SX1509_read_register(SX1509_REG_INPUT_DISABLE, 18);
  
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG,  "SX1509 refresh failed\n");
    }

    return ret;
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  none
* RETURN:      none
* NOTES:       none
*****************************************************************************/
esp_err_t SX1509_digitalWrite(uint8_t pin, uint8_t value) 
{
    esp_err_t ret = ESP_FAIL;

    if (pin < 8) 
    {
        uint8_t reg0 = SX1509_REG_DATA;
        uint8_t val0 = registers[SX1509_REG_DATA];

        if (value > 0)
        {
            // set bit
            val0 |= (1 << pin);
        }
        else
        {
            // clear bit
            val0 &= ~(1 << pin);
        }

        ret = SX1509_write_register(reg0, val0);   
    }
  
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG,  "SX1509 digital write failed %d\n", ret);
    }

    return ret;
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  none
* RETURN:      none
* NOTES:       none
*****************************************************************************/
esp_err_t SX1509_digitalRead(uint8_t pin, uint8_t* value) 
{
    esp_err_t ret = ESP_FAIL;
    
    if (SX1509_read_register(SX1509_REG_DATA, 1) == ESP_OK)
    {
        if (pin < 8) 
        {
            *value = (registers[SX1509_REG_DATA] >> pin) & 0x01;
            ret = ESP_OK;
        }
    }
    
    return ret;
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  none
* RETURN:      none
* NOTES:       none
*****************************************************************************/
uint16_t SX1509_getPinValues(void) 
{
    uint16_t ret0 = (uint16_t)registers[SX1509_REG_DATA];
  
    return ret0;
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  none
* RETURN:      none
* NOTES:       none
*****************************************************************************/
esp_err_t SX1509_gpioMode(uint8_t pin, uint8_t mode) 
{
    esp_err_t ret = -1;
  
    if (pin < 8) 
    {
        uint8_t reg0 = SX1509_REG_DIR;
        uint8_t reg1 = SX1509_REG_PULLUP;
        uint8_t reg2 = SX1509_REG_PULLDOWN;
        uint8_t reg3 = SX1509_REG_INTERRUPT_MASK;

        uint8_t val0 = registers[reg0];
        uint8_t val1 = registers[reg1];
        uint8_t val2 = registers[reg2];
        uint8_t val3 = registers[reg3];

        switch (mode) 
        {
            case EXPANDER_OUTPUT_PULLDOWN:
            {
                // clear input bit
                val0 &= ~(1 << pin);

                // clear pullup bit
                val1 |= (1 << pin);

                // set pulldown bit
                val2 |= (1 << pin);                
            } break;

            case EXPANDER_OUTPUT_PULLUP:
            {
                // clear input bit
                val0 &= ~(1 << pin);

                // set pullup bit
                val1 |= (1 << pin);

                // clear pulldown bit
                val2 &= ~(1 << pin);    
            } break;

            case EXPANDER_OUTPUT:
            {
                // clear input bit
                val0 &= ~(1 << pin);

                // clear pullup bit
                val1 &= ~(1 << pin);

                // clear pulldown bit
                val2 &= ~(1 << pin);    
            } break;

            case EXPANDER_INPUT_PULLUP:
            {
                // set input bit
                val0 |= (1 << pin);

                // set pullup bit
                val1 |= (1 << pin);

                // clear pulldown bit
                val2 &= ~(1 << pin);    
            } break;

            case EXPANDER_INPUT_PULLDOWN:
            {
                // set input bit
                val0 |= (1 << pin);

                // clear pullup bit
                val1 &= ~(1 << pin);

                // set pulldown bit
                val2 |= (1 << pin);    
            } break;
            
            case EXPANDER_INPUT:
            default:
            {
                // set input bit
                val0 |= (1 << pin);

                // clear pullup bit
                val1 &= ~(1 << pin);

                // clear pulldown bit
                val2 &= ~(1 << pin);    
                break;
            }
        }

        ret = SX1509_write_register(reg0, val0);   
        ret = SX1509_write_register(reg1, val1);   
        ret = SX1509_write_register(reg2, val2);   
        ret = SX1509_write_register(reg3, val3);   
    }
  
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG,  "SX1509 gpio mode failed\n");
    }

    return ret;
}
