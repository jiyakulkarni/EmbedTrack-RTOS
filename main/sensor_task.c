#include "common.h"
#include "dht11.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "freertos/semphr.h"

/* Access mutex resources created in main file */
extern SemaphoreHandle_t resource1;
extern SemaphoreHandle_t resource2;

static const char *TAG = "SENSOR_TASK";

void sensor_task(void *pv)
{
    dht11_reading_t data;

    /* Register task with watchdog */
    esp_task_wdt_add(NULL);

    while (1)
    {
        ESP_LOGI(TAG, "Sensor Task trying resource1");

        xSemaphoreTake(resource1, portMAX_DELAY);

        ESP_LOGI(TAG, "Sensor Task got resource1");

        /* Delay to allow LED task to grab resource2 */
        vTaskDelay(pdMS_TO_TICKS(1000));

        ESP_LOGI(TAG, "Sensor Task waiting for resource2");

        /* Deadlock point */
        xSemaphoreTake(resource2, portMAX_DELAY);

        if (dht11_read(DHT_GPIO, &data) == ESP_OK)
        {
            ESP_LOGI(TAG, "Temperature %.1f C  Humidity %.1f%%",
                     data.temperature, data.humidity);
        }
        else
        {
            ESP_LOGW(TAG, "DHT read failed");
        }

        /* Release resources (never reached during deadlock) */
        xSemaphoreGive(resource2);
        xSemaphoreGive(resource1);

        esp_task_wdt_reset();

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
