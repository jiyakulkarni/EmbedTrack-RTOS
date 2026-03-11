#include "common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_task_wdt.h"

static const char *TAG = "SYSTEM_TASK";

void system_task(void *pvParameters)
{
    /* Register task with watchdog */
    esp_task_wdt_add(NULL);

    while (1)
    {
        ESP_LOGI(TAG, "System Alive : Watchdog OK");
        /* Feed watchdog */
        esp_task_wdt_reset();

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
