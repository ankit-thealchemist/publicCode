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
#include "freertos/event_groups.h"
#include <iostream>
#include <thread>

#define EVENT_ONE (1UL << 0UL)

EventGroupHandle_t handleEvents;

static void example_task(void *pvParameters)
{

    EventBits_t uBits= xEventGroupWaitBits(handleEvents,
                        EVENT_ONE,
                        pdFALSE,
                        pdFALSE,
                        portMAX_DELAY);
    std::cout<<"Event bits: "<<uBits<<std::endl;

    uBits= xEventGroupWaitBits(handleEvents,
                        EVENT_ONE,
                        pdTRUE,
                        pdFALSE,
                        portMAX_DELAY);
    std::cout<<"Event bits: "<<uBits<<std::endl;

    vTaskDelete(NULL);
}

static void thread_func(){
    std::cout<<"Thread started"<<std::endl;
}

extern "C" void app_main(void)
{
    std::thread t(thread_func);
    t.detach();
    handleEvents = xEventGroupCreate();
    vTaskDelay(pdMS_TO_TICKS(2000));
    xTaskCreate(&example_task, "example_task", 2048, NULL, 7, NULL);
    xEventGroupSetBits(handleEvents, EVENT_ONE);

}
