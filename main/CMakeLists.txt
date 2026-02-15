idf_component_register(SRCS
    "blink_example_main.c"
    "wifi_task.c"
    "sensor_task.c"
    "led_task.c"
    "system_task.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES dht11 esp_wifi esp_event nvs_flash esp_netif
)
