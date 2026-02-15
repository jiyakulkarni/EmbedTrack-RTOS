#include "common.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "wifi.h"
#include "nvs_flash.h"
static const char *TAG = "MAIN";
extern void wifi_init(void);
extern void wifi_status_task(void *);
extern void mqtt_app_start(void);

void app_main(void)
{
    ESP_LOGI(TAG, "RTOS Project Starting");

    esp_task_wdt_config_t cfg = {
        .timeout_ms = 8000,
        .trigger_panic = false
    };
    nvs_flash_init();
    wifi_init();
    xTaskCreate(led_task, "LED_TASK", 4096, NULL, 5, NULL);
    xTaskCreate(sensor_task, "SENSOR_TASK", 4096, NULL, 5, NULL);
    xTaskCreate(system_task, "SYSTEM_TASK", 4096, NULL, 5, NULL);
    xTaskCreate(wifi_status_task, "WIFI_TASK", 4096, NULL, 5, NULL);
}
