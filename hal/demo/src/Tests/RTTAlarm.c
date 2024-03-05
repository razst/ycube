/*
 * RTTAlarm.c
 *
 *  Created on: 22 Apr 2020
 *      Author: asau
 */

#include <hal/Timing/Time.h>
#include <hal/Timing/RTT.h>
#include <hal/Utility/dbgu_int.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include "RTTAlarm.h"

xSemaphoreHandle _semaphore = NULL;

void alarm(void)
{
    signed portBASE_TYPE yield;
    xSemaphoreGiveFromISR(_semaphore, &yield);
    if (yield)
    {
        portYIELD_FROM_ISR();
    }
}

Boolean RTTAlarm(void)
{
    if (_semaphore == NULL)
    {
        vSemaphoreCreateBinary(_semaphore);
        xSemaphoreTake(_semaphore, portMAX_DELAY);
        RTT_SetAlarmInterrupt(alarm);
    }
    Time_start(NULL, 60);
    if (DBGU_IntIsRxReady() == 0)
    {
        vTaskDelay(1000);
    }

    unsigned int epoch;
    Time_getUnixEpoch(&epoch);
    printf("Setting alarm\n\r");
    RTT_SetAlarm(epoch + 10);

    if (xSemaphoreTake(_semaphore, 11000) == pdTRUE)
    {
        printf("ALARM!\n\r");
    }
    else
    {
        printf("Failed to get alarm!\n\r");
    }

    return TRUE;
}
