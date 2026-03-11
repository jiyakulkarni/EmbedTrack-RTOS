#include "common.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "freertos/semphr.h"

/* Access mutex resources created in main */
extern SemaphoreHandle_t resource1;
extern SemaphoreHandle_t resource2;

static const char *TAG = "LED_TASK";

void led_task(void *pv)
{
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    esp_task_wdt_add(NULL);

    while (1)
    {
        ESP_LOGI(TAG, "LED Task trying resource2");

        xSemaphoreTake(resource2, portMAX_DELAY);

        ESP_LOGI(TAG, "LED Task got resource2");

        vTaskDelay(pdMS_TO_TICKS(1000));

        ESP_LOGI(TAG, "LED Task waiting for resource1");

        /* Deadlock occurs here */
        xSemaphoreTake(resource1, portMAX_DELAY);

        gpio_set_level(LED_GPIO, 1);
        ESP_LOGI(TAG, "LED ON");

        esp_task_wdt_reset();

        vTaskDelay(pdMS_TO_TICKS(1000));

        gpio_set_level(LED_GPIO, 0);
        ESP_LOGI(TAG, "LED OFF");

        xSemaphoreGive(resource1);
        xSemaphoreGive(resource2);

        esp_task_wdt_reset();

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
