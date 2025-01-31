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


#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_check.h"
#include "esp_log.h"
#include "usb/usb_host.h"
#include "driver/i2c.h"
#include "nvs_flash.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_ota_ops.h"
#include "sys/param.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "json_parser.h"
#include "json_generator.h"
#include "mdns.h"
#include <esp_http_server.h>
#include "control.h"
#include "wifi_config.h"
#include "usb_comms.h"
#include "task_priorities.h"
#include "tonex_params.h"

#define WIFI_CONFIG_TASK_STACK_SIZE   (3 * 1024)

#define ESP_WIFI_SSID           "TonexConfig"
#define ESP_WIFI_PASS           "12345678"
#define ESP_WIFI_CHANNEL        7
#define MAX_STA_CONN            2
#define WIFI_STA_MAXIMUM_RETRY  5
#define WIFI_CONNECTED_BIT      BIT0
#define WIFI_FAIL_BIT           BIT1
#define MAX_TEMP_BUFFER         (20 * 1024)

static int s_retry_num = 0;
int wifi_connect_status = 0;
static const char *TAG = "wifi_config";
static uint8_t client_connected = 0;
static httpd_handle_t http_server = NULL;
static httpd_config_t http_config = HTTPD_DEFAULT_CONFIG();
static EventGroupHandle_t s_wifi_event_group;

static esp_err_t index_get_handler(httpd_req_t *req);
static esp_err_t update_post_handler(httpd_req_t* req);
static esp_err_t get_handler(httpd_req_t *req);
static esp_err_t ws_handler(httpd_req_t *req);
static void ws_async_send(void *arg);
static esp_err_t trigger_async_send(httpd_handle_t handle, httpd_req_t *req);
static void wifi_kill_all(void);

typedef struct
{
    httpd_handle_t hd;
    int fd;
} async_resp_arg;

typedef struct 
{    
    jparse_ctx_t jctx;
    json_gen_str_t jstr;
    httpd_ws_frame_t ws_rsp;
    char wifi_ssid[MAX_WIFI_SSID_PW];
    char wifi_password[MAX_WIFI_SSID_PW];
    char TempBuffer[MAX_TEMP_BUFFER];
} tWebConfigData;

static const httpd_uri_t index_get = 
{
	.uri	  = "/",
	.method   = HTTP_GET,
	.handler  = index_get_handler,
	.user_ctx = NULL
};

#if 0
static const httpd_uri_t update_post = 
{
	.uri	  = "/config",
	.method   = HTTP_POST,
	.handler  = update_post_handler,
	.user_ctx = NULL
};
#endif 

static const httpd_uri_t ws = {
    .uri        = "/ws",
    .method     = HTTP_GET,
    .handler    = ws_handler,
    .user_ctx   = NULL,
    .is_websocket = true
};

// web page for config
extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");

static esp_err_t stop_webserver(void);
static void wifi_init_sta(void);
static tWebConfigData* pWebConfig;

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  
* RETURN:      none
* NOTES:       none
****************************************************************************/
static void ws_async_send(void *arg)
{
    static const char * data = "Async data";
    async_resp_arg *resp_arg = arg;
    httpd_handle_t hd = resp_arg->hd;
    int fd = resp_arg->fd;
    httpd_ws_frame_t ws_pkt;

    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t*)data;
    ws_pkt.len = strlen(data);
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    httpd_ws_send_frame_async(hd, fd, &ws_pkt);
    free(resp_arg);
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  
* RETURN:      none
* NOTES:       none
****************************************************************************/
static esp_err_t trigger_async_send(httpd_handle_t handle, httpd_req_t *req)
{
    async_resp_arg *resp_arg = malloc(sizeof(async_resp_arg));
    resp_arg->hd = req->handle;
    resp_arg->fd = httpd_req_to_sockfd(req);
    return httpd_queue_work(handle, ws_async_send, resp_arg);
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  
* RETURN:      none
* NOTES:       none
****************************************************************************/
static esp_err_t build_send_ws_response_packet(httpd_req_t *req, char* payload)
{
    esp_err_t ret;

    // clear anyt old data
    memset(&pWebConfig->ws_rsp, 0, sizeof(httpd_ws_frame_t));

    pWebConfig->ws_rsp.type = HTTPD_WS_TYPE_TEXT;
    pWebConfig->ws_rsp.payload = (uint8_t*)payload;
    pWebConfig->ws_rsp.len = strlen(payload);
    
    // send it
    ret = httpd_ws_send_frame(req, &pWebConfig->ws_rsp);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(TAG, "httpd_ws_send_frame failed with %d", ret);
    }        

    return ret;
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  
* RETURN:      none
* NOTES:       none
****************************************************************************/
static esp_err_t ws_handler(httpd_req_t *req)
{
    if (req->method == HTTP_GET) 
    {
        ESP_LOGI(TAG, "Handshake done, the new connection was opened");
        return ESP_OK;
    }
    
    httpd_ws_frame_t ws_pkt;
    uint8_t* buf = NULL;

    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    
    // Set max_len = 0 to get the frame len 
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }
    
    ESP_LOGI(TAG, "frame len is %d", ws_pkt.len);
    if (ws_pkt.len) 
    {
        // ws_pkt.len + 1 is for NULL termination as we are expecting a string 
        buf = heap_caps_malloc(ws_pkt.len + 1, MALLOC_CAP_SPIRAM);
        if (buf == NULL) 
        {
            ESP_LOGE(TAG, "Failed to calloc memory for buf");
            return ESP_ERR_NO_MEM;
        }
        memset((void*)buf, 0, ws_pkt.len + 1);
        
        ws_pkt.payload = buf;
        
        // Set max_len = ws_pkt.len to get the frame payload
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK) 
        {
            ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
            free(buf);
            return ret;
        }
        
        //ESP_LOGI(TAG, "Got ws packet with message: %s", ws_pkt.payload);

        // parse the json command        
        char str_val[64];
        int int_val;

        ESP_LOGI(TAG, "%s", ws_pkt.payload);

        if (json_parse_start(&pWebConfig->jctx, (const char*)ws_pkt.payload, strlen((const char*)ws_pkt.payload)) == OS_SUCCESS)
        {
            // get the command
            if (json_obj_get_string(&pWebConfig->jctx, "CMD", str_val, sizeof(str_val)) == OS_SUCCESS)
            {
                ESP_LOGI(TAG, "WS got command %s", str_val);

                if (strcmp(str_val, "GETPARAMS") == 0)
                {
                    // send current params
                    ESP_LOGI(TAG, "Param request");
                    tTonexParameter* param_ptr;

                    // init generation of json response
                    json_gen_str_start(&pWebConfig->jstr, pWebConfig->TempBuffer, MAX_TEMP_BUFFER, NULL, NULL);

                    // start json object, adds {
                    json_gen_start_object(&pWebConfig->jstr);

                    // add response
                    json_gen_obj_set_string(&pWebConfig->jstr, "CMD", "GETPARAMS");

                    json_gen_push_object(&pWebConfig->jstr, "PARAMS");

                    for (uint16_t loop = 0; loop < TONEX_PARAM_LAST; loop++)
                    {
                        // get access to parameters
                        tonex_params_get_locked_access(&param_ptr);

                        // add param index
                        sprintf(str_val, "%d", loop);
                        json_gen_push_object(&pWebConfig->jstr, str_val);

                        // add param details
                        json_gen_obj_set_float(&pWebConfig->jstr, "Val", param_ptr[loop].Value);
                        json_gen_obj_set_float(&pWebConfig->jstr, "Min", param_ptr[loop].Min);
                        json_gen_obj_set_float(&pWebConfig->jstr, "Max", param_ptr[loop].Max);
                        json_gen_obj_set_string(&pWebConfig->jstr, "NAME", param_ptr[loop].Name);

                        json_gen_pop_object(&pWebConfig->jstr);

                        // don't hog the param pointer                    
                        tonex_params_release_locked_access();
                    }
                    
                    // add the } for PARAMS
                    json_gen_pop_object(&pWebConfig->jstr);

                    // add the } for end
                    json_gen_end_object(&pWebConfig->jstr);

                    // end generation
                    json_gen_str_end(&pWebConfig->jstr);

                    //debug ESP_LOGI(TAG, "Json: %s", pWebConfig->TempBuffer);

                    // build packet and send
                    build_send_ws_response_packet(req, pWebConfig->TempBuffer);
                }
                else if (strcmp(str_val, "GETCONFIG") == 0)
                {
                    // send current config
                    ESP_LOGI(TAG, "Config request");

                    // init generation of json response
                    json_gen_str_start(&pWebConfig->jstr, pWebConfig->TempBuffer, MAX_TEMP_BUFFER, NULL, NULL);

                    // start json object, adds {
                    json_gen_start_object(&pWebConfig->jstr);

                    // add response
                    json_gen_obj_set_string(&pWebConfig->jstr, "CMD", "GETCONFIG");

                    // add config
                    json_gen_obj_set_int(&pWebConfig->jstr, "BT_MODE", control_get_config_bt_mode());
                    json_gen_obj_set_int(&pWebConfig->jstr, "BT_CHOC_EN", control_get_config_bt_mvave_choc_enable());
                    json_gen_obj_set_int(&pWebConfig->jstr, "BT_MD1_EN", control_get_config_bt_xvive_md1_enable());
                    json_gen_obj_set_int(&pWebConfig->jstr, "BT_CUST_EN", control_get_config_bt_custom_enable());

                    control_get_config_custom_bt_name(str_val);
                    json_gen_obj_set_string(&pWebConfig->jstr, "BT_CUST_NAME", str_val);

                    json_gen_obj_set_int(&pWebConfig->jstr, "TOGGLE_BYPASS", control_get_config_double_toggle());
                    json_gen_obj_set_int(&pWebConfig->jstr, "S_MIDI_EN", control_get_config_midi_serial_enable());
                    json_gen_obj_set_int(&pWebConfig->jstr, "S_MIDI_CH", control_get_config_midi_channel());
                    json_gen_obj_set_int(&pWebConfig->jstr, "FOOTSW_MODE", control_get_config_footswitch_mode());
                    json_gen_obj_set_int(&pWebConfig->jstr, "BT_MIDI_CC", control_get_config_enable_bt_midi_CC());
                    json_gen_obj_set_int(&pWebConfig->jstr, "WIFI_MODE", control_get_config_wifi_mode());

                    control_get_config_wifi_ssid(str_val);
                    json_gen_obj_set_string(&pWebConfig->jstr, "WIFI_SSID", str_val);

                    // might be best not to send password??
                    control_get_config_wifi_password(str_val);
                    json_gen_obj_set_string(&pWebConfig->jstr, "WIFI_PW", str_val);

                    // add the }
                    json_gen_end_object(&pWebConfig->jstr);

                    // end generation
                    json_gen_str_end(&pWebConfig->jstr);

                    ESP_LOGI(TAG, "Json: %s", pWebConfig->TempBuffer);

                    // build packet and send
                    build_send_ws_response_packet(req, pWebConfig->TempBuffer);
                }
                else if (strcmp(str_val, "GETPRESET") == 0)
                {
                    // send current preset details
                    ESP_LOGI(TAG, "Preset request");
                    
                }
                else if (strcmp(str_val, "SETCONFIG") == 0)
                {
                    // set config
                    ESP_LOGI(TAG, "Config Set");

                    if (json_obj_get_string(&pWebConfig->jctx, "S_MIDI_EN", str_val, sizeof(str_val)) == OS_SUCCESS)
                    {
                        control_set_config_serial_midi_enable(atoi(str_val));
                    }

                    if (json_obj_get_string(&pWebConfig->jctx, "S_MIDI_CH", str_val, sizeof(str_val)) == OS_SUCCESS)
                    {
                        control_set_config_serial_midi_channel(atoi(str_val));        
                    }

                    if (json_obj_get_string(&pWebConfig->jctx, "TOGGLE_BYPASS", str_val, sizeof(str_val)) == OS_SUCCESS)
                    {
                        control_set_config_toggle_bypass(atoi(str_val));
                    }
                    
                    if (json_obj_get_string(&pWebConfig->jctx, "BT_CHOC_EN", str_val, sizeof(str_val)) == OS_SUCCESS)
                    {
                        control_set_config_mv_choc_enable(atoi(str_val));
                    }

                    if (json_obj_get_string(&pWebConfig->jctx, "BT_MD1_EN", str_val, sizeof(str_val)) == OS_SUCCESS)
                    {
                        control_set_config_xv_md1_enable(atoi(str_val));
                    }

                    if (json_obj_get_string(&pWebConfig->jctx, "BT_CUST_EN", str_val, sizeof(str_val)) == OS_SUCCESS)
                    {
                        control_set_config_bt_custom_enable(atoi(str_val));
                    }
                    
                    if (json_obj_get_string(&pWebConfig->jctx, "BT_CUST_NAME", str_val, sizeof(str_val)) == OS_SUCCESS)
                    {
                        control_set_config_custom_bt_name(str_val);
                    }

                    if (json_obj_get_string(&pWebConfig->jctx, "BT_MIDI_CC", str_val, sizeof(str_val)) == OS_SUCCESS)
                    {
                        control_set_config_enable_bt_midi_CC(atoi(str_val));
                    }
                    
                    if (json_obj_get_string(&pWebConfig->jctx, "FOOTSW_MODE", str_val, sizeof(str_val)) == OS_SUCCESS)
                    {
                        control_set_config_footswitch_mode(atoi(str_val));
                    }

                    vTaskDelay(pdMS_TO_TICKS(250));
                    stop_webserver();
                    vTaskDelay(pdMS_TO_TICKS(250));

                    // save it and reboot after
                    control_save_user_data(1);
                }
                else if (strcmp(str_val, "SETWIFI") == 0)
                {
                    // set config
                    ESP_LOGI(TAG, "WiFi Set");

                    if (json_obj_get_int(&pWebConfig->jctx, "WIFI_MODE", &int_val) == OS_SUCCESS)
                    {
                        control_set_config_wifi_mode(int_val);
                    }

                    if (json_obj_get_string(&pWebConfig->jctx, "WIFI_SSID", str_val, sizeof(str_val)) == OS_SUCCESS)
                    {
                        control_set_config_wifi_ssid(str_val);
                    }

                    if (json_obj_get_string(&pWebConfig->jctx, "WIFI_PW", str_val, sizeof(str_val)) == OS_SUCCESS)
                    {
                        control_set_config_wifi_password(str_val);
                    }

                    vTaskDelay(pdMS_TO_TICKS(250));
                    stop_webserver();
                    vTaskDelay(pdMS_TO_TICKS(250));

                    // save it and reboot after
                    control_save_user_data(1);
                }
               
                else if (strcmp(str_val, "SETPARAM") == 0)
                {
                    uint16_t index;
                    float value;

                    ESP_LOGI(TAG, "Set Param");

                    if (json_obj_get_string(&pWebConfig->jctx, "INDEX", str_val, sizeof(str_val)) == OS_SUCCESS)
                    {
                        index = atoi(str_val);

                        if (json_obj_get_string(&pWebConfig->jctx, "VALUE", str_val, sizeof(str_val)) == OS_SUCCESS)
                        {
                            value = atof(str_val);
                            usb_modify_parameter(index, value);
                        }
                    }
                }
                else if (strcmp(str_val, "SETPRESET") == 0)
                {
                    // set preset
                    ESP_LOGI(TAG, "Preset Set");

                    if (json_obj_get_string(&pWebConfig->jctx, "PRESET", str_val, sizeof(str_val)) == OS_SUCCESS)
                    {
                        control_request_preset_index(atoi(str_val));
                    }
                }
            }
        }

        free(buf);
    }    
    
#if 0    
    ESP_LOGI(TAG, "Packet type: %d", ws_pkt.type);
    if (ws_pkt.type == HTTPD_WS_TYPE_TEXT && strcmp((char*)ws_pkt.payload,"Trigger async") == 0) 
    {
        free(buf);
        return trigger_async_send(req->handle, req);
    }
#endif 

    return ret;
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  
* RETURN:      none
* NOTES:       none
****************************************************************************/
static esp_err_t __attribute__((unused)) get_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "get_handler\n");

    // Send a simple response
    const char resp[] = "URI GET Response";

    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  
* RETURN:      none
* NOTES:       none
****************************************************************************/
static esp_err_t index_get_handler(httpd_req_t *req)
{
	httpd_resp_send(req, (const char*)index_html_start, index_html_end - index_html_start);
	return ESP_OK;
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  
* RETURN:      none
* NOTES:       none
****************************************************************************/
static char from_hex(char ch) 
{
    return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  
* RETURN:      none
* NOTES:       none
****************************************************************************/
static char* url_decode(char *str) 
{
    char *pstr = str, *buf = malloc(strlen(str) + 1), *pbuf = buf;
  
    while (*pstr) 
    {
        if (*pstr == '%') 
        {
            if (pstr[1] && pstr[2]) 
            {
                *pbuf++ = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
                pstr += 2;
            }
        } 
        else if (*pstr == '+') 
        { 
            *pbuf++ = ' ';
        } 
        else 
        {
            *pbuf++ = *pstr;
        }
    
        pstr++;
    }
  
    *pbuf = '\0';
    
    return buf;
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  
* RETURN:      none
* NOTES:       none
****************************************************************************/
uint8_t get_submitted_value(char* dest, char* ptr)
{
    char tmp_str[128];
    char* tmp_ptr = tmp_str;
    char* decoded;
    
    // copy until end of value
    while ((*ptr != ' ') && (*ptr != 0) && (*ptr != '&'))
    {
        *tmp_ptr = *ptr;
        tmp_ptr++;
        ptr++;
    } 
    
    // ensure null terminated
    *tmp_ptr = 0;
    
    // decode the URL
    decoded = url_decode(tmp_str);
    
    strcpy(dest, decoded);
    free(decoded);
      
    return 1;
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  
* RETURN:      none
* NOTES:       none
****************************************************************************/
static esp_err_t update_post_handler(httpd_req_t* req)
{
    char value[32] = {0};
    char* ptr;
    uint8_t temp_val;

    //ESP_LOGI(TAG, "root_post_handler req->uri=[%s]", req->uri);
	//ESP_LOGI(TAG, "root_post_handler content length %d", req->content_len);
	char*  buf = malloc(req->content_len + 1);

	size_t off = 0;
	while (off < req->content_len) 
    {
		// Read data received in the request
		int ret = httpd_req_recv(req, buf + off, req->content_len - off);
		if (ret <= 0) 
        {
			if (ret == HTTPD_SOCK_ERR_TIMEOUT) 
            {
				httpd_resp_send_408(req);
			}
			free (buf);
            ESP_LOGE(TAG, "update_post_handler failed timeout");
			return ESP_FAIL;
		}
		off += ret;
		//ESP_LOGI(TAG, "root_post_handler recv length %d", ret);
	}
	buf[off] = '\0';
	ESP_LOGI(TAG, "root_post_handler buf=[%s]", buf);

    // check for POST elements                  
    // look for bt mode
    ptr = strstr(buf, "btmode=");    
    if (ptr != NULL)
    {
        // skip up to =
        ptr += strlen("btmode=");
        get_submitted_value(value, ptr);
 
        uint8_t index = BT_MODE_CENTRAL;
        if (strcmp(value, "disabled") == 0)
        {
            index = BT_MODE_DISABLED;
        }
        else if (strcmp(value, "client") == 0)
        {
            index = BT_MODE_CENTRAL;
        }
        else if (strcmp(value, "server") == 0)
        {
            index = BT_MODE_PERIPHERAL;
        }
        control_set_config_btmode(index);
    }

    // look for midienabled
    ptr = strstr(buf, "midienabled=");    
    temp_val = 0;
    if (ptr != NULL)
    {
        // skip up to =
        ptr += strlen("midienabled=");
        get_submitted_value(value, ptr);

        if (strcmp(value, "on") == 0)
        {
            temp_val = 1;
        }
    }
    control_set_config_serial_midi_enable(temp_val);

    // look for midichannel
    ptr = strstr(buf, "midichannel=");    
    if (ptr != NULL)
    {
        // skip up to =
        ptr += strlen("midichannel=");
        get_submitted_value(value, ptr);

        control_set_config_serial_midi_channel(atoi(value));        
    }
    
    // look for togglebypass
    ptr = strstr(buf, "togglebypass=");   
    temp_val = 0;
    if (ptr != NULL)
    {
        // skip up to =
        ptr += strlen("togglebypass=");
        get_submitted_value(value, ptr);

        if (strcmp(value, "on") == 0)
        {
            temp_val = 1;
        }
    }
    control_set_config_toggle_bypass(temp_val);

    // look for mvavechoc
    ptr = strstr(buf, "mvavechoc=");    
    temp_val = 0;
    if (ptr != NULL)
    {
        // skip up to =
        ptr += strlen("mvavechoc=");
        get_submitted_value(value, ptr);

        if (strcmp(value, "on") == 0)
        {
            temp_val = 1;
        }
    }   
    control_set_config_mv_choc_enable(temp_val);

    // look for xvivemd1
    ptr = strstr(buf, "xvivemd1=");    
    temp_val = 0;
    if (ptr != NULL)
    {
        // skip up to =
        ptr += strlen("xvivemd1=");
        get_submitted_value(value, ptr);

        if (strcmp(value, "on") == 0)
        {
            temp_val = 1;
        }
    }
    control_set_config_xv_md1_enable(temp_val);


    // look for custom BT device enable
    ptr = strstr(buf, "custombte=");    
    temp_val = 0;
    if (ptr != NULL)
    {
        // skip up to =
        ptr += strlen("custombte=");
        get_submitted_value(value, ptr);

        if (strcmp(value, "on") == 0)
        {
            temp_val = 1;
        }
    }   
    control_set_config_bt_custom_enable(temp_val);

    // look for custom BT name
    ptr = strstr(buf, "custombt=");    
    if (ptr != NULL)
    {
        // skip up to =
        ptr += strlen("custombt=");
        get_submitted_value(value, ptr);

        control_set_config_custom_bt_name(value);
    }

    // look for BT midi CC
    ptr = strstr(buf, "btmidicc=");    
    temp_val = 0;
    if (ptr != NULL)
    {
        // skip up to =
        ptr += strlen("btmidicc=");
        get_submitted_value(value, ptr);

        if (strcmp(value, "on") == 0)
        {
            temp_val = 1;
        }
    }   
    control_set_config_enable_bt_midi_CC(temp_val);

    // look for footswitch mode
    ptr = strstr(buf, "footmode=");    
    if (ptr != NULL)
    {
        // skip up to =
        ptr += strlen("footmode=");
        get_submitted_value(value, ptr);
 
        uint8_t mode = FOOTSWITCH_MODE_DUAL_UP_DOWN;
        if (strcmp(value, "Dual Next/Previous") == 0)
        {
            mode = FOOTSWITCH_MODE_DUAL_UP_DOWN;
        }
        else if (strcmp(value, "Quad Banked") == 0)
        {
            mode = FOOTSWITCH_MODE_QUAD_BANKED;
        }
        else if (strcmp(value, "Quad Binary") == 0)
        {
            mode = FOOTSWITCH_MODE_QUAD_BINARY;
        }
        control_set_config_footswitch_mode(mode);
    }

    free(buf);

    // Send a simple response
    const char resp[] = "<span style=\"font-size: 7vw;\">Config save complete. Rebooting...</span>\n";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);

    vTaskDelay(pdMS_TO_TICKS(250));
    stop_webserver();
    vTaskDelay(pdMS_TO_TICKS(250));

    // save it and reboot after
    control_save_user_data(1);

	return ESP_OK;
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  
* RETURN:      none
* NOTES:       none
****************************************************************************/
static esp_err_t stop_webserver(void)
{
    // Stop the httpd server
    if (http_server != NULL)
    {
        httpd_stop(http_server);
    }

    return ESP_OK;
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  
* RETURN:      none
* NOTES:       none
****************************************************************************/
static esp_err_t http_server_init(void)
{
    ESP_LOGI(TAG, "Http server init");
    
    // set config
    http_config.task_priority      = tskIDLE_PRIORITY + 1;
    http_config.core_id            = 0;
    http_config.stack_size         = (3 * 1024);  
    http_config.server_port        = 80;
    http_config.ctrl_port          = 32768;
    http_config.max_open_sockets   = 4;
    http_config.max_uri_handlers   = 2;
    http_config.max_resp_headers   = 6;
    http_config.backlog_conn       = 1;
    http_config.keep_alive_enable  = true;
    http_config.keep_alive_idle    = 10;     // seconds
    http_config.keep_alive_interval = 10;    // seconds
    http_config.keep_alive_count = 5;
    http_config.open_fn = NULL;
    http_config.close_fn = NULL;
    http_config.uri_match_fn = NULL;
    http_config.lru_purge_enable = true;

	if (httpd_start(&http_server, &http_config) == ESP_OK) 
    {
        if (http_server != NULL)
        {
            ESP_LOGI(TAG, "Http register uri 1");
    	    httpd_register_uri_handler(http_server, &index_get);

            //ESP_LOGI(TAG, "Http register uri 2");
		    //httpd_register_uri_handler(http_server, &update_post);

            // Registering the ws handler
            ESP_LOGI(TAG, "Registering ws handler");
            httpd_register_uri_handler(http_server, &ws);
        }
	}

    if (http_server == NULL)
    {
        ESP_LOGE(TAG, "Http server init failed!");
        return ESP_FAIL;
    }
    else
    {
        ESP_LOGI(TAG, "Http server init OK");
        return ESP_OK;
    }
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  
* RETURN:      
* NOTES:       
*****************************************************************************/
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) 
    {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d", MAC2STR(event->mac), event->aid);
        client_connected = 1;
    } 
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) 
    {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d", MAC2STR(event->mac), event->aid);
        client_connected = 0;

        ESP_LOGI(TAG, "Wifi config stopping");
        wifi_kill_all();
    }
    else if ((event_base == WIFI_EVENT) && (event_id == WIFI_EVENT_STA_START))
    {
        esp_wifi_connect();
    }
    else if ((event_base == WIFI_EVENT) && (event_id == WIFI_EVENT_STA_DISCONNECTED))
    {
        if (s_retry_num < WIFI_STA_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        wifi_connect_status = 0;
        ESP_LOGI(TAG, "connect to the AP fail");
    }
    else if ((event_base == IP_EVENT) && (event_id == IP_EVENT_STA_GOT_IP))
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;

        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        wifi_connect_status = 1;
    }
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  
* RETURN:      
* NOTES:       
*****************************************************************************/
static void wifi_init_sta(void)
{
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            // Setting a password implies station will connect to all security modes including WEP/WPA.
            // However these modes are deprecated and not advisable to be used. Incase your Access point
            // doesn't support WPA2, these mode can be enabled by commenting below line
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    // get credentials from config
    control_get_config_wifi_ssid(pWebConfig->wifi_ssid);
    control_get_config_wifi_password(pWebConfig->wifi_password);

    // set SSID and password
    strcpy((char*)wifi_config.sta.ssid, pWebConfig->wifi_ssid);
    strcpy((char*)wifi_config.sta.password, pWebConfig->wifi_password);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    // Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
    // number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) 
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    // xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
    // happened. 
    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "connected to ap SSID:%s", pWebConfig->wifi_ssid);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s", pWebConfig->wifi_ssid);
    }
    else
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
    vEventGroupDelete(s_wifi_event_group);
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  
* RETURN:      
* NOTES:       
*****************************************************************************/
static void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .channel = ESP_WIFI_CHANNEL,
            .max_connection = MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .pmf_cfg = {
                .required = false,
            },
        },
    };
    
    // get credentials from config
    control_get_config_wifi_ssid(pWebConfig->wifi_ssid);
    control_get_config_wifi_password(pWebConfig->wifi_password);

    // set SSID and password
    strcpy((char*)wifi_config.ap.ssid, pWebConfig->wifi_ssid);
    wifi_config.ap.ssid_len = strlen((char*)pWebConfig->wifi_ssid),
    strcpy((char*)wifi_config.ap.password, pWebConfig->wifi_password);

    if (wifi_config.ap.ssid_len == 0) 
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s channel:%d", wifi_config.sta.ssid, ESP_WIFI_CHANNEL);
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  
* RETURN:      
* NOTES:       
*****************************************************************************/
void start_mdns_service()
{
    //initialize mDNS service
    esp_err_t err = mdns_init();
    if (err) 
    {
        ESP_LOGE(TAG, "MDNS Init failed: %d\n", err);
        return;
    }

    mdns_hostname_set("tonex");
    mdns_instance_name_set("tonex");
    mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  
* RETURN:      
* NOTES:       
*****************************************************************************/
static void wifi_kill_all(void)
{
    ESP_LOGI(TAG, "Wifi config stopping");
    stop_webserver();
    vTaskDelay(pdMS_TO_TICKS(5000));
    esp_wifi_stop();
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  
* RETURN:      
* NOTES:       
*****************************************************************************/
static void wifi_config_task(void *arg)
{
    ESP_LOGI(TAG, "Wifi config task start");

    // let everything settle and init
    vTaskDelay(pdMS_TO_TICKS(1000));

    ESP_LOGI(TAG, "Wifi config starting");
    s_wifi_event_group = xEventGroupCreate();

    pWebConfig = heap_caps_malloc(sizeof(tWebConfigData), MALLOC_CAP_SPIRAM);
    if (pWebConfig == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate pWebConfig buffer!");
        return;
    }

    // check wifi mode
    switch (control_get_config_wifi_mode())    
    {
        case WIFI_MODE_STATION:
        {
            // conect to AP
            wifi_init_sta();
        } break;

        case WIFI_MODE_ACCESS_POINT_TIMED:    // fall through
        case WIFI_MODE_ACCESS_POINT:          // fall through
        default:
        {
            // start up WiFi access point
            wifi_init_softap();
        } break;
    }

    start_mdns_service();

    // start web server
    http_server_init();

    if (control_get_config_wifi_mode() == WIFI_MODE_ACCESS_POINT_TIMED)
    {
        // allow WiFi AP to run for 60 seconds
        vTaskDelay(pdMS_TO_TICKS(60000));

        // if no clients, kill Wifi    
        if (client_connected == 0)
        {
            // kill
            ESP_LOGI(TAG, "Wifi config stopping");
            wifi_kill_all();
        }
    }

    // do nothing
    while (1) 
    {
		vTaskDelay(pdMS_TO_TICKS(10000));
	}
}

/****************************************************************************
* NAME:        
* DESCRIPTION: 
* PARAMETERS:  
* RETURN:      
* NOTES:       
*****************************************************************************/
void wifi_config_init(void)
{
    xTaskCreatePinnedToCore(wifi_config_task, "WIFI", WIFI_CONFIG_TASK_STACK_SIZE, NULL, WIFI_TASK_PRIORITY, NULL, 0);
}
