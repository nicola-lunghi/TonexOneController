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

#pragma once

void control_init(void);

enum AmpSkins
{
    AMP_SKIN_JCM800,
    AMP_SKIN_TWIN_REVERB,
    AMP_SKIN_2001RB,
    AMP_SKIN_5150,
    AMP_SKIN_ACOUSTIC360,
    AMP_SKIN_B18N,
    AMP_SKIN_B15N,
    AMP_SKIN_BLUES_DELUXE,
    AMP_SKIN_CUSTOM_DELUXE,
    AMP_SKIN_DEVILLE,
    AMP_SKIN_DUAL_RECTIFIER,
    AMP_SKIN_GOLD_FINGER,
    AMP_SKIN_INVADER,
    AMP_SKIN_JAZZ_CHORUS,
    AMP_SKIN_OR_50,
    AMP_SKIN_POWERBALL,
    AMP_SKIN_PRINCETON,
    AMP_SKIN_ROCKERVERB,
    AMP_SKIN_SVTCL,
    AMP_SKIN_MAVERICK,
    AMP_SKIN_MK3,
    AMP_SKIN_SUPERBASS,
    AMP_SKIN_TRINITY,
    AMP_SKIN_DUMBLE,
    AMP_SKIN_JETCITY,
    AMP_SKIN_AC30,
    AMP_SKIN_EVH5150,
    AMP_SKIN_TINY_TERROR,
    AMP_SKIN_2020,
    AMP_SKIN_PINK_TACO,
    AMP_SKIN_SUPRO_50,
    AMP_SKIN_DIEZEL,
    AMP_SKIN_MAX        // must be last
};

// thread safe public API
void control_request_preset_up(void);
void control_request_preset_down(void);
void control_request_preset_index(uint8_t index);
void control_set_preset_name(char* name);
void control_set_preset_details(char* name);
void control_set_max_presets(uint32_t max);
void control_set_usb_status(uint32_t status);
void control_set_bt_status(uint32_t status);
void control_set_amp_skin_index(uint32_t status);
void control_set_amp_skin_next(void);
void control_set_amp_skin_previous(void);
void control_save_user_data(void);