/* Simple HTTP Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "http_server.h"
#include "soft_ap.h"
#include <esp_netif.h>
#include <esp_http_server.h>

/* A simple example that demonstrates how to create GET and POST
 * handlers for the web server.
 */

static const char *TAG = "Main_file";
esp_netif_t *myAccessPoint; // structure to create the netif structure
/**
 * @brief Event handler for wifi Task
 * 
 * @param arg  User data
 * @param event_base Event Base
 * @param event_id Event Id
 * @param event_data Event related date
 */
extern "C" void wifi_event_handler(void *arg, esp_event_base_t event_base,
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
        ap_records = (wifi_ap_record_t *)malloc(number * sizeof(ap_records));
        memset(ap_records, 0, number * sizeof(ap_records));

        uint16_t ap_count = 0;
        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_records));
        ESP_LOGI(TAG, "No of ap count is %d", ap_count);
        for (uint16_t i = 0; (i < number) && (i < ap_count); i++)
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
    {
        ESP_LOGI(TAG, "WIFI_EVENT_STA_START,");
        //ESP_ERROR_CHECK(esp_wifi_connect());
        wifi_scan_config_t wifi_config;
        wifi_config.channel = 0;
        wifi_config.scan_type = WIFI_SCAN_TYPE_PASSIVE;

        ESP_ERROR_CHECK(esp_wifi_scan_start(&wifi_config, true));
        break;
    }

    case WIFI_EVENT_STA_STOP:
    {
        ESP_LOGI(TAG, "WIFI_EVENT_STA_STOP");
        break;
    }

    case WIFI_EVENT_STA_CONNECTED:
    {
        ESP_LOGI(TAG, "WIFI_EVENT_STA_CONNECTED");
    }
    break;

    case WIFI_EVENT_STA_DISCONNECTED:
    {
        ESP_LOGI(TAG, "WIFI_EVENT_STA_DISCONNECTED");
    }
    break;

    case WIFI_EVENT_STA_AUTHMODE_CHANGE:
    {
        ESP_LOGI(TAG, "WIFI_EVENT_STA_AUTHMODE_CHANGE");
    }
    break;

    case WIFI_EVENT_STA_WPS_ER_SUCCESS:
    {
        ESP_LOGI(TAG, "WIFI_EVENT_STA_WPS_ER_SUCCESS");
    }
    break;

    case WIFI_EVENT_STA_WPS_ER_FAILED:
    {
        ESP_LOGI(TAG, "WIFI_EVENT_STA_WPS_ER_FAILED");
    }
    break;

    case WIFI_EVENT_STA_WPS_ER_TIMEOUT:
    {
        ESP_LOGI(TAG, "WIFI_EVENT_STA_WPS_ER_TIMEOUT");
    }
    break;

    case WIFI_EVENT_STA_WPS_ER_PIN:
    {
        ESP_LOGI(TAG, "WIFI_EVENT_STA_WPS_ER_PIN");
    }
    break;

    case WIFI_EVENT_STA_WPS_ER_PBC_OVERLAP:
    {
        ESP_LOGI(TAG, "WIFI_EVENT_STA_WPS_ER_PBC_OVERLAP");
    }
    break;

    case WIFI_EVENT_AP_START:
    {
        ESP_LOGI(TAG, "WIFI_EVENT_AP_START");
        esp_netif_ip_info_t ip_info;
        esp_netif_get_ip_info(myAccessPoint, &ip_info);
        printf("My IP: " IPSTR "\n", IP2STR(&ip_info.ip));
        printf("My GW: " IPSTR "\n", IP2STR(&ip_info.gw));
        printf("My NETMASK: " IPSTR "\n", IP2STR(&ip_info.netmask));
    }
    break;

    case WIFI_EVENT_AP_STOP:
    {
        ESP_LOGI(TAG, "WIFI_EVENT_AP_STOP");
    }
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
extern "C"
{
    void app_main(void)
    {
        //Initialize NVS
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
        }
        ESP_ERROR_CHECK(ret);

        // starting wifi
        wifi_init_softap();
        http_server_init();
    }
}
