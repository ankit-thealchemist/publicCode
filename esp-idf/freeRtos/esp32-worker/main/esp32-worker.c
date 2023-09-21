#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <freertos/queue.h>



typedef void (*worker)(void *arg1, void* arg2); 

QueueHandle_t worker_queue = NULL;

void worker_1(void *arg1,void *arg2)
{
    printf("I am worker 1 \n");
}


void worker_2(void *arg1,void *arg2)
{
    printf("I am worker 2 \n");
}

void worker_default(void *arg1,void *arg2)
{
    printf("I am defalut worker \n");
}


void worker_task(void *param)
{   
    worker my_worker = NULL;
    while (1)
    {   
        printf("waiting for the work\r\n");
        if(pdFALSE != xQueueReceive(worker_queue,&my_worker,pdMS_TO_TICKS(1000)))
        {
            if(NULL != my_worker)
            {
                my_worker(NULL,NULL);
            }
        }

        else
        {
            worker_default(NULL,NULL);
        }
    }
    
}


void app_main(void)
{   
    worker_queue = xQueueCreate(5,sizeof(worker));
    assert(NULL != worker_queue);
    xTaskCreate(worker_task,"work_task",4*1024,NULL,8,NULL);
    
    

    worker work = NULL;

    work = worker_1;

    xQueueSend(worker_queue,&work,portMAX_DELAY);

    vTaskDelay(pdMS_TO_TICKS(5000));

    work = worker_2;

    xQueueSend(worker_queue,&work,portMAX_DELAY);

}
