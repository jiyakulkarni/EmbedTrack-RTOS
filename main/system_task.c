#include "common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_task_wdt.h"

static const char *TAG = "SYSTEM_TASK";

void system_task(void *pvParameters)
{
    esp_task_wdt_add(NULL);

    while (1)
    {
        esp_task_wdt_reset();

        ESP_LOGI(TAG, "System Alive : Watchdog OK");

        vTaskDelay(pdMS_TO_TICKS(8000));
    }
}

