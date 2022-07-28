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
#include <driver/twai.h>
#include <esp_log.h>

/*******************************************defines****************************************************/

#define TAG "canBusTx"
/**
 * @brief can tx
 *
 */
#define TX_GPIO_NUM (gpio_num_t)26 // c++ won't allow int to be used as the gpio_num_t as it is more strict about the type of the variable
/**
 * @brief can rx
 *
 */
#define RX_GPIO_NUM (gpio_num_t)27

#define TRANSMIT_DELAY_MS 2000
/**
 * @brief can commands
 * 
 */
#define CAN_COMMAND_OTA_START 0xAAA
/******************************************variables****************************************************/

/***********************************************functions************************************************/
void initialize_canBus(void)
{
    /**
     * @brief  setting the bit rate of the can bus
     *
     */
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS();

    /**
     * @brief Accept every message from can bus
     *
     */
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    /**
     * @brief setting the configuration of the can bus
     *
     */
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_GPIO_NUM, RX_GPIO_NUM, TWAI_MODE_NORMAL);

    /**
     * @brief  Installing can bus
     *
     */
    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK)
    {
        ESP_LOGI(TAG, "can bus installed");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to install driver");
        return;
    }

    /**
     * @brief starting can bus
     *
     */
    if (twai_start() == ESP_OK)
    {
        ESP_LOGI(TAG, "Driver started\n");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to start driver\n");
        return;
    }
}

void can_tx(void *pvParameters)
{
    /**
     * @brief  sending the can message
     *
     */
    // Configure message to transmit
    twai_message_t message;
    message.identifier = CAN_COMMAND_OTA_START;
    message.extd = 1;
    message.data_length_code = 4;
    for (int i = 0; i < 4; i++)
    {
        message.data[i] = i;
    }

    while (1)
    {
        if (twai_transmit(&message,pdMS_TO_TICKS(1000)) == ESP_OK)
        {
            ESP_LOGI(TAG, "Message sent Sucessfully");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to send message");
        }
        vTaskDelay(pdMS_TO_TICKS(TRANSMIT_DELAY_MS));
    }
}

extern "C" void app_main(void)
{
    initialize_canBus();
    xTaskCreate(can_tx, "can_tx", 2048, NULL, 5, NULL);
}
