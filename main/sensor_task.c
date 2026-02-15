#include "common.h"
#include "dht11.h"
#include "esp_log.h"
#include "esp_task_wdt.h"


static const char *TAG = "SENSOR_TASK";

void sensor_task(void *pv)
{
    dht11_reading_t data;

    esp_task_wdt_add(NULL);

    while (1)
    {
        if (dht11_read(DHT_GPIO, &data) == ESP_OK)
        {
            ESP_LOGI(TAG, "Temperature %.1f C  Humidity %.1f%%",
                     data.temperature, data.humidity);
        }
        else
        {
            ESP_LOGW(TAG, "DHT read failed");
        }

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
