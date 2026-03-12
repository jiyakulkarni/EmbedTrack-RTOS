#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "mqtt_client.h"
#include "esp_task_wdt.h"
#include "esp_netif.h"
#include "esp_random.h"
#define WIFI_SSID "jiyak"
#define WIFI_PASS "12345678"
#include "common.h"

system_state_t system_status = SYSTEM_WARNING;
static const char *TAG = "MAIN";

SemaphoreHandle_t resource1;
SemaphoreHandle_t resource2;

esp_mqtt_client_handle_t client;

int reboot_counter = 0;



void print_reset_reason()
{
    esp_reset_reason_t reason = esp_reset_reason();

    if(reason == ESP_RST_TASK_WDT)
        ESP_LOGI("RESET","Reset due to Watchdog");
    else
        ESP_LOGI("RESET","Other Reset Reason: %d",reason);
}



static void mqtt_event_handler(void *handler_args,
                               esp_event_base_t base,
                               int32_t event_id,
                               void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;

    switch(event->event_id)
    {

        case MQTT_EVENT_CONNECTED:
            ESP_LOGI("MQTT","MQTT CONNECTED");
            system_status = SYSTEM_OK;
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI("MQTT","Reconnecting...");
            system_status = SYSTEM_WARNING;
            esp_mqtt_client_reconnect(client);
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGI("MQTT","MQTT ERROR");
            system_status = SYSTEM_ERROR;
            break;

        default:
            break;
    }
}



void wifi_event_handler(void* arg,
                        esp_event_base_t event_base,
                        int32_t event_id,
                        void* event_data)
{

    if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
         system_status = SYSTEM_WARNING;
         esp_wifi_connect();
    }
    else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
    system_status = SYSTEM_WARNING;
    esp_wifi_connect();
    }
    else if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ESP_LOGI("WIFI","WiFi Connected");
        system_status = SYSTEM_WARNING;
        esp_mqtt_client_start(client);
    }
}



void wifi_init()
{
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_register(WIFI_EVENT,
                               ESP_EVENT_ANY_ID,
                               &wifi_event_handler,
                               NULL);

    esp_event_handler_register(IP_EVENT,
                               IP_EVENT_STA_GOT_IP,
                               &wifi_event_handler,
                               NULL);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS
        }
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
}



void system_task(void *pv)
{
    esp_task_wdt_add(NULL);

    while(1)
    {
        ESP_LOGI("SYSTEM_TASK","System Alive : Watchdog OK");
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}



void led_task_deadlock(void *pv)
{
    esp_task_wdt_add(NULL);
gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
    while(1)
    {
        ESP_LOGI("LED_TASK","LED Task trying resource2");

        xSemaphoreTake(resource2,portMAX_DELAY);

        ESP_LOGI("LED_TASK","LED Task got resource2");
 gpio_set_level(GPIO_NUM_2, 1);

        vTaskDelay(pdMS_TO_TICKS(1000));

        ESP_LOGI("LED_TASK","LED Task waiting for resource1");

        xSemaphoreTake(resource1,portMAX_DELAY);
    }
}



void sensor_task_deadlock(void *pv)
{
    esp_task_wdt_add(NULL);

    while(1)
    {
        ESP_LOGI("SENSOR_TASK","Sensor Task trying resource1");

        xSemaphoreTake(resource1,portMAX_DELAY);

        ESP_LOGI("SENSOR_TASK","Sensor Task got resource1");

        vTaskDelay(1000/portTICK_PERIOD_MS);

        ESP_LOGI("SENSOR_TASK","Sensor Task waiting for resource2");

        xSemaphoreTake(resource2,portMAX_DELAY);
    }
}

void led_task_safe(void *pv)
{
    esp_task_wdt_add(NULL);

    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);

    while(1)
    {
        ESP_LOGI("LED_TASK","Safe mode trying resource1");

        xSemaphoreTake(resource1, portMAX_DELAY);

        ESP_LOGI("LED_TASK","Safe mode got resource1");

        vTaskDelay(pdMS_TO_TICKS(500));

        xSemaphoreTake(resource2, portMAX_DELAY);

        ESP_LOGI("LED_TASK","Safe mode got resource2");

        /* -------- LED behaviour based on system state -------- */

        if(system_status == SYSTEM_WARNING)
        {
            /* WARNING : slow blink */

            gpio_set_level(GPIO_NUM_2,1);
            vTaskDelay(pdMS_TO_TICKS(50));

            gpio_set_level(GPIO_NUM_2,0);
            vTaskDelay(pdMS_TO_TICKS(50));
        }

        else if(system_status == SYSTEM_OK)
        {
            /* OK : LED ON */

            gpio_set_level(GPIO_NUM_2,1);
            vTaskDelay(pdMS_TO_TICKS(500));
        }

        else if(system_status == SYSTEM_ERROR)
        {
            /* ERROR : extreme blink */
            gpio_set_level(GPIO_NUM_2,0);
            vTaskDelay(pdMS_TO_TICKS(500));
        }

        xSemaphoreGive(resource2);
        xSemaphoreGive(resource1);

        ESP_LOGI("LED_TASK","Safe mode work complete");

        esp_task_wdt_reset();

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}


void sensor_task_safe(void *pv)
{
    esp_task_wdt_add(NULL);

    while(1)
    {

        ESP_LOGI("SENSOR_TASK","Safe mode trying resource1");

        xSemaphoreTake(resource1,portMAX_DELAY);

        ESP_LOGI("SENSOR_TASK","Safe mode got resource1");

        vTaskDelay(500/portTICK_PERIOD_MS);

        xSemaphoreTake(resource2,portMAX_DELAY);

        ESP_LOGI("SENSOR_TASK","Safe mode got resource2");

        int temp = 25 + (esp_random()%10);

        char msg[50];

        sprintf(msg,"Temperature = %d C",temp);

        ESP_LOGI("SENSOR_TASK","%s",msg);

        if(client)
        {
            esp_mqtt_client_publish(client,
                                    "esp32/temperature",
                                    msg,
                                    0,
                                    1,
                                    0);

            ESP_LOGI("MQTT","Published: %s",msg);
        }

        xSemaphoreGive(resource2);
        xSemaphoreGive(resource1);

        ESP_LOGI("SENSOR_TASK","Safe mode work complete");

        esp_task_wdt_reset();

        vTaskDelay(3000/portTICK_PERIOD_MS);
    }
}


void app_main(void)
{
    print_reset_reason();

    ESP_LOGI(TAG,"RTOS Project Starting");

    esp_reset_reason_t reason = esp_reset_reason();

    if(reason == ESP_RST_TASK_WDT)
        reboot_counter++;


    if(reboot_counter == 0)
        ESP_LOGI("MODE","NORMAL MODE : Deadlock Demonstration");
    else
        ESP_LOGI("MODE","RECOVERY MODE : Deadlock Prevention");


    esp_task_wdt_config_t wdt_config = {
        .timeout_ms = 5000,
        .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
        .trigger_panic = true
    };

    esp_task_wdt_deinit();
    esp_task_wdt_init(&wdt_config);


    ESP_ERROR_CHECK(nvs_flash_init());

    wifi_init();


    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://broker.hivemq.com",
        .session.keepalive = 60
    };

    client = esp_mqtt_client_init(&mqtt_cfg);

    esp_mqtt_client_register_event(client,
                                   ESP_EVENT_ANY_ID,
                                   mqtt_event_handler,
                                   NULL);


    resource1 = xSemaphoreCreateMutex();
    resource2 = xSemaphoreCreateMutex();


    xTaskCreate(system_task,"SYSTEM_TASK",4096,NULL,5,NULL);


    if(reboot_counter == 0)
    {
        xTaskCreate(led_task_deadlock,"LED_TASK",4096,NULL,5,NULL);
        xTaskCreate(sensor_task_deadlock,"SENSOR_TASK",4096,NULL,5,NULL);
    }
    else
    {
        xTaskCreate(led_task_safe,"LED_TASK",4096,NULL,5,NULL);
        xTaskCreate(sensor_task_safe,"SENSOR_TASK",4096,NULL,5,NULL);
    }
}
