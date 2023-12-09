/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"

TaskHandle_t task_counter;
TaskHandle_t task_logging;
QueueHandle_t xQueue;

uint8_t SYSTEM_READY = 0 ;

typedef struct log_message_ {
    uint32_t data;
    uint32_t time;
} log_message;




volatile uint32_t GLOBAL_COUNTER = 0;

void vTask_Counter(void* parameter){
    uint32_t period = (uint32_t) parameter;

    while(!SYSTEM_READY) vTaskDelay(1000);
    printf("LOG-> TASK START\r\n");

    while(1){
        vTaskDelay(period);

        if(GLOBAL_COUNTER++ > 1000)
            GLOBAL_COUNTER  = 0 ;

        log_message* new_log = malloc(sizeof(log_message));
        *new_log =(log_message){GLOBAL_COUNTER  , 0};

        if (xQueueSend(xQueue,new_log,10) != pdPASS){
            free(new_log);
            printf("ERROR-> Queue  : Send incomplete\r\n");
        }

    }
}

void vTask_Logging(void* parametr){
    while (!SYSTEM_READY) vTaskDelay(1000);

    while (xQueue)
    {
        log_message rec_log;
        if (xQueueReceive(xQueue,&rec_log,10) == pdPASS){
            if (1){
                sprintf("DATA:%d,TIME:%d",(uint32_t)rec_log.data,(uint32_t)rec_log.time);
                free(&rec_log);
                }
            else
                printf("ERROR-> Queue : Receive Incomplete\r\n");
            }
    }
    
    
}

void app_main(void)
{

    if (xTaskCreate(vTask_Counter,"",256,(void*) 5000,1,task_counter) == pdFAIL){
        printf("ERROR-> Task Creation\r\n");
        vTaskDelete(task_counter);
        return;
    }
    if (xTaskCreate(vTask_Logging,"",256,(void*)NULL,1,task_logging) == pdFAIL){
        printf("ERROR-> Task Creation\r\n");
        vTaskDelete(task_logging);
        return;
    }
    xQueue = xQueueCreate(10,sizeof(log_message*));
    if(xQueue == NULL){
        printf("ERROR-> Queue Creation\r\n");
        return;
    }

    SYSTEM_READY = 1;
    printf("Hello world!\n");
}
