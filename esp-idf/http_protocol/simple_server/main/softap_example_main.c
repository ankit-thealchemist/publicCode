/*  WiFi softAP Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <esp_log.h>

#include "lwip/err.h"
#include "lwip/sys.h"

/* The examples use WiFi configuration that you can set via project configuration menu.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID "mywebserver"
#define EXAMPLE_ESP_WIFI_PASS "ankit12345"
#define EXAMPLE_ESP_WIFI_CHANNEL 0
#define EXAMPLE_MAX_STA_CONN 5

static const char *TAG = "wifi softAP";

extern esp_netif_t *myAccessPoint;

void wifi_event_handler(void *arg, esp_event_base_t event_base,int32_t event_id, void *event_data);

void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());                // create the lwip task
    ESP_ERROR_CHECK(esp_event_loop_create_default()); // create the event loop task

    // saving the handle of the network interface so that we can retrive the ip address later
    myAccessPoint = esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg)); // create the wifi task

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,          // wifi event to be filtered out
                                                        ESP_EVENT_ANY_ID,    // any event id with the event base of WIFI_EVENT
                                                        &wifi_event_handler, //handler function for the wifi events
                                                        NULL,
                                                        NULL));


    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK},
        /*
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .scan_method = 0,
            .bssid_set = 0,
            .channel = 0,
        }*/
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0)
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
}
