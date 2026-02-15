#include "common.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_task_wdt.h"



static const char *TAG = "LED_TASK";

void led_task(void *pv)
{
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    esp_task_wdt_add(NULL);

    while (1)
    {
        gpio_set_level(LED_GPIO, 1);
        ESP_LOGI(TAG, "LED ON");
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(1000));

        gpio_set_level(LED_GPIO, 0);
        ESP_LOGI(TAG, "LED OFF");
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
