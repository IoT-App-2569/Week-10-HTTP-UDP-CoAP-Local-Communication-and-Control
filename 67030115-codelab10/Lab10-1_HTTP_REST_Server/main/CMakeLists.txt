idf_component_register(SRCS "Lab10-1_HTTP_REST_Server.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_http_server mdns esp_wifi esp_event nvs_flash cjson esp_adc esp_driver_gpio)