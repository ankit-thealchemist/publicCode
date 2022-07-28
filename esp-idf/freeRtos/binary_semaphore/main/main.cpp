#include <iostream>
#include <esp_system.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_spiffs.h>
#include <fstream>
#include <string>
#include <array>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#define TAG "example"
#define DEFAULT_PRIORITY 5

#define TASK_SIZE 2048
/**
 * @brief declaring the handle for binary semaphore
 * 
 */
SemaphoreHandle_t binary_semaphore;

extern "C" void app_main();

void task_one(void *parameters)
{

    while (1)
    {   
        ESP_LOGI(TAG,"capturing the semaphore from %s",__func__);
        xSemaphoreTake(binary_semaphore, portMAX_DELAY);
        {
            ESP_LOGI(TAG, "I am the task One. I have semaphore \n");
        }

        xSemaphoreGive(binary_semaphore);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    vTaskDelete(NULL);
}

void task_two(void *parameters)
{

    while (1)
    {   
        ESP_LOGI(TAG,"capturing the semaphore from %s",__func__);
        xSemaphoreTake(binary_semaphore, portMAX_DELAY);
        {
            ESP_LOGI(TAG, "I am the task two. I have semaphore\n");
        }

        xSemaphoreGive(binary_semaphore);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }

    vTaskDelete(NULL);
}

void app_main()
{
    binary_semaphore = xSemaphoreCreateBinary();
    xTaskCreate(task_one, __func__, TASK_SIZE, NULL, DEFAULT_PRIORITY, NULL);
    xTaskCreate(task_two, __func__, TASK_SIZE, NULL, DEFAULT_PRIORITY, NULL);
    xSemaphoreGive(binary_semaphore);
}