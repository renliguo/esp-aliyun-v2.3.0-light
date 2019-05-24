/*
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2018 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP32 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

//#include "iot_import.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "tcpip_adapter.h"
#include "esp_smartconfig.h"

#include "app_entry.h"
#include "platform_hal.h"

#include "driver_control.h"

#include "driver/pwm.h"
#include "driver/uart.h"

int WIFIconfig_cnt=0;

uint8_t data[BUF_SIZE];
uint8_t UART_RX_flag=0;
int len;

uint32_t pwm0_duty=1000;

const uint32_t pin_num[3] = {
    12,
    13,
    15
};

int16_t phase[4] = {
    0, 0, 50,
};

/* The examples use simple WiFi configuration that you can set via
   'make menuconfig'.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_WIFI_SSID CONFIG_WIFI_SSID
#define EXAMPLE_WIFI_PASS CONFIG_WIFI_PASSWORD

#define WIFI_CONF_SPACE_NAME     "WIFI_CONF_APP"
#define NVS_KEY_WIFI_CONFIG "wifi_config"

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int CONNECTED_BIT = BIT0;
static const int ESPTOUCH_DONE_BIT = BIT1;
const int WIFIconf_out_BIT = BIT2;

static const char* TAG = "app main";

wifi_config_t  wifi_config;

TaskHandle_t WIFIcofig_timer_task_Handler;

void smartconfig_example_task(void * parm);

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            //esp_wifi_connect();
        	xTaskCreate(smartconfig_example_task, "smartconfig_example_task", 5120, NULL, 3, NULL);
            break;

        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
            break;

        case SYSTEM_EVENT_STA_DISCONNECTED:
            /* This is a workaround as ESP8266 WiFi libs don't currently
               auto-reassociate. */
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
            break;

        default:
            break;
    }

    return ESP_OK;
}

static void initialise_wifi(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
//    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
//    wifi_config_t wifi_config = {
//        .sta = {
//            .ssid = EXAMPLE_WIFI_SSID,
//            .password = EXAMPLE_WIFI_PASS,
//        },
//    };
//    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
//    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static void sc_callback(smartconfig_status_t status, void *pdata)
{
    switch (status) {
        case SC_STATUS_WAIT:
            ESP_LOGI(TAG, "SC_STATUS_WAIT");
            break;
        case SC_STATUS_FIND_CHANNEL:
            ESP_LOGI(TAG, "SC_STATUS_FINDING_CHANNEL");
            break;
        case SC_STATUS_GETTING_SSID_PSWD:
            ESP_LOGI(TAG, "SC_STATUS_GETTING_SSID_PSWD");
            break;
        case SC_STATUS_LINK:
            ESP_LOGI(TAG, "SC_STATUS_LINK");

            wifi_config_t *wifi_config_temp = pdata;

            memcpy(&wifi_config,wifi_config_temp,sizeof(wifi_config_t));

            ESP_LOGI(TAG, "SSID:%s", wifi_config.sta.ssid);
            ESP_LOGI(TAG, "PASSWORD:%s", wifi_config.sta.password);
            //¥Ê¥¢WIFI’Àªß£¨√‹¬Î°£
            //esp_info_erase(NVS_KEY_WIFI_CONFIG);
            esp_info_save(NVS_KEY_WIFI_CONFIG,&wifi_config,sizeof(wifi_config_t));
            //vTaskDelete(WIFIcofig_timer_task_Handler);

            ESP_ERROR_CHECK( esp_wifi_disconnect() );
            ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
            ESP_ERROR_CHECK( esp_wifi_connect() );
            break;
        case SC_STATUS_LINK_OVER:
            ESP_LOGI(TAG, "SC_STATUS_LINK_OVER");
            if (pdata != NULL) {
                uint8_t phone_ip[4] = { 0 };
                memcpy(phone_ip, (uint8_t* )pdata, 4);
                ESP_LOGI(TAG, "Phone ip: %d.%d.%d.%d\n", phone_ip[0], phone_ip[1], phone_ip[2], phone_ip[3]);
            }
            xEventGroupSetBits(wifi_event_group, ESPTOUCH_DONE_BIT);
            break;
        default:
            break;
    }
}

void smartconfig_example_task(void * parm)
{
    EventBits_t uxBits;
    ESP_ERROR_CHECK( esp_smartconfig_set_type(SC_TYPE_ESPTOUCH_AIRKISS) );
    ESP_ERROR_CHECK( esp_smartconfig_start(sc_callback) );
    while (1) {
        uxBits = xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT | ESPTOUCH_DONE_BIT|WIFIconf_out_BIT, true, false, portMAX_DELAY);
        if(uxBits & CONNECTED_BIT) {
            ESP_LOGI(TAG, "WiFi Connected to ap");
        }
        if(uxBits & ESPTOUCH_DONE_BIT) {
            ESP_LOGI(TAG, "smartconfig over");

            esp_smartconfig_stop();
            vTaskDelete(WIFIcofig_timer_task_Handler);
            vTaskDelete(NULL);
        }
        if(uxBits & WIFIconf_out_BIT) {
            ESP_LOGI(TAG, "MY_WiFiconfig stop");
            esp_smartconfig_stop();

            ESP_ERROR_CHECK( esp_wifi_disconnect() );
            ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
            ESP_ERROR_CHECK( esp_wifi_connect() );

            vTaskDelete(NULL);
        }
    }
}

void mqtt_example(void* parameter)
{
    while(1) {
        ESP_LOGI(TAG, "wait wifi connect...");
        xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);

        app_main_paras_t paras;
        const char* argv[] = {"main", "loop"};
        paras.argc = 2;
        paras.argv = argv; 
    
        linkkit_main((void *)&paras);
    }
}

void LED_task(void *pvParameter)
{
    for(; ; )
    {
    	LED_toggle();
    	vTaskDelay(3000 / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}

void WIFIcofig_timer_task(void *pvParameter)
{
    for(; ; )
    {
    	//if(WIFIconfig_cnt<oxf)
    	WIFIconfig_cnt++;

    	if(WIFIconfig_cnt>200)
    	{
    		WIFIconfig_cnt=0;

    		xEventGroupSetBits(wifi_event_group, WIFIconf_out_BIT);

    		vTaskDelete(WIFIcofig_timer_task_Handler);
    	}
    	LED_toggle();
    	vTaskDelay(100 / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}

void UART_task(void *pvParameter)
{
    for(; ; )
    {
    	//LED_toggle();

    	if(UART_RX_flag==0)
    	{
			len = uart_read_bytes(UART_NUM_0, data, BUF_SIZE, 20 / portTICK_RATE_MS);
			//uart_write_bytes(UART_NUM_0, (const char *) data, len);

	    	if (strstr((const char *)data, "thing.event.property.post") != NULL)
	    	{
	    		UART_RX_flag=1;
	    		//HAL_SleepMs(2000);
	    	}
    	}
    	vTaskDelay(5 / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}

void app_main()
{
	uint32_t pwm_duty_init[3]={1000,0,0};
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "IDF version: %s", esp_get_idf_version());
    ESP_LOGI(TAG, "esp-aliyun verison: %s", HAL_GetEAVerison());
    ESP_LOGI(TAG, "iotkit-embedded version: %s", HAL_GetIEVerison());

    esp_info_load(NVS_KEY_WIFI_CONFIG, &wifi_config, sizeof(wifi_config_t));

    ESP_LOGI(TAG, "SSID:%s", wifi_config.sta.ssid);
    ESP_LOGI(TAG, "PASSWORD:%s", wifi_config.sta.password);

    xTaskCreate(WIFIcofig_timer_task, "WIFIcofig_timer_task", 4096, NULL, 5, (TaskHandle_t*  )WIFIcofig_timer_task_Handler);

    initialise_wifi();

    GPIO_init();

    // Configure parameters of an UART driver,
    // communication pins and install the driver
    uart_config_t uart_config = {
        .baud_rate = 74880,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_0, &uart_config);
    uart_driver_install(UART_NUM_0, BUF_SIZE * 2, 0, 0, NULL);

    // Configure a temporary buffer for the incoming data
//    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

//    xTaskCreate(LED_task, "LED_task", 512, NULL, 5, NULL);
    xTaskCreate(UART_task, "UART_task", 2048, NULL, 5, NULL);
    // SNTP service uses LwIP, please allocate large stack space.
    xTaskCreate(mqtt_example, "mqtt_example", 10240, NULL, 5, NULL);
}

