#include <stdio.h>
#include "curl/curl.h"
#include "esp_log.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"


#define OTA_BUF_SIZE (1500 * 1024)

#define TAG "ESP_LIBCURL"

#define URI "ftp://sftp-uat-user:sftp-uat-password@ftp.evfirst.ca:2022/firmware-downloader/69/7.4.2.56/OCPP_CA7.4.2.56.bin"

#define EXAMPLE_ESP_WIFI_SSID      "developer"
#define EXAMPLE_ESP_WIFI_PASS      "home@1234"

uint8_t *otaBuffer = NULL;
uint32_t ota_size = 0;
uint8_t connected = 0;
/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1


static int s_retry_num = 0;

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < 5)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

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
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (password len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
             * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            .threshold.authmode = WIFI_AUTH_WPA_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}

/**
 * @brief this fuction is the call back function for the downloading  the ota file
 *
 * @param data
 * @param size
 * @param nmemb
 * @param userp
 * @return size_t
 */
static size_t throw_away(char *data, size_t size, size_t nmemb, void *userp)
{
    ESP_LOGD(TAG, " size of nmeb %d", nmemb);

    memcpy(otaBuffer + ota_size, data, nmemb);
    ota_size += nmemb;
    ESP_LOGI(TAG, "File downloaded  %d bytes", ota_size);
    return (size_t)(size * nmemb);
}

void app_main(void)
{

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();

    
    esp_err_t err;

    CURL *curl;
    CURLcode res;


    ESP_LOGI(TAG, "URI IS %s",URI);

     // now inquiring the ota file size from the server
     curl_global_init(CURL_GLOBAL_DEFAULT);
     ota_size = 0;
     curl = curl_easy_init();
     if (curl == NULL)
     {
         curl_global_cleanup();
         return;
     }
     ESP_LOGI(TAG, "Allocating the memory for storing the buffer");
     curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
     /**
      * @brief HERE we are not getting the file size and doing the ota update, because we are concern of the binary size of the app itself. we have limited flash size. So we are allocating the large memory from the ram. In later phases we can update it to himem api
      *
      */
     otaBuffer = (uint8_t *)malloc(OTA_BUF_SIZE);
     memset(otaBuffer,0,OTA_BUF_SIZE);
     curl_easy_setopt(curl, CURLOPT_URL, URI);
 
     curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, throw_away);
 
     curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);
 
     /* Switch on full protocol/debug output */
     // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
 
     res = curl_easy_perform(curl);
 
     /* always cleanup */
     curl_easy_cleanup(curl);
 
     if (CURLE_OK != res)
     {
         ESP_LOGE(TAG, "curl_easy_perform() failed: %s", curl_easy_strerror(res));
         free(otaBuffer);
         otaBuffer = NULL;
         return;
     }
 
     curl_global_cleanup();
     ESP_LOGI(TAG, "size of ota is  %d", ota_size);
     return;

}
