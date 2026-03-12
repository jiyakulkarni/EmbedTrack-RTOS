#include "common.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

/* Access mutex resources created in main */
extern SemaphoreHandle_t resource1;
extern SemaphoreHandle_t resource2;

static const char *TAG = "LED_TASK";

/* SAFE LED TASK (used after watchdog recovery) */
void led_task_safe(void *pvParameters)
{
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    while (1)
    {
        if(system_status == SYSTEM_OK)
        {
            /* System healthy -> LED ON */
            gpio_set_level(LED_GPIO, 1);
            ESP_LOGI(TAG, "SYSTEM OK - LED ON");
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        else if(system_status == SYSTEM_WARNING)
        {
            /* Warning -> Slow blink */
            ESP_LOGI(TAG, "SYSTEM WARNING - Slow Blink");

            gpio_set_level(LED_GPIO, 1);
            vTaskDelay(pdMS_TO_TICKS(1000));

            gpio_set_level(LED_GPIO, 0);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        else if(system_status == SYSTEM_ERROR)
        {
            /* Error -> Fast blink */
            ESP_LOGI(TAG, "SYSTEM ERROR - Fast Blink");

            gpio_set_level(LED_GPIO, 1);
            vTaskDelay(pdMS_TO_TICKS(200));

            gpio_set_level(LED_GPIO, 0);
            vTaskDelay(pdMS_TO_TICKS(200));
        }
    }
}


/* DEADLOCK DEMONSTRATION TASK (used on first boot) */
void led_task_deadlock(void *pv)
{
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    while (1)
    {
        ESP_LOGI(TAG, "LED Task trying resource2");

        xSemaphoreTake(resource2, portMAX_DELAY);

        ESP_LOGI(TAG, "LED Task got resource2");

        vTaskDelay(pdMS_TO_TICKS(1000));

        ESP_LOGI(TAG, "LED Task waiting for resource1");

        /* Deadlock happens here */
        xSemaphoreTake(resource1, portMAX_DELAY);

        gpio_set_level(LED_GPIO, 1);
        ESP_LOGI(TAG, "LED ON");

        vTaskDelay(pdMS_TO_TICKS(1000));

        gpio_set_level(LED_GPIO, 0);
        ESP_LOGI(TAG, "LED OFF");

        xSemaphoreGive(resource1);
        xSemaphoreGive(resource2);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
