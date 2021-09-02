/*  WiFi softAP Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

/* The examples use WiFi configuration that you can set via project configuration menu.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID ""
#define EXAMPLE_ESP_WIFI_PASS ""
#define EXAMPLE_ESP_WIFI_CHANNEL CONFIG_ESP_WIFI_CHANNEL
#define EXAMPLE_MAX_STA_CONN CONFIG_ESP_MAX_STA_CONN

static const char *TAG = "wifi softAP";

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_WIFI_READY:
        ESP_LOGI(TAG, "WIFI_EVENT_WIFI_READY");
        break;

    case WIFI_EVENT_SCAN_DONE:
    {
        ESP_LOGI(TAG, "WIFI_EVENT_SCAN_DONE");
        uint16_t number = 5;
        wifi_ap_record_t *ap_records;
        ap_records = (wifi_ap_record_t *)malloc(number*sizeof(ap_records));
        memset(ap_records, 0,number* sizeof(ap_records));

        uint16_t ap_count = 0;
        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_records));
        ESP_LOGI(TAG,"No of ap count is %d",ap_count);
        for (uint16_t i = 0; (i < number) && (i<ap_count); i++)
        {
            ESP_LOGI(TAG, "SsIDs: %s\n", ap_records[i].ssid);
            ESP_LOGI(TAG, "primary channel: %d\n", ap_records[i].primary);
            ESP_LOGI(TAG, "rssi: %d\n", ap_records[i].rssi);
            ESP_LOGI(TAG, "MAC ADDRESS:%02x:%02x:%02x:%02x:%02x:%02x\n",
                     (unsigned char)ap_records[i].bssid[0],
                     (unsigned char)ap_records[i].bssid[1],
                     (unsigned char)ap_records[i].bssid[2],
                     (unsigned char)ap_records[i].bssid[3],
                     (unsigned char)ap_records[i].bssid[4],
                     (unsigned char)ap_records[i].bssid[5]);
            ESP_LOGI(TAG, "======================================================================================================");
        }
        free(ap_records);
        break;
    }

    case WIFI_EVENT_STA_START:
        ESP_LOGI(TAG, "WIFI_EVENT_STA_START,");
        //ESP_ERROR_CHECK(esp_wifi_connect());
        wifi_scan_config_t wifi_config = {
            .channel = 0,
            .scan_type = WIFI_SCAN_TYPE_PASSIVE,
        };
        ESP_ERROR_CHECK(esp_wifi_scan_start(&wifi_config, true));
        break;

    case WIFI_EVENT_STA_STOP:
        ESP_LOGI(TAG, "WIFI_EVENT_STA_STOP");
        break;

    case WIFI_EVENT_STA_CONNECTED:
        ESP_LOGI(TAG, "WIFI_EVENT_STA_CONNECTED");
        break;

    case WIFI_EVENT_STA_DISCONNECTED:
        ESP_LOGI(TAG, "WIFI_EVENT_STA_DISCONNECTED");
        break;

    case WIFI_EVENT_STA_AUTHMODE_CHANGE:
        ESP_LOGI(TAG, "WIFI_EVENT_STA_AUTHMODE_CHANGE");
        break;

    case WIFI_EVENT_STA_WPS_ER_SUCCESS:
        ESP_LOGI(TAG, "WIFI_EVENT_STA_WPS_ER_SUCCESS");
        break;

    case WIFI_EVENT_STA_WPS_ER_FAILED:
        ESP_LOGI(TAG, "WIFI_EVENT_STA_WPS_ER_FAILED");
        break;

    case WIFI_EVENT_STA_WPS_ER_TIMEOUT:
        ESP_LOGI(TAG, "WIFI_EVENT_STA_WPS_ER_TIMEOUT");
        break;

    case WIFI_EVENT_STA_WPS_ER_PIN:
        ESP_LOGI(TAG, "WIFI_EVENT_STA_WPS_ER_PIN");
        break;

    case WIFI_EVENT_STA_WPS_ER_PBC_OVERLAP:
        ESP_LOGI(TAG, "WIFI_EVENT_STA_WPS_ER_PBC_OVERLAP");
        break;

    case WIFI_EVENT_AP_START:
        ESP_LOGI(TAG, "WIFI_EVENT_AP_START");
        break;

    case WIFI_EVENT_AP_STOP:
        ESP_LOGI(TAG, "WIFI_EVENT_AP_STOP");
        break;

    case WIFI_EVENT_AP_STACONNECTED:
    {
        ESP_LOGI(TAG, "WIFI_EVENT_AP_STACONNECTED");
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d",
                 MAC2STR(event->mac), event->aid);
        break;
    }
    case WIFI_EVENT_AP_STADISCONNECTED:
    {
        ESP_LOGI(TAG, "WIFI_EVENT_AP_STADISCONNECTED");
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
        break;
    }

    case WIFI_EVENT_AP_PROBEREQRECVED:
        ESP_LOGI(TAG, "WIFI_EVENT_AP_PROBEREQRECVED");
        break;

    default:
        ESP_LOGI(TAG, "Value of the event is %d", event_id);
        break;
    }
}

void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());                // create the lwip task
    ESP_ERROR_CHECK(esp_event_loop_create_default()); // create the event loop task
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg)); // create the wifi task

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,          // wifi event to be filtered out
                                                        ESP_EVENT_ANY_ID,    // any event id with the event base of WIFI_EVENT
                                                        &wifi_event_handler, //handler function for the wifi events
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        /* .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK},*/

        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .scan_method = 0,
            .bssid_set = 0,
            .channel = 0,
        }};
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0)
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
}
