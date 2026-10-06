idf_component_register(SRCS "Lab10-3_CoAP_Server.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_wifi esp_event nvs_flash coap esp_adc esp_driver_gpio)
