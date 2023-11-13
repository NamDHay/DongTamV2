#include "WiFi.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "HTTP_POSTGET.h"
#include "DNS_Ping.h"
#include "OnlineStatusEvent.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static int s_retry_num = 0;


static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < 3) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI("WiFi_Station", "retry to connect to the AP");
        } else {
            OnlEvt_SetBit(ONL_EVT_WIFI_FAIL);
        }
        OnlEvt_ClearBit(ONL_EVT_WIFI_CONNECTED | ONL_EVT_PING_SUCCESS);
        ESP_LOGI("WiFi_Station","connect to the AP fail");
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI("WiFi_Station", "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        OnlEvt_SetBit(ONL_EVT_WIFI_CONNECTED);
        OnlEvt_ClearBit(ONL_EVT_WIFI_FAIL);
    }
}

void wifi_init_sta(void)
{
    // OnlEvt_CreateEventGroup();
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = USER_WIFI_SSID,
            .password = USER_WIFI_PASS,
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (pasword len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
	     * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );


    ESP_LOGI("WiFi_Station", "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    OnlEvt_WaitBit(ONL_EVT_WIFI_CONNECTED | ONL_EVT_WIFI_FAIL,pdFALSE,pdFALSE,portMAX_DELAY);
    if (OnlEvt_CheckBit(ONL_EVT_WIFI_CONNECTED)) {
        ESP_LOGI("WiFi_Station", "connected to ap SSID:%s password:%s",USER_WIFI_SSID, USER_WIFI_PASS);
    } else if(OnlEvt_CheckBit(ONL_EVT_WIFI_FAIL)){
        ESP_LOGI("WiFi_Station", "Failed to connect to SSID:%s, password:%s",USER_WIFI_SSID, USER_WIFI_PASS);
    } else {
        ESP_LOGE("WiFi_Station", "UNEXPECTED EVENT");
    }
}