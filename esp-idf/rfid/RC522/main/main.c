/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include <esp_log.h>
#include <rc522.h>

static const char *TAG = "app";

void tag_handler(uint8_t *sn)
{ // serial number is always 5 bytes long
    ESP_LOGI(TAG, "Tag: %#x %#x %#x %#x %#x",
             sn[0], sn[1], sn[2], sn[3], sn[4]);
}

void app_main(void)
{
    const rc522_start_args_t start_args = {
        .miso_io = 25,
        .mosi_io = 23,
        .sck_io = 19,
        .sda_io = 22,
        .callback = &tag_handler,

        // Uncomment next line for attaching RC522 to SPI2 bus. Default is VSPI_HOST (SPI3)
        //.spi_host_id = HSPI_HOST
    };

    rc522_start(start_args);
}
