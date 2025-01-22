/*
 Copyright (C) 2025  Greg Smith

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "usb/usb_host.h"
#include "usb/cdc_acm_host.h"
#include "driver/i2c.h"
#include "usb_comms.h"
#include "usb_tonex_one.h"
#include "control.h"
#include "tonex_params.h"


#define PARAM_MUTEX_TIMEOUT         2000        // msec

static const char *TAG = "app_ToneParams";


/*
** Static vars
*/
static SemaphoreHandle_t ParamMutex;
static tTonexParameter* TonexParameters;

#if 0
// "value" below is just a default, is overridden by the preset on load
static tTonexParameter TonexParameters[TONEX_PARAM_LAST] = 
{
    //value, Min,    Max,  Name
    {0,      0,      1,    "NG POST"},            // TONEX_PARAM_NOISE_GATE_POST   
    {1,      0,      1,    "NG POWER"},           // TONEX_PARAM_NOISE_GATE_ENABLE,
    {-64,    -100,   0,    "NG THRESH"},          // TONEX_PARAM_NOISE_GATE_THRESHOLD,
    {20,     5,      500,  "NG REL"},             // TONEX_PARAM_NOISE_GATE_RELEASE,
    {-60,    -100,   -20,  "NG DEPTH"},           // TONEX_PARAM_NOISE_GATE_DEPTH,

    // Compressor
    {1,      0,      1,    "COMP POST"},           // TONEX_PARAM_COMP_POST,             
    {0,      0,      1,    "COMP POWER"},          // TONEX_PARAM_COMP_ENABLE,
    {-14,    -40,    0,    "COMP THRESH"},         // TONEX_PARAM_COMP_THRESHOLD,
    {-12,    -30,    10,   "COMP GAIN"},           // TONEX_PARAM_COMP_MAKE_UP,
    {14,     1,      51,   "COMP ATTACK"},          // TONEX_PARAM_COMP_ATTACK,

    // EQ    
    {0,      0,      1,    "EQ POST"},             // TONEX_PARAM_EQ_POST,                // Pre/Post
    {5,      0,      10,   "EQ BASS"},             // TONEX_PARAM_EQ_BASS,
    {300,    75,     600,  "EQ BFREQ"},            // TONEX_PARAM_EQ_BASS_FREQ,
    {5,      0,      10,   "EQ MID"},              // TONEX_PARAM_EQ_MID,
    {0.7,    0.2,    3.0,  "EQ MIDQ"},             // TONEX_PARAM_EQ_MIDQ,
    {750,    150,    500,  "EQ MFREQ"},            // TONEX_PARAM_EQ_MID_FREQ,
    {5,      0,      10,   "EQ TREBLE"},           // TONEX_PARAM_EQ_TREBLE,
    {1900,   1000,   4000, "EQ TFREQ"},            // TONEX_PARAM_EQ_TREBLE_FREQ,
    
    // Model and VIR
    {0,      0,      1,    "UNK 1"},              // TONEX_PARAM_UNKNOWN_1,
    {0,      0,      1,    "UNK 2"},              // TONEX_PARAM_UNKNOWN_2,
    {5,      0,      10,   "MDL GAIN"},           // TONEX_PARAM_MODEL_GAIN,
    {5,      0,      10,   "MDL VOL"},            // TONEX_PARAM_MODEL_VOLUME,
    {100,    0,      100,  "MDL MIX"},            // TONEX_PARAM_MODEX_MIX,
    {0,      0,      0,    "UNK 3"},              // TONEX_PARAM_UNKNOWN_3,   
    {5,      0,      10,   "MOD PRES"},           // TONEX_PARAM_PRESENCE,
    {5,      0,      10,   "MOD DEPTH"},          // TONEX_PARAM_DEPTH,
    {0,      0,      10,   "VIR_RESO"},           // TONEX_PARAM_VIR_RESO,
    {0,      0,      2,    "VIR_M1"},             // TONEX_PARAM_VIR_MIC_1,
    {0,      0,      10,   "VIR_M1X"},            // TONEX_PARAM_VIR_MIC_1_X,
    {0,      0,      10,   "VIR_M1Y"},            // TONEX_PARAM_VIR_MIC_1_Y,
    {0,      0,      10,   "VIR_M1Z"},            // TONEX_PARAM_VIR_MIC_1_Z,
    {0,      0,      2,    "VIR_M2"},             // TONEX_PARAM_VIR_MIC_2,
    {0,      0,      10,   "VIR_M2X"},            // TONEX_PARAM_VIR_MIC_2_X,
    {0,      0,      10,   "VIR_M2Y"},            // TONEX_PARAM_VIR_MIC_2_Y,
    {0,      0,      10,   "VIR_M2Z"},            // TONEX_PARAM_VIR_MIC_2_Z,
    {0,      -100,   100,  "VIR_BLEND"},          // TONEX_PARAM_VIR_BLEND,
    
    // Reverb
    {0,      0,      1,    "RVB POS"},             // TONEX_PARAM_REVERB_POSITION,
    {1,      0,      1,    "RVB POWER"},           // TONEX_PARAM_REVERB_ENABLE,
    {0,      0,      5,    "RVB MODEL"},           // TONEX_PARAM_REVERB_MODEL,
    {5,      0,      10,   "RVB S1 T"},            // TONEX_PARAM_REVERB_SPRING1_TIME,
    {0,      0,      500,  "RVB S1 P"},            // TONEX_PARAM_REVERB_SPRING1_PREDELAY,
    {0,      -10,    10,   "RVB S1 C"},            // TONEX_PARAM_REVERB_SPRING1_COLOR,
    {0,      0,      100,  "RVB S1 M"},            // TONEX_PARAM_REVERB_SPRING1_MIX,
    {5,      0,      10,   "RVB S2 T"},            // TONEX_PARAM_REVERB_SPRING2_TIME,
    {0,      0,      500,  "RVB S2 P"},            // TONEX_PARAM_REVERB_SPRING2_PREDELAY,
    {0,      -10,    10,   "RVB S2 C"},            // TONEX_PARAM_REVERB_SPRING2_COLOR,
    {0,      0,      100,  "RVB S2 M"},            // TONEX_PARAM_REVERB_SPRING2_MIX,
    {5,      0,      10,   "RVB S3 T"},            // TONEX_PARAM_REVERB_SPRING3_TIME,
    {0,      0,      500,  "RVB S3 P"},            // TONEX_PARAM_REVERB_SPRING3_PREDELAY,
    {0,      -10,    10,   "RVB S3 C"},            // TONEX_PARAM_REVERB_SPRING3_COLOR,
    {0,      0,      100,  "RVB S3 M"},            // TONEX_PARAM_REVERB_SPRING3_MIX,
    {5,      0,      10,   "RVB S4 T"},            // TONEX_PARAM_REVERB_SPRING4_TIME,
    {0,      0,      500,  "RVB S4 P"},            // TONEX_PARAM_REVERB_SPRING4_PREDELAY,
    {0,      -10,    10,   "RVB S4 C"},            // TONEX_PARAM_REVERB_SPRING4_COLOR,
    {0,      0,      100,  "RVB S4 M"},            // TONEX_PARAM_REVERB_SPRING4_MIX,
    {5,      0,      10,   "RVB RM T"},            // TONEX_PARAM_REVERB_ROOM_TIME,
    {0,      0,      500,  "RVB RM P"},            // TONEX_PARAM_REVERB_ROOM_PREDELAY,
    {0,      -10,    10,   "RVB RM C"},            // TONEX_PARAM_REVERB_ROOM_COLOR,
    {0,      0,      100,  "RVB RM M"},            // TONEX_PARAM_REVERB_ROOM_MIX,
    {5,      0,      10,   "RVB PL T"},            // TONEX_PARAM_REVERB_PLATE_TIME,
    {0,      0,      500,  "RVB PL P"},            // TONEX_PARAM_REVERB_PLATE_PREDELAY,
    {0,      -10,    10,   "RVB PL C"},            // TONEX_PARAM_REVERB_PLATE_COLOR,
    {0,      0,      100,  "RVB PL M"},            // TONEX_PARAM_REVERB_PLATE_MIX,

    // Modulation
    {0,      0,      1,    "MOD POST"},            // TONEX_PARAM_MODULATION_POST,
    {0,      0,      1,    "MOD POWER"},            // TONEX_PARAM_MODULATION_ENABLE,
    {0,      0,      4,    "MOD MODEL"},            // TONEX_PARAM_MODULATION_MODEL,
    {0,      0,      1,    "MOD CH S"},            // TONEX_PARAM_MODULATION_CHORUS_SYNC,
    {0,      0,      1,    "MOD CH T"},            // TONEX_PARAM_MODULATION_CHORUS_TS,
    {0.5,    0.1,    10,   "MOD CH R"},            // TONEX_PARAM_MODULATION_CHORUS_RATE,
    {0,      0,      100,  "MOD CH D"},            // TONEX_PARAM_MODULATION_CHORUS_DEPTH,
    {0,      0,      10,   "MOD CH L"},            // TONEX_PARAM_MODULATION_CHORUS_LEVEL,
    {0,      0,      1,    "MOD TR S"},            // TONEX_PARAM_MODULATION_TREMOLO_SYNC,
    {0,      0,      1,    "MOD TR T"},            // TONEX_PARAM_MODULATION_TREMOLO_TS,
    {0.5,    0.1,    10,   "MOD TR R"},            // TONEX_PARAM_MODULATION_TREMOLO_RATE,
    {0,      0,      10,   "MOD TR P"},            // TONEX_PARAM_MODULATION_TREMOLO_SHAPE,
    {0,      0,      100,  "MOD TR D"},            // TONEX_PARAM_MODULATION_TREMOLO_SPREAD,
    {0,      0,      10,   "MOD TR L"},            // TONEX_PARAM_MODULATION_TREMOLO_LEVEL,
    {0,      0,      1,    "MOD PH S"},            // TONEX_PARAM_PHASER_SYNC,
    {0,      0,      1,    "MOD PH T"},            // TONEX_PARAM_PHASER_TS,
    {0.5,    0.1,    10,   "MOD PH R"},            // TONEX_PARAM_PHASER_RATE,
    {0,      0,      100,  "MOD PH D"},            // TONEX_PARAM_PHASER_DEPTH,
    {0,      0,      10,   "MOD PH L"},            // TONEX_PARAM_PHASER_LEVEL,
    {0,      0,      1,    "MOD FL S"},            // TONEX_PARAM_FLANGER_SYNC,
    {0,      0,      1,    "MOD FL T"},            // TONEX_PARAM_FLANGER_TS,
    {0.5,    0.1,    10,   "MOD FL R"},            // TONEX_PARAM_FLANGER_RATE,
    {0,      0,      100,  "MOD FL D"},            // TONEX_PARAM_FLANGER_DEPTH,
    {0,      0,      100,  "MOD FL F"},            // TONEX_PARAM_FLANGER_FEEDEBACK,
    {0,      0,      10,   "MOD FL L"},            // TONEX_PARAM_FLANGER_LEVEL,
    {0,      0,      1,    "MOD RO S"},            // TONEX_PARAM_ROTARY_SYNC,
    {0,      0,      1,    "MOD RO T"},            // TONEX_PARAM_ROTARY_TS,
    {0,      0,      400,  "MOD RO S"},            // TONEX_PARAM_ROTARY_SPEED,
    {0,      0,      300,  "MOD RO R"},            // TONEX_PARAM_ROTARY_RADIUS,
    {0,      0,      100,  "MOD RO D"},            // TONEX_PARAM_ROTARY_SPREAD,
    {0,      0,      10,   "MOD RO L"},            // TONEX_PARAM_ROTARY_LEVEL,
    
    // Delay
    {0,      0,      1,    "DLY POST"},            // TONEX_PARAM_DELAY_POST,    
    {0,      0,      1,    "DLY POWER"},           // TONEX_PARAM_DELAY_ENABLE,
    {0,      0,      1,    "DLY MODEL"},           // TONEX_PARAM_DELAY_MODEL,
    {0,      0,      1,    "DLY DG S"},            // TONEX_PARAM_DELAY_DIGITAL_SYNC,
    {0,      0,      1,    "DLY DG T"},            // TONEX_PARAM_DELAY_DIGITAL_TS,
    {0,      0,      1000, "DLY DT M"},            // TONEX_PARAM_DELAY_DIGITAL_TIME,
    {0,      0,      100,  "DLY DT F"},            // TONEX_PARAM_DELAY_DIGITAL_FEEDBACK,
    {0,      0,      1,    "DLY DT O"},            // TONEX_PARAM_DELAY_DIGITAL_MODE,
    {0,      0,      100,  "DLY DT X"},            // TONEX_PARAM_DELAY_DIGITAL_MIX,
    {0,      0,      1,    "DLY TA S"},            // TONEX_PARAM_DELAY_TAPE_SYNC,
    {0,      0,      1,    "DLY TA T"},            // TONEX_PARAM_DELAY_TAPE_TS,
    {0,      0,      1000, "DLY TA M"},            // TONEX_PARAM_DELAY_TAPE_TIME,
    {0,      0,      100,  "DLY TA F"},            // TONEX_PARAM_DELAY_TAPE_FEEDBACK,
    {0,      0,      1,    "DLY TA O"},            // TONEX_PARAM_DELAY_TAPE_MODE,
    {0,      0,      100,  "DLY TA X"},            // TONEX_PARAM_DELAY_TAPE_MIX,    
};
#endif

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  
* RETURN:      
* NOTES:       
*****************************************************************************/
esp_err_t tonex_params_get_locked_access(tTonexParameter** param_ptr)
{
    // take mutex
    if (xSemaphoreTake(ParamMutex, pdMS_TO_TICKS(PARAM_MUTEX_TIMEOUT)) == pdTRUE)
    {		
        *param_ptr = TonexParameters;
        return ESP_OK;
    }
    else
    {
        ESP_LOGE(TAG, "tonex_params_get_locked_access Mutex timeout!");   
    }

    return ESP_FAIL;
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  
* RETURN:      
* NOTES:       
*****************************************************************************/
esp_err_t tonex_params_release_locked_access(void)
{
    // release mutex
    xSemaphoreGive(ParamMutex);

    return ESP_OK;
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  
* RETURN:      
* NOTES:       
*****************************************************************************/
esp_err_t tonex_params_get_min_max(uint16_t param_index, float* min, float* max)
{
    if (param_index >= TONEX_PARAM_LAST)
    {
        // invalid
        return ESP_FAIL;
    }

    // take mutex
    if (xSemaphoreTake(ParamMutex, pdMS_TO_TICKS(PARAM_MUTEX_TIMEOUT)) == pdTRUE)
    {		
        *min = TonexParameters[param_index].Min;
        *max = TonexParameters[param_index].Max;

        // release mutex
        xSemaphoreGive(ParamMutex);

        return ESP_OK;
    }
    else
    {
        ESP_LOGE(TAG, "tonex_params_get_min_max Mutex timeout!");   
    }

    return ESP_FAIL;
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  
* RETURN:      
* NOTES:       
*****************************************************************************/
esp_err_t tonex_params_init(void)
{
    // create mutex to protect interprocess issues with memory sharing
    ParamMutex = xSemaphoreCreateMutex();
    if (ParamMutex == NULL)
    {
        ESP_LOGE(TAG, "ParamMutex Mutex create failed!");
        return ESP_FAIL;
    }

    // allocate param storage in PSRAM
    TonexParameters = heap_caps_malloc(sizeof(tTonexParameter) * TONEX_PARAM_LAST, MALLOC_CAP_SPIRAM);
    if (TonexParameters == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate TonexParameters!");
        return ESP_FAIL;
    }

    // clear mem
    memset((void*)TonexParameters, 0, sizeof(tTonexParameter) * TONEX_PARAM_LAST);

    // init params    
    // note: this took hours to type. Would have preferred to init the struct on defintion
    // but being about 2.5KB it takes up too much ram that way. PSRAM allocation so harder init :(
    TonexParameters[TONEX_PARAM_NOISE_GATE_POST].Value = 0;
    TonexParameters[TONEX_PARAM_NOISE_GATE_POST].Min = 0;
    TonexParameters[TONEX_PARAM_NOISE_GATE_POST].Max = 1;
    sprintf(TonexParameters[TONEX_PARAM_NOISE_GATE_POST].Name, "NG POST");

    TonexParameters[TONEX_PARAM_NOISE_GATE_ENABLE].Value = 1;
    TonexParameters[TONEX_PARAM_NOISE_GATE_ENABLE].Min = 0;
    TonexParameters[TONEX_PARAM_NOISE_GATE_ENABLE].Max = 1;
    sprintf(TonexParameters[TONEX_PARAM_NOISE_GATE_ENABLE].Name, "NG POWER");

    TonexParameters[TONEX_PARAM_NOISE_GATE_THRESHOLD].Value = -64;
    TonexParameters[TONEX_PARAM_NOISE_GATE_THRESHOLD].Min = -100;
    TonexParameters[TONEX_PARAM_NOISE_GATE_THRESHOLD].Max = 0;
    sprintf(TonexParameters[TONEX_PARAM_NOISE_GATE_THRESHOLD].Name, "NG THRESH");

    TonexParameters[TONEX_PARAM_NOISE_GATE_RELEASE].Value = 20;
    TonexParameters[TONEX_PARAM_NOISE_GATE_RELEASE].Min = 5;
    TonexParameters[TONEX_PARAM_NOISE_GATE_RELEASE].Max = 500;
    sprintf(TonexParameters[TONEX_PARAM_NOISE_GATE_RELEASE].Name, "NG REL");

    TonexParameters[TONEX_PARAM_NOISE_GATE_DEPTH].Value = -60;
    TonexParameters[TONEX_PARAM_NOISE_GATE_DEPTH].Min = -100;
    TonexParameters[TONEX_PARAM_NOISE_GATE_DEPTH].Max =-20 ;
    sprintf(TonexParameters[TONEX_PARAM_NOISE_GATE_DEPTH].Name, "NG DEPTH");

    // Compressor
    TonexParameters[TONEX_PARAM_COMP_POST].Value = 1;
    TonexParameters[TONEX_PARAM_COMP_POST].Min = 0;
    TonexParameters[TONEX_PARAM_COMP_POST].Max = 1;
    sprintf(TonexParameters[TONEX_PARAM_COMP_POST].Name, "COMP POST");
 
    TonexParameters[TONEX_PARAM_COMP_ENABLE].Value = 0;
    TonexParameters[TONEX_PARAM_COMP_ENABLE].Min = 0;
    TonexParameters[TONEX_PARAM_COMP_ENABLE].Max = 1;
    sprintf(TonexParameters[TONEX_PARAM_COMP_ENABLE].Name, "COMP POWER");    

    TonexParameters[TONEX_PARAM_COMP_THRESHOLD].Value = -14;
    TonexParameters[TONEX_PARAM_COMP_THRESHOLD].Min = -40;
    TonexParameters[TONEX_PARAM_COMP_THRESHOLD].Max = 0;
    sprintf(TonexParameters[TONEX_PARAM_COMP_THRESHOLD].Name, "COMP THRESH");

    TonexParameters[TONEX_PARAM_COMP_MAKE_UP].Value = -12;
    TonexParameters[TONEX_PARAM_COMP_MAKE_UP].Min = -30;
    TonexParameters[TONEX_PARAM_COMP_MAKE_UP].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_COMP_MAKE_UP].Name, "COMP GAIN");

    TonexParameters[TONEX_PARAM_COMP_ATTACK].Value = 14;
    TonexParameters[TONEX_PARAM_COMP_ATTACK].Min = 1;
    TonexParameters[TONEX_PARAM_COMP_ATTACK].Max = 51;
    sprintf(TonexParameters[TONEX_PARAM_COMP_ATTACK].Name, "COMP ATTACK");

    // EQ    
    TonexParameters[TONEX_PARAM_EQ_POST].Value = 0;
    TonexParameters[TONEX_PARAM_EQ_POST].Min = 0;
    TonexParameters[TONEX_PARAM_EQ_POST].Max = 1;
    sprintf(TonexParameters[TONEX_PARAM_EQ_POST].Name, "EQ POST");
    
    TonexParameters[TONEX_PARAM_EQ_BASS].Value = 5;
    TonexParameters[TONEX_PARAM_EQ_BASS].Min = 0;
    TonexParameters[TONEX_PARAM_EQ_BASS].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_EQ_BASS].Name, "EQ BASS");

    TonexParameters[TONEX_PARAM_EQ_BASS_FREQ].Value = 300;
    TonexParameters[TONEX_PARAM_EQ_BASS_FREQ].Min = 75;
    TonexParameters[TONEX_PARAM_EQ_BASS_FREQ].Max = 600;
    sprintf(TonexParameters[TONEX_PARAM_EQ_BASS_FREQ].Name, "EQ BFREQ");

    TonexParameters[TONEX_PARAM_EQ_MID].Value = 5;
    TonexParameters[TONEX_PARAM_EQ_MID].Min = 0;
    TonexParameters[TONEX_PARAM_EQ_MID].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_EQ_MID].Name, "EQ MID");

    TonexParameters[TONEX_PARAM_EQ_MIDQ].Value = 0.7;
    TonexParameters[TONEX_PARAM_EQ_MIDQ].Min = 0.2;
    TonexParameters[TONEX_PARAM_EQ_MIDQ].Max = 3.0;
    sprintf(TonexParameters[TONEX_PARAM_EQ_MIDQ].Name, "EQ MIDQ");

    TonexParameters[TONEX_PARAM_EQ_MID_FREQ].Value = 750;
    TonexParameters[TONEX_PARAM_EQ_MID_FREQ].Min = 150;
    TonexParameters[TONEX_PARAM_EQ_MID_FREQ].Max = 500;
    sprintf(TonexParameters[TONEX_PARAM_EQ_MID_FREQ].Name, "EQ MFREQ");

    TonexParameters[TONEX_PARAM_EQ_TREBLE].Value = 5;
    TonexParameters[TONEX_PARAM_EQ_TREBLE].Min = 0;
    TonexParameters[TONEX_PARAM_EQ_TREBLE].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_EQ_TREBLE].Name, "EQ TREBLE");

    TonexParameters[TONEX_PARAM_EQ_TREBLE_FREQ].Value = 1900;
    TonexParameters[TONEX_PARAM_EQ_TREBLE_FREQ].Min = 1000;
    TonexParameters[TONEX_PARAM_EQ_TREBLE_FREQ].Max = 4000;
    sprintf(TonexParameters[TONEX_PARAM_EQ_TREBLE_FREQ].Name, "EQ TFREQ");

    // Model and VIR
    TonexParameters[TONEX_PARAM_UNKNOWN_1].Value = 0;
    TonexParameters[TONEX_PARAM_UNKNOWN_1].Min = 0;
    TonexParameters[TONEX_PARAM_UNKNOWN_1].Max = 1;
    sprintf(TonexParameters[TONEX_PARAM_UNKNOWN_1].Name, "UNK 1");

    TonexParameters[TONEX_PARAM_UNKNOWN_2].Value = 0;
    TonexParameters[TONEX_PARAM_UNKNOWN_2].Min = 0;
    TonexParameters[TONEX_PARAM_UNKNOWN_2].Max = 1;
    sprintf(TonexParameters[TONEX_PARAM_UNKNOWN_2].Name, "UNK 2");

    TonexParameters[TONEX_PARAM_MODEL_GAIN].Value = 5;
    TonexParameters[TONEX_PARAM_MODEL_GAIN].Min = 0;
    TonexParameters[TONEX_PARAM_MODEL_GAIN].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_MODEL_GAIN].Name, "MDL GAIN");

    TonexParameters[TONEX_PARAM_MODEL_VOLUME].Value = 5;
    TonexParameters[TONEX_PARAM_MODEL_VOLUME].Min = 0;
    TonexParameters[TONEX_PARAM_MODEL_VOLUME].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_MODEL_VOLUME].Name, "MDL VOL");

    TonexParameters[TONEX_PARAM_MODEX_MIX].Value = 100;
    TonexParameters[TONEX_PARAM_MODEX_MIX].Min = 0;
    TonexParameters[TONEX_PARAM_MODEX_MIX].Max = 100;
    sprintf(TonexParameters[TONEX_PARAM_MODEX_MIX].Name, "MDL MIX");

    TonexParameters[TONEX_PARAM_UNKNOWN_3].Value = 0;
    TonexParameters[TONEX_PARAM_UNKNOWN_3].Min = 0;
    TonexParameters[TONEX_PARAM_UNKNOWN_3].Max = 0;
    sprintf(TonexParameters[TONEX_PARAM_UNKNOWN_3].Name, "UNK 3");

    TonexParameters[TONEX_PARAM_PRESENCE].Value = 5;
    TonexParameters[TONEX_PARAM_PRESENCE].Min = 0;
    TonexParameters[TONEX_PARAM_PRESENCE].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_PRESENCE].Name, "MOD PRES");

    TonexParameters[TONEX_PARAM_DEPTH].Value = 5;
    TonexParameters[TONEX_PARAM_DEPTH].Min = 0;
    TonexParameters[TONEX_PARAM_DEPTH].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_DEPTH].Name, "MOD DEPTH");

    TonexParameters[TONEX_PARAM_VIR_RESO].Value = 0;
    TonexParameters[TONEX_PARAM_VIR_RESO].Min = 0;
    TonexParameters[TONEX_PARAM_VIR_RESO].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_VIR_RESO].Name, "VIR RESO");

    TonexParameters[TONEX_PARAM_VIR_MIC_1].Value = 0;
    TonexParameters[TONEX_PARAM_VIR_MIC_1].Min = 0;
    TonexParameters[TONEX_PARAM_VIR_MIC_1].Max = 2;
    sprintf(TonexParameters[TONEX_PARAM_VIR_MIC_1].Name, "VIR M1");

    TonexParameters[TONEX_PARAM_VIR_MIC_1_X].Value = 0;
    TonexParameters[TONEX_PARAM_VIR_MIC_1_X].Min = 0;
    TonexParameters[TONEX_PARAM_VIR_MIC_1_X].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_VIR_MIC_1_X].Name, "VIR M1X");

    TonexParameters[TONEX_PARAM_VIR_MIC_1_Y].Value = 0;
    TonexParameters[TONEX_PARAM_VIR_MIC_1_Y].Min = 0;
    TonexParameters[TONEX_PARAM_VIR_MIC_1_Y].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_VIR_MIC_1_Y].Name, "VIR M1Y");

    TonexParameters[TONEX_PARAM_VIR_MIC_1_Z].Value = 0;
    TonexParameters[TONEX_PARAM_VIR_MIC_1_Z].Min = 0;
    TonexParameters[TONEX_PARAM_VIR_MIC_1_Z].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_VIR_MIC_1_Z].Name, "VIR M1Z");

    TonexParameters[TONEX_PARAM_VIR_MIC_2].Value = 0;
    TonexParameters[TONEX_PARAM_VIR_MIC_2].Min = 0;
    TonexParameters[TONEX_PARAM_VIR_MIC_2].Max = 2;
    sprintf(TonexParameters[TONEX_PARAM_VIR_MIC_2].Name, "VIR M2");

    TonexParameters[TONEX_PARAM_VIR_MIC_2_X].Value = 0;
    TonexParameters[TONEX_PARAM_VIR_MIC_2_X].Min = 0;
    TonexParameters[TONEX_PARAM_VIR_MIC_2_X].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_VIR_MIC_2_X].Name, "VIR M2X");

    TonexParameters[TONEX_PARAM_VIR_MIC_2_Y].Value = 0;
    TonexParameters[TONEX_PARAM_VIR_MIC_2_Y].Min = 0;
    TonexParameters[TONEX_PARAM_VIR_MIC_2_Y].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_VIR_MIC_2_Y].Name, "VIR M2Y");

    TonexParameters[TONEX_PARAM_VIR_MIC_2_Z].Value = 0;
    TonexParameters[TONEX_PARAM_VIR_MIC_2_Z].Min = 0;
    TonexParameters[TONEX_PARAM_VIR_MIC_2_Z].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_VIR_MIC_2_Z].Name, "VIR M2Z");

    TonexParameters[TONEX_PARAM_VIR_BLEND].Value = 0;
    TonexParameters[TONEX_PARAM_VIR_BLEND].Min = -100;
    TonexParameters[TONEX_PARAM_VIR_BLEND].Max = 100;
    sprintf(TonexParameters[TONEX_PARAM_VIR_BLEND].Name, "VIR BLEND");

    // Reverb
    TonexParameters[TONEX_PARAM_REVERB_POSITION].Value = 0;
    TonexParameters[TONEX_PARAM_REVERB_POSITION].Min = 0;
    TonexParameters[TONEX_PARAM_REVERB_POSITION].Max = 1;
    sprintf(TonexParameters[TONEX_PARAM_REVERB_POSITION].Name, "RVB POS");

    TonexParameters[TONEX_PARAM_REVERB_ENABLE].Value = 1;
    TonexParameters[TONEX_PARAM_REVERB_ENABLE].Min = 0;
    TonexParameters[TONEX_PARAM_REVERB_ENABLE].Max = 1;
    sprintf(TonexParameters[TONEX_PARAM_REVERB_ENABLE].Name, "RVB POWER");

    TonexParameters[TONEX_PARAM_REVERB_MODEL].Value = 0;
    TonexParameters[TONEX_PARAM_REVERB_MODEL].Min = 0;
    TonexParameters[TONEX_PARAM_REVERB_MODEL].Max = 5;
    sprintf(TonexParameters[TONEX_PARAM_REVERB_MODEL].Name, "RVB MODEL");

    TonexParameters[TONEX_PARAM_REVERB_SPRING1_TIME].Value = 5;
    TonexParameters[TONEX_PARAM_REVERB_SPRING1_TIME].Min = 0;
    TonexParameters[TONEX_PARAM_REVERB_SPRING1_TIME].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_REVERB_SPRING1_TIME].Name, "RVB S1 T");

    TonexParameters[TONEX_PARAM_REVERB_SPRING1_PREDELAY].Value = 0;
    TonexParameters[TONEX_PARAM_REVERB_SPRING1_PREDELAY].Min = 0;
    TonexParameters[TONEX_PARAM_REVERB_SPRING1_PREDELAY].Max = 500;
    sprintf(TonexParameters[TONEX_PARAM_REVERB_SPRING1_PREDELAY].Name, "RVB S1 P");

    TonexParameters[TONEX_PARAM_REVERB_SPRING1_COLOR].Value = 0;
    TonexParameters[TONEX_PARAM_REVERB_SPRING1_COLOR].Min = -10;
    TonexParameters[TONEX_PARAM_REVERB_SPRING1_COLOR].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_REVERB_SPRING1_COLOR].Name, "RVB S1 C");

    TonexParameters[TONEX_PARAM_REVERB_SPRING1_MIX].Value = 0;
    TonexParameters[TONEX_PARAM_REVERB_SPRING1_MIX].Min = 0;
    TonexParameters[TONEX_PARAM_REVERB_SPRING1_MIX].Max = 100;
    sprintf(TonexParameters[TONEX_PARAM_REVERB_SPRING1_MIX].Name, "RVB S1 M");

    TonexParameters[TONEX_PARAM_REVERB_SPRING2_TIME].Value = 5;
    TonexParameters[TONEX_PARAM_REVERB_SPRING2_TIME].Min = 0;
    TonexParameters[TONEX_PARAM_REVERB_SPRING2_TIME].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_REVERB_SPRING2_TIME].Name, "RVB S2 T");

    TonexParameters[TONEX_PARAM_REVERB_SPRING2_PREDELAY].Value = 0;
    TonexParameters[TONEX_PARAM_REVERB_SPRING2_PREDELAY].Min = 0;
    TonexParameters[TONEX_PARAM_REVERB_SPRING2_PREDELAY].Max = 500;
    sprintf(TonexParameters[TONEX_PARAM_REVERB_SPRING2_PREDELAY].Name, "RVB S2 P");

    TonexParameters[TONEX_PARAM_REVERB_SPRING2_COLOR].Value = 0;
    TonexParameters[TONEX_PARAM_REVERB_SPRING2_COLOR].Min = -10;
    TonexParameters[TONEX_PARAM_REVERB_SPRING2_COLOR].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_REVERB_SPRING2_COLOR].Name, "RVB S2 C");

    TonexParameters[TONEX_PARAM_REVERB_SPRING2_MIX].Value = 0;
    TonexParameters[TONEX_PARAM_REVERB_SPRING2_MIX].Min = 0;
    TonexParameters[TONEX_PARAM_REVERB_SPRING2_MIX].Max = 100;
    sprintf(TonexParameters[TONEX_PARAM_REVERB_SPRING2_MIX].Name, "RVB S2 M");

    TonexParameters[TONEX_PARAM_REVERB_SPRING3_TIME].Value = 5;
    TonexParameters[TONEX_PARAM_REVERB_SPRING3_TIME].Min = 0;
    TonexParameters[TONEX_PARAM_REVERB_SPRING3_TIME].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_REVERB_SPRING3_TIME].Name, "RVB S3 T");

    TonexParameters[TONEX_PARAM_REVERB_SPRING3_PREDELAY].Value = 0;
    TonexParameters[TONEX_PARAM_REVERB_SPRING3_PREDELAY].Min = 0;
    TonexParameters[TONEX_PARAM_REVERB_SPRING3_PREDELAY].Max = 500;
    sprintf(TonexParameters[TONEX_PARAM_REVERB_SPRING3_PREDELAY].Name, "RVB S3 P");

    TonexParameters[TONEX_PARAM_REVERB_SPRING3_COLOR].Value = 0;
    TonexParameters[TONEX_PARAM_REVERB_SPRING3_COLOR].Min = -10;
    TonexParameters[TONEX_PARAM_REVERB_SPRING3_COLOR].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_REVERB_SPRING3_COLOR].Name, "RVB S3 C");

    TonexParameters[TONEX_PARAM_REVERB_SPRING3_MIX].Value = 0;
    TonexParameters[TONEX_PARAM_REVERB_SPRING3_MIX].Min = 0;
    TonexParameters[TONEX_PARAM_REVERB_SPRING3_MIX].Max = 100;
    sprintf(TonexParameters[TONEX_PARAM_REVERB_SPRING3_MIX].Name, "RVB S3 M");

    TonexParameters[TONEX_PARAM_REVERB_SPRING4_TIME].Value = 5;
    TonexParameters[TONEX_PARAM_REVERB_SPRING4_TIME].Min = 0;
    TonexParameters[TONEX_PARAM_REVERB_SPRING4_TIME].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_REVERB_SPRING4_TIME].Name, "RVB S4 T");

    TonexParameters[TONEX_PARAM_REVERB_SPRING4_PREDELAY].Value = 0;
    TonexParameters[TONEX_PARAM_REVERB_SPRING4_PREDELAY].Min = 0;
    TonexParameters[TONEX_PARAM_REVERB_SPRING4_PREDELAY].Max = 500;
    sprintf(TonexParameters[TONEX_PARAM_REVERB_SPRING4_PREDELAY].Name, "RVB S4 P");

    TonexParameters[TONEX_PARAM_REVERB_SPRING4_COLOR].Value = 0;
    TonexParameters[TONEX_PARAM_REVERB_SPRING4_COLOR].Min = 10;
    TonexParameters[TONEX_PARAM_REVERB_SPRING4_COLOR].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_REVERB_SPRING4_COLOR].Name, "RVB S4 C");

    TonexParameters[TONEX_PARAM_REVERB_SPRING4_MIX].Value =0;
    TonexParameters[TONEX_PARAM_REVERB_SPRING4_MIX].Min = 0;
    TonexParameters[TONEX_PARAM_REVERB_SPRING4_MIX].Max = 100;
    sprintf(TonexParameters[TONEX_PARAM_REVERB_SPRING4_MIX].Name, "RVB S4 M");

    TonexParameters[TONEX_PARAM_REVERB_ROOM_TIME].Value = 5;
    TonexParameters[TONEX_PARAM_REVERB_ROOM_TIME].Min = 0;
    TonexParameters[TONEX_PARAM_REVERB_ROOM_TIME].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_REVERB_ROOM_TIME].Name, "RVB RM T");

    TonexParameters[TONEX_PARAM_REVERB_ROOM_PREDELAY].Value =0 ;
    TonexParameters[TONEX_PARAM_REVERB_ROOM_PREDELAY].Min = 0;
    TonexParameters[TONEX_PARAM_REVERB_ROOM_PREDELAY].Max = 500;
    sprintf(TonexParameters[TONEX_PARAM_REVERB_ROOM_PREDELAY].Name, "RVB RM P");

    TonexParameters[TONEX_PARAM_REVERB_ROOM_COLOR].Value = 0;
    TonexParameters[TONEX_PARAM_REVERB_ROOM_COLOR].Min = -10;
    TonexParameters[TONEX_PARAM_REVERB_ROOM_COLOR].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_REVERB_ROOM_COLOR].Name, "RVB RM C");

    TonexParameters[TONEX_PARAM_REVERB_ROOM_MIX].Value = 0;
    TonexParameters[TONEX_PARAM_REVERB_ROOM_MIX].Min = 0;
    TonexParameters[TONEX_PARAM_REVERB_ROOM_MIX].Max = 100;
    sprintf(TonexParameters[TONEX_PARAM_REVERB_ROOM_MIX].Name, "RVB RM M");

    TonexParameters[TONEX_PARAM_REVERB_PLATE_TIME].Value = 5;
    TonexParameters[TONEX_PARAM_REVERB_PLATE_TIME].Min = 0;
    TonexParameters[TONEX_PARAM_REVERB_PLATE_TIME].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_REVERB_PLATE_TIME].Name, "RVB PL T");

    TonexParameters[TONEX_PARAM_REVERB_PLATE_PREDELAY].Value = 0;
    TonexParameters[TONEX_PARAM_REVERB_PLATE_PREDELAY].Min = 0;
    TonexParameters[TONEX_PARAM_REVERB_PLATE_PREDELAY].Max = 500;
    sprintf(TonexParameters[TONEX_PARAM_REVERB_PLATE_PREDELAY].Name, "RVB PL P");

    TonexParameters[TONEX_PARAM_REVERB_PLATE_COLOR].Value = 0;
    TonexParameters[TONEX_PARAM_REVERB_PLATE_COLOR].Min = -10;
    TonexParameters[TONEX_PARAM_REVERB_PLATE_COLOR].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_REVERB_PLATE_COLOR].Name, "RVB PL C");

    TonexParameters[TONEX_PARAM_REVERB_PLATE_MIX].Value = 0;
    TonexParameters[TONEX_PARAM_REVERB_PLATE_MIX].Min = 0;
    TonexParameters[TONEX_PARAM_REVERB_PLATE_MIX].Max = 100;
    sprintf(TonexParameters[TONEX_PARAM_REVERB_PLATE_MIX].Name, "RVB PL M");

    // Modulation
    TonexParameters[TONEX_PARAM_MODULATION_POST].Value = 0;
    TonexParameters[TONEX_PARAM_MODULATION_POST].Min = 0;
    TonexParameters[TONEX_PARAM_MODULATION_POST].Max = 1;
    sprintf(TonexParameters[TONEX_PARAM_MODULATION_POST].Name, "MOD POST");

    TonexParameters[TONEX_PARAM_MODULATION_ENABLE].Value = 0;
    TonexParameters[TONEX_PARAM_MODULATION_ENABLE].Min = 0;
    TonexParameters[TONEX_PARAM_MODULATION_ENABLE].Max = 1;
    sprintf(TonexParameters[TONEX_PARAM_MODULATION_ENABLE].Name, "MOD POWER");

    TonexParameters[TONEX_PARAM_MODULATION_MODEL].Value = 0;
    TonexParameters[TONEX_PARAM_MODULATION_MODEL].Min = 0;
    TonexParameters[TONEX_PARAM_MODULATION_MODEL].Max = 4;
    sprintf(TonexParameters[TONEX_PARAM_MODULATION_MODEL].Name, "MOD MODEL");

    TonexParameters[TONEX_PARAM_MODULATION_CHORUS_SYNC].Value = 0;
    TonexParameters[TONEX_PARAM_MODULATION_CHORUS_SYNC].Min = 0;
    TonexParameters[TONEX_PARAM_MODULATION_CHORUS_SYNC].Max = 1;
    sprintf(TonexParameters[TONEX_PARAM_MODULATION_CHORUS_SYNC].Name, "MOD CH S");

    TonexParameters[TONEX_PARAM_MODULATION_CHORUS_TS].Value = 0;
    TonexParameters[TONEX_PARAM_MODULATION_CHORUS_TS].Min = 0;
    TonexParameters[TONEX_PARAM_MODULATION_CHORUS_TS].Max = 1;
    sprintf(TonexParameters[TONEX_PARAM_MODULATION_CHORUS_TS].Name, "MOD CH T");

    TonexParameters[TONEX_PARAM_MODULATION_CHORUS_RATE].Value = 0.5;
    TonexParameters[TONEX_PARAM_MODULATION_CHORUS_RATE].Min = 0.1;
    TonexParameters[TONEX_PARAM_MODULATION_CHORUS_RATE].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_MODULATION_CHORUS_RATE].Name, "MOD CH R");

    TonexParameters[TONEX_PARAM_MODULATION_CHORUS_DEPTH].Value = 0;
    TonexParameters[TONEX_PARAM_MODULATION_CHORUS_DEPTH].Min = 0;
    TonexParameters[TONEX_PARAM_MODULATION_CHORUS_DEPTH].Max = 100;
    sprintf(TonexParameters[TONEX_PARAM_MODULATION_CHORUS_DEPTH].Name, "MOD CH D");

    TonexParameters[TONEX_PARAM_MODULATION_CHORUS_LEVEL].Value = 0;
    TonexParameters[TONEX_PARAM_MODULATION_CHORUS_LEVEL].Min = 0;
    TonexParameters[TONEX_PARAM_MODULATION_CHORUS_LEVEL].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_MODULATION_CHORUS_LEVEL].Name, "MOD CH L");

    TonexParameters[TONEX_PARAM_MODULATION_TREMOLO_SYNC].Value = 0;
    TonexParameters[TONEX_PARAM_MODULATION_TREMOLO_SYNC].Min = 0;
    TonexParameters[TONEX_PARAM_MODULATION_TREMOLO_SYNC].Max = 1;
    sprintf(TonexParameters[TONEX_PARAM_MODULATION_TREMOLO_SYNC].Name, "MOD TR S");

    TonexParameters[TONEX_PARAM_MODULATION_TREMOLO_TS].Value = 0;
    TonexParameters[TONEX_PARAM_MODULATION_TREMOLO_TS].Min = 0;
    TonexParameters[TONEX_PARAM_MODULATION_TREMOLO_TS].Max = 1;
    sprintf(TonexParameters[TONEX_PARAM_MODULATION_TREMOLO_TS].Name, "MOD TR T");

    TonexParameters[TONEX_PARAM_MODULATION_TREMOLO_RATE].Value = 0.5;
    TonexParameters[TONEX_PARAM_MODULATION_TREMOLO_RATE].Min = 0.1;
    TonexParameters[TONEX_PARAM_MODULATION_TREMOLO_RATE].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_MODULATION_TREMOLO_RATE].Name, "MOD TR R");

    TonexParameters[TONEX_PARAM_MODULATION_TREMOLO_SHAPE].Value = 0;
    TonexParameters[TONEX_PARAM_MODULATION_TREMOLO_SHAPE].Min = 0;
    TonexParameters[TONEX_PARAM_MODULATION_TREMOLO_SHAPE].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_MODULATION_TREMOLO_SHAPE].Name, "MOD TR P");

    TonexParameters[TONEX_PARAM_MODULATION_TREMOLO_SPREAD].Value = 0;
    TonexParameters[TONEX_PARAM_MODULATION_TREMOLO_SPREAD].Min = 0;
    TonexParameters[TONEX_PARAM_MODULATION_TREMOLO_SPREAD].Max = 100;
    sprintf(TonexParameters[TONEX_PARAM_MODULATION_TREMOLO_SPREAD].Name, "MOD TR D");

    TonexParameters[TONEX_PARAM_MODULATION_TREMOLO_LEVEL].Value = 0;
    TonexParameters[TONEX_PARAM_MODULATION_TREMOLO_LEVEL].Min = 0;
    TonexParameters[TONEX_PARAM_MODULATION_TREMOLO_LEVEL].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_MODULATION_TREMOLO_LEVEL].Name, "MOD TR L");

    TonexParameters[TONEX_PARAM_MODULATION_PHASER_SYNC].Value = 0;
    TonexParameters[TONEX_PARAM_MODULATION_PHASER_SYNC].Min = 0;
    TonexParameters[TONEX_PARAM_MODULATION_PHASER_SYNC].Max = 1;
    sprintf(TonexParameters[TONEX_PARAM_MODULATION_PHASER_SYNC].Name, "MOD PH S");

    TonexParameters[TONEX_PARAM_MODULATION_PHASER_TS].Value = 0;
    TonexParameters[TONEX_PARAM_MODULATION_PHASER_TS].Min = 0;
    TonexParameters[TONEX_PARAM_MODULATION_PHASER_TS].Max = 1;
    sprintf(TonexParameters[TONEX_PARAM_MODULATION_PHASER_TS].Name, "MOD PH T");

    TonexParameters[TONEX_PARAM_MODULATION_PHASER_RATE].Value = 0.5;
    TonexParameters[TONEX_PARAM_MODULATION_PHASER_RATE].Min = 0.1;
    TonexParameters[TONEX_PARAM_MODULATION_PHASER_RATE].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_MODULATION_PHASER_RATE].Name, "MOD PH R");

    TonexParameters[TONEX_PARAM_MODULATION_PHASER_DEPTH].Value = 0;
    TonexParameters[TONEX_PARAM_MODULATION_PHASER_DEPTH].Min = 0;
    TonexParameters[TONEX_PARAM_MODULATION_PHASER_DEPTH].Max = 100;
    sprintf(TonexParameters[TONEX_PARAM_MODULATION_PHASER_DEPTH].Name, "MOD PH D");

    TonexParameters[TONEX_PARAM_MODULATION_PHASER_LEVEL].Value = 0;
    TonexParameters[TONEX_PARAM_MODULATION_PHASER_LEVEL].Min = 0;
    TonexParameters[TONEX_PARAM_MODULATION_PHASER_LEVEL].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_MODULATION_PHASER_LEVEL].Name, "MOD PH L");

    TonexParameters[TONEX_PARAM_MODULATION_FLANGER_SYNC].Value = 0;
    TonexParameters[TONEX_PARAM_MODULATION_FLANGER_SYNC].Min = 0;
    TonexParameters[TONEX_PARAM_MODULATION_FLANGER_SYNC].Max = 1;
    sprintf(TonexParameters[TONEX_PARAM_MODULATION_FLANGER_SYNC].Name, "MOD FL S");

    TonexParameters[TONEX_PARAM_MODULATION_FLANGER_TS].Value = 0;
    TonexParameters[TONEX_PARAM_MODULATION_FLANGER_TS].Min = 0;
    TonexParameters[TONEX_PARAM_MODULATION_FLANGER_TS].Max = 1;
    sprintf(TonexParameters[TONEX_PARAM_MODULATION_FLANGER_TS].Name, "MOD FL T");

    TonexParameters[TONEX_PARAM_MODULATION_FLANGER_RATE].Value = 0.5;
    TonexParameters[TONEX_PARAM_MODULATION_FLANGER_RATE].Min = 0.1;
    TonexParameters[TONEX_PARAM_MODULATION_FLANGER_RATE].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_MODULATION_FLANGER_RATE].Name, "MOD FL R");

    TonexParameters[TONEX_PARAM_MODULATION_FLANGER_DEPTH].Value = 0;
    TonexParameters[TONEX_PARAM_MODULATION_FLANGER_DEPTH].Min = 0;
    TonexParameters[TONEX_PARAM_MODULATION_FLANGER_DEPTH].Max = 100;
    sprintf(TonexParameters[TONEX_PARAM_MODULATION_FLANGER_DEPTH].Name, "MOD FL D");

    TonexParameters[TONEX_PARAM_MODULATION_FLANGER_FEEDBACK].Value = 0;
    TonexParameters[TONEX_PARAM_MODULATION_FLANGER_FEEDBACK].Min = 0;
    TonexParameters[TONEX_PARAM_MODULATION_FLANGER_FEEDBACK].Max = 100;
    sprintf(TonexParameters[TONEX_PARAM_MODULATION_FLANGER_FEEDBACK].Name, "MOD FL F");

    TonexParameters[TONEX_PARAM_MODULATION_FLANGER_LEVEL].Value = 0;
    TonexParameters[TONEX_PARAM_MODULATION_FLANGER_LEVEL].Min = 0;
    TonexParameters[TONEX_PARAM_MODULATION_FLANGER_LEVEL].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_MODULATION_FLANGER_LEVEL].Name, "MOD FL L");

    TonexParameters[TONEX_PARAM_MODULATION_ROTARY_SYNC].Value = 0;
    TonexParameters[TONEX_PARAM_MODULATION_ROTARY_SYNC].Min = 0;
    TonexParameters[TONEX_PARAM_MODULATION_ROTARY_SYNC].Max = 1;
    sprintf(TonexParameters[TONEX_PARAM_MODULATION_ROTARY_SYNC].Name, "MOD RO S");

    TonexParameters[TONEX_PARAM_MODULATION_ROTARY_TS].Value = 0;
    TonexParameters[TONEX_PARAM_MODULATION_ROTARY_TS].Min = 0;
    TonexParameters[TONEX_PARAM_MODULATION_ROTARY_TS].Max = 1;
    sprintf(TonexParameters[TONEX_PARAM_MODULATION_ROTARY_TS].Name, "MOD RO T");

    TonexParameters[TONEX_PARAM_MODULATION_ROTARY_SPEED].Value = 0;
    TonexParameters[TONEX_PARAM_MODULATION_ROTARY_SPEED].Min = 0;
    TonexParameters[TONEX_PARAM_MODULATION_ROTARY_SPEED].Max = 400;
    sprintf(TonexParameters[TONEX_PARAM_MODULATION_ROTARY_SPEED].Name, "MOD RO S");

    TonexParameters[TONEX_PARAM_MODULATION_ROTARY_RADIUS].Value = 0;
    TonexParameters[TONEX_PARAM_MODULATION_ROTARY_RADIUS].Min = 0;
    TonexParameters[TONEX_PARAM_MODULATION_ROTARY_RADIUS].Max = 300;
    sprintf(TonexParameters[TONEX_PARAM_MODULATION_ROTARY_RADIUS].Name, "MOD RO R");

    TonexParameters[TONEX_PARAM_MODULATION_ROTARY_SPREAD].Value = 0;
    TonexParameters[TONEX_PARAM_MODULATION_ROTARY_SPREAD].Min = 0;
    TonexParameters[TONEX_PARAM_MODULATION_ROTARY_SPREAD].Max = 100;
    sprintf(TonexParameters[TONEX_PARAM_MODULATION_ROTARY_SPREAD].Name, "MOD RO D");

    TonexParameters[TONEX_PARAM_MODULATION_ROTARY_LEVEL].Value = 0;
    TonexParameters[TONEX_PARAM_MODULATION_ROTARY_LEVEL].Min = 0;
    TonexParameters[TONEX_PARAM_MODULATION_ROTARY_LEVEL].Max = 10;
    sprintf(TonexParameters[TONEX_PARAM_MODULATION_ROTARY_LEVEL].Name, "MOD RO L");

    // Delay
     TonexParameters[TONEX_PARAM_DELAY_POST].Value = 0;
    TonexParameters[TONEX_PARAM_DELAY_POST].Min = 0;
    TonexParameters[TONEX_PARAM_DELAY_POST].Max = 1;
    sprintf(TonexParameters[TONEX_PARAM_DELAY_POST].Name, "DLY POST");
    
    TonexParameters[TONEX_PARAM_DELAY_ENABLE].Value = 0;
    TonexParameters[TONEX_PARAM_DELAY_ENABLE].Min = 0;
    TonexParameters[TONEX_PARAM_DELAY_ENABLE].Max = 1;
    sprintf(TonexParameters[TONEX_PARAM_DELAY_ENABLE].Name, "DLY POWER");
    
    TonexParameters[TONEX_PARAM_DELAY_MODEL].Value = 0;
    TonexParameters[TONEX_PARAM_DELAY_MODEL].Min = 0;
    TonexParameters[TONEX_PARAM_DELAY_MODEL].Max = 1;
    sprintf(TonexParameters[TONEX_PARAM_DELAY_MODEL].Name, "DLY MODEL");
    
    TonexParameters[TONEX_PARAM_DELAY_DIGITAL_SYNC].Value = 0;
    TonexParameters[TONEX_PARAM_DELAY_DIGITAL_SYNC].Min = 0;
    TonexParameters[TONEX_PARAM_DELAY_DIGITAL_SYNC].Max = 1;
    sprintf(TonexParameters[TONEX_PARAM_DELAY_DIGITAL_SYNC].Name, "DLY DG S");
    
    TonexParameters[TONEX_PARAM_DELAY_DIGITAL_TS].Value =0 ;
    TonexParameters[TONEX_PARAM_DELAY_DIGITAL_TS].Min = 0;
    TonexParameters[TONEX_PARAM_DELAY_DIGITAL_TS].Max = 1;
    sprintf(TonexParameters[TONEX_PARAM_DELAY_DIGITAL_TS].Name, "DLY DG T");
    
    TonexParameters[TONEX_PARAM_DELAY_DIGITAL_TIME].Value = 0;
    TonexParameters[TONEX_PARAM_DELAY_DIGITAL_TIME].Min = 0;
    TonexParameters[TONEX_PARAM_DELAY_DIGITAL_TIME].Max = 1000;
    sprintf(TonexParameters[TONEX_PARAM_DELAY_DIGITAL_TIME].Name, "DLY DT M");
    
    TonexParameters[TONEX_PARAM_DELAY_DIGITAL_FEEDBACK].Value = 0;
    TonexParameters[TONEX_PARAM_DELAY_DIGITAL_FEEDBACK].Min = 0;
    TonexParameters[TONEX_PARAM_DELAY_DIGITAL_FEEDBACK].Max = 100;
    sprintf(TonexParameters[TONEX_PARAM_DELAY_DIGITAL_FEEDBACK].Name, "DLY DT F");
    
    TonexParameters[TONEX_PARAM_DELAY_DIGITAL_MODE].Value = 0;
    TonexParameters[TONEX_PARAM_DELAY_DIGITAL_MODE].Min = 0;
    TonexParameters[TONEX_PARAM_DELAY_DIGITAL_MODE].Max = 1;
    sprintf(TonexParameters[TONEX_PARAM_DELAY_DIGITAL_MODE].Name, "DLY DT O");
    
    TonexParameters[TONEX_PARAM_DELAY_DIGITAL_MIX].Value = 0;
    TonexParameters[TONEX_PARAM_DELAY_DIGITAL_MIX].Min = 0;
    TonexParameters[TONEX_PARAM_DELAY_DIGITAL_MIX].Max = 100;
    sprintf(TonexParameters[TONEX_PARAM_DELAY_DIGITAL_MIX].Name, "DLY DT X");
    
    TonexParameters[TONEX_PARAM_DELAY_TAPE_SYNC].Value = 0;
    TonexParameters[TONEX_PARAM_DELAY_TAPE_SYNC].Min = 0;
    TonexParameters[TONEX_PARAM_DELAY_TAPE_SYNC].Max = 1;
    sprintf(TonexParameters[TONEX_PARAM_DELAY_TAPE_SYNC].Name, "DLY TA S");
    
    TonexParameters[TONEX_PARAM_DELAY_TAPE_TS].Value = 0;
    TonexParameters[TONEX_PARAM_DELAY_TAPE_TS].Min = 0;
    TonexParameters[TONEX_PARAM_DELAY_TAPE_TS].Max = 1;
    sprintf(TonexParameters[TONEX_PARAM_DELAY_TAPE_TS].Name, "DLY TA T");
    
    TonexParameters[TONEX_PARAM_DELAY_TAPE_TIME].Value = 0;
    TonexParameters[TONEX_PARAM_DELAY_TAPE_TIME].Min = 0;
    TonexParameters[TONEX_PARAM_DELAY_TAPE_TIME].Max = 1000;
    sprintf(TonexParameters[TONEX_PARAM_DELAY_TAPE_TIME].Name, "DLY TA M");
    
    TonexParameters[TONEX_PARAM_DELAY_TAPE_FEEDBACK].Value = 0;
    TonexParameters[TONEX_PARAM_DELAY_TAPE_FEEDBACK].Min = 0;
    TonexParameters[TONEX_PARAM_DELAY_TAPE_FEEDBACK].Max = 100;
    sprintf(TonexParameters[TONEX_PARAM_DELAY_TAPE_FEEDBACK].Name, "DLY TA F");
        
    TonexParameters[TONEX_PARAM_DELAY_TAPE_MODE].Value = 0;
    TonexParameters[TONEX_PARAM_DELAY_TAPE_MODE].Min = 0;
    TonexParameters[TONEX_PARAM_DELAY_TAPE_MODE].Max = 1;
    sprintf(TonexParameters[TONEX_PARAM_DELAY_TAPE_MODE].Name, "DLY TA O");
    
    TonexParameters[TONEX_PARAM_DELAY_TAPE_MIX].Value = 0;
    TonexParameters[TONEX_PARAM_DELAY_TAPE_MIX].Min = 0;
    TonexParameters[TONEX_PARAM_DELAY_TAPE_MIX].Max = 100;
    sprintf(TonexParameters[TONEX_PARAM_DELAY_TAPE_MIX].Name, "DLY TA X");

    return ESP_OK;
}