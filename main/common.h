#pragma once

#include "driver/gpio.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "dht11.h"

/* GPIO configuration */

#define LED_GPIO GPIO_NUM_2
#define DHT_GPIO 5


/* System status states */
typedef enum
{
    SYSTEM_WARNING = 0,
    SYSTEM_OK,
    SYSTEM_ERROR

} system_state_t;

/* Global system status */
extern system_state_t system_status;

/* Mutex resources shared between tasks */

extern SemaphoreHandle_t resource1;
extern SemaphoreHandle_t resource2;


/* Task declarations */

void led_task_deadlock(void *pv);
void sensor_task_deadlock(void *pv);

void led_task_safe(void *pv);
void sensor_task_safe(void *pv);

void system_task(void *pv);
void wifi_status_task(void *pv);
