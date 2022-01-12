/**
 * @file main.cpp
 * @author Ankit Bansal (iotdevelope@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-01-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <stdio.h>
#include "customEvent.h"
#include <esp_log.h>


#define TAG "customEvent"

/**
 * @brief Custom event base
 * 
 */
ESP_EVENT_DEFINE_BASE(CUSTOM_EVENT);  // defining the event base


void add_task(void *pvParamterts);


extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Starting the esp32 event loop");

    //  creating the default event handler
        ESP_ERROR_CHECK(esp_event_loop_create_default());

      // Registering the event handler
     ESP_ERROR_CHECK(esp_event_handler_instance_register(CUSTOM_EVENT, ESP_EVENT_ANY_ID, custom_event_handler, NULL, NULL));

      xTaskCreate(add_task, "add_task", 2048, NULL, 5, NULL);

}

/**
 * @brief virtual task to for creating the event
 * 
 * @param parameters 
 */
void add_task(void *parameters){

      ESP_LOGI(TAG, "Starting the add task");

         
   
      while(1){
   
         ESP_LOGI(TAG, "Adding the event");
   
         // Adding the event
         esp_event_post(CUSTOM_EVENT, CUSTOM_EVENT_1, NULL, 0, portMAX_DELAY);
   
         vTaskDelay(3000 / portTICK_PERIOD_MS);

         ESP_ERROR_CHECK(esp_event_post(CUSTOM_EVENT, CUSTOM_EVENT_2, NULL, 0, portMAX_DELAY));

         vTaskDelay(3000 / portTICK_PERIOD_MS);

         esp_event_post(CUSTOM_EVENT, CUSTOM_EVENT_3, NULL, 0, portMAX_DELAY);

         vTaskDelay(3000 / portTICK_PERIOD_MS);



      }
}

