#include "mqtt_client.h"
#include "esp_log.h"

static const char *TAG = "MQTT";

void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://broker.hivemq.com"
    };

    esp_mqtt_client_handle_t client =
        esp_mqtt_client_init(&mqtt_cfg);

    if (client == NULL)
    {
        ESP_LOGE(TAG, "MQTT client init failed");
        return;
    }

    esp_mqtt_client_start(client);
    ESP_LOGI(TAG, "MQTT Started Successfully");
}
