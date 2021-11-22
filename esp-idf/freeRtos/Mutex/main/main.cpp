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
 * @brief declaring the handle for mutex
 * 
 */
SemaphoreHandle_t mutex;

extern "C" void app_main();

void task_one(void *parameters)
{

    while (1)
    {   
        ESP_LOGI(TAG,"-----------------------------------------------------------------------------\n");
        ESP_LOGI(TAG, "capturing the semaphore  %s", __func__);
        if (xSemaphoreTake(mutex, pdMS_TO_TICKS(1)) == pdTRUE)
        {
            ESP_LOGI(TAG, "I am the task One. I have semaphore \n");
           
            xSemaphoreGive(mutex);
        }

        else
        {
            ESP_LOGI(TAG, "unable to accquire the mutex  %s",__func__);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    vTaskDelete(NULL);
}

void task_two(void *parameters)
{

    while (1)
    {   
         ESP_LOGI(TAG,"-----------------------------------------------------------------------------\n");
        ESP_LOGI(TAG, "capturing the semaphore  %s", __func__);
        if (xSemaphoreTake(mutex, pdMS_TO_TICKS(1)) == pdTRUE)
        {
            ESP_LOGI(TAG, "I am the task two. I have semaphore \n");
            ESP_LOGI(TAG,"blocking the task for 3 seconds and doing the work\n");
            vTaskDelay(3000);
           
            xSemaphoreGive(mutex);
        }

        else
        {
            ESP_LOGI(TAG, "unable to accquire the mutex %s",__func__);
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }

    vTaskDelete(NULL);
}

void app_main()
{
    mutex = xSemaphoreCreateMutex();
    xTaskCreate(task_one, __func__, TASK_SIZE, NULL, DEFAULT_PRIORITY, NULL);
    xTaskCreate(task_two, __func__, TASK_SIZE, NULL, DEFAULT_PRIORITY, NULL);
}