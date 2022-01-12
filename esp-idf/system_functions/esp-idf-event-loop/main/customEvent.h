#ifndef __CUSTOMEVENT_H__
#define __CUSTOMEVENT_H__



#include <esp_event.h>

#ifdef __cplusplus
extern "C" {
#endif


ESP_EVENT_DECLARE_BASE(CUSTOM_EVENT);

enum{
    CUSTOM_EVENT_1,
    CUSTOM_EVENT_2,
    CUSTOM_EVENT_3,
    CUSTOM_EVENT_4
};


void custom_event_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data);


#ifdef __cplusplus
}
#endif



#endif // __CUSTOMEVENT_H__