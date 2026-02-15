#pragma once
#include "driver/gpio.h"
#include "esp_wifi.h"
#include "dht11.h"

#define LED_GPIO GPIO_NUM_2
#define DHT_GPIO 5

void led_task(void *pv);
void sensor_task(void *pv);
void system_task(void *pv);
void wifi_status_task(void *pv);
