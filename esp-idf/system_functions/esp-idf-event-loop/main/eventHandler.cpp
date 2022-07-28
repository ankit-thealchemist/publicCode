#include"customEvent.h"

#include<esp_log.h>




#define TAG "customEventLibarary"


void custom_event_handler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data){

   ESP_LOGI(TAG, "Event handler called");

    switch (event_id)
    {
    case CUSTOM_EVENT_1:
        
        ESP_LOGI(TAG,"Event 1 received");
        break;
    case CUSTOM_EVENT_2:
        ESP_LOGI(TAG,"Event 2 received");
        break;
    
    case CUSTOM_EVENT_3:
        ESP_LOGI(TAG,"Event 3 received");
        break;
    
    case CUSTOM_EVENT_4:
        ESP_LOGI(TAG,"Event 4 received");
        break;

    
    default:
    // Unknown event should not happen
        break;
    }
}