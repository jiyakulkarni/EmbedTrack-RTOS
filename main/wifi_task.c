#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_task_wdt.h"

#define WIFI_SSID "jiyak"
#define WIFI_PASS "12345678"

#define WIFI_CONNECTED_BIT BIT0

static EventGroupHandle_t wifi_event_group;
static const char *TAG = "WIFI_TASK";
void mqtt_app_start(void);

/* ---------------- WiFi Event Handler ---------------- */
static void wifi_event_handler(void* arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void* event_data)
{
    if (event_base == WIFI_EVENT)
    {
        switch(event_id)
        {
            case WIFI_EVENT_STA_START:
                esp_wifi_connect();
                break;

            case WIFI_EVENT_STA_DISCONNECTED:
                esp_wifi_connect();
                ESP_LOGI("WIFI_TASK", "Reconnecting...");
                break;
        }
    }

    if (event_base == IP_EVENT)
    {
        switch(event_id)
        {
            case IP_EVENT_STA_GOT_IP:
                ESP_LOGI("WIFI_TASK", "WiFi Connected");
                mqtt_app_start();
                break;
        }
    }
}

/* --------------- WiFi Initialization ---------------- */

void wifi_init(void)
{
    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &wifi_event_handler,
            NULL,
            NULL));

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &wifi_event_handler,
            NULL,
            NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(
        esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}


/* ---------------- WiFi Status Task ---------------- */

void wifi_status_task(void *pv)
{
    esp_task_wdt_add(NULL);

    bool last_state = false;

    while (1)
    {
        esp_task_wdt_reset();

        if (wifi_event_group == NULL)
        {
            ESP_LOGW(TAG, "Event group not ready");
            vTaskDelay(pdMS_TO_TICKS(3000));
            continue;
        }

        bool connected =
            (xEventGroupGetBits(wifi_event_group)
             & WIFI_CONNECTED_BIT) != 0;

        if (connected != last_state)
        {
            if (connected)
                ESP_LOGI(TAG, "WiFi Connected");
            else
                ESP_LOGW(TAG, "WiFi Not Connected");

            last_state = connected;
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

