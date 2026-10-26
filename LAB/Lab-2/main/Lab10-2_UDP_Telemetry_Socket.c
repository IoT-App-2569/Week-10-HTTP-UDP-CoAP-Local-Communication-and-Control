#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "nvs_flash.h"

#define TAG "UDP_LAB"

/* Wi-Fi settings from Lab 1. */
#define CONFIG_WIFI_SSID "Pornprom"
#define CONFIG_WIFI_PASSWORD "12345678"
#define MAXIMUM_RETRY 5

#define LED_GPIO_PIN GPIO_NUM_2
#define POT_ADC_CHANNEL ADC_CHANNEL_6 /* GPIO 34, ADC1 Channel 6 */

#define UDP_CONTROL_PORT 3333
#define UDP_BROADCAST_PORT 3334

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static EventGroupHandle_t s_wifi_event_group;
static adc_oneshot_unit_handle_t s_adc1_handle;
static int s_retry_num;

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT &&
               event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Retrying Wi-Fi connection (%d/%d)...",
                     s_retry_num, MAXIMUM_RETRY);
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            ESP_LOGE(TAG, "Failed to connect to Wi-Fi");
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Wi-Fi Connected! IP Address: " IPSTR,
                 IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static bool wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();
    if (s_wifi_event_group == NULL) {
        ESP_LOGE(TAG, "Unable to create Wi-Fi event group");
        return false;
    }

    if (esp_netif_create_default_wifi_sta() == NULL) {
        ESP_LOGE(TAG, "Unable to create default Wi-Fi station interface");
        return false;
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    size_t ssid_len = strlen(CONFIG_WIFI_SSID);
    if (ssid_len > sizeof(wifi_config.sta.ssid)) {
        ssid_len = sizeof(wifi_config.sta.ssid);
    }
    memcpy(wifi_config.sta.ssid, CONFIG_WIFI_SSID, ssid_len);

    size_t password_len = strlen(CONFIG_WIFI_PASSWORD);
    if (password_len > sizeof(wifi_config.sta.password)) {
        password_len = sizeof(wifi_config.sta.password);
    }
    memcpy(wifi_config.sta.password, CONFIG_WIFI_PASSWORD, password_len);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "Connecting to AP: %s...", CONFIG_WIFI_SSID);

    EventBits_t bits = xEventGroupWaitBits(
        s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE,
        pdFALSE, portMAX_DELAY);
    return (bits & WIFI_CONNECTED_BIT) != 0;
}

static void send_control_ack(int sock, const char *message,
                             const struct sockaddr_in *client_addr,
                             socklen_t client_addr_len)
{
    int result = sendto(sock, message, strlen(message), 0,
                        (const struct sockaddr *)client_addr, client_addr_len);
    if (result < 0) {
        ESP_LOGE(TAG, "Unable to send ACK: errno %d", errno);
    }
}

static void udp_control_server_task(void *pvParameters)
{
    char rx_buffer[128];
    struct sockaddr_in server_addr = {0};
    struct sockaddr_in client_addr = {0};
    socklen_t client_addr_len = sizeof(client_addr);

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create control socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(UDP_CONTROL_PORT);

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        ESP_LOGE(TAG, "Unable to bind control socket: errno %d", errno);
        close(sock);
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "Control Server listening on UDP Port %d...",
             UDP_CONTROL_PORT);

    while (true) {
        int length = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0,
                              (struct sockaddr *)&client_addr,
                              &client_addr_len);
        if (length < 0) {
            ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
            continue;
        }

        rx_buffer[length] = '\0';
        ESP_LOGI(TAG, "Received command: %s", rx_buffer);

        if (strcmp(rx_buffer, "LED_ON") == 0) {
            gpio_set_level(LED_GPIO_PIN, 1);
            send_control_ack(sock, "ACK:LED_ON", &client_addr,
                             client_addr_len);
        } else if (strcmp(rx_buffer, "LED_OFF") == 0) {
            gpio_set_level(LED_GPIO_PIN, 0);
            send_control_ack(sock, "ACK:LED_OFF", &client_addr,
                             client_addr_len);
        } else {
            send_control_ack(sock, "ACK:UNKNOWN", &client_addr,
                             client_addr_len);
        }
    }
}

static void udp_telemetry_broadcast_task(void *pvParameters)
{
    struct sockaddr_in destination = {0};
    destination.sin_addr.s_addr = inet_addr("255.255.255.255");
    destination.sin_family = AF_INET;
    destination.sin_port = htons(UDP_BROADCAST_PORT);

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create broadcast socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    int broadcast_enabled = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast_enabled,
                   sizeof(broadcast_enabled)) < 0) {
        ESP_LOGE(TAG, "Unable to enable broadcast: errno %d", errno);
        close(sock);
        vTaskDelete(NULL);
        return;
    }

    uint32_t sequence_number = 0;
    char tx_buffer[64];
    ESP_LOGI(TAG, "Broadcasting Telemetry to Port %d (10 Hz)...",
             UDP_BROADCAST_PORT);

    while (true) {
        int potentiometer_value = 0;
        if (s_adc1_handle != NULL) {
            esp_err_t result = adc_oneshot_read(
                s_adc1_handle, POT_ADC_CHANNEL, &potentiometer_value);
            if (result != ESP_OK) {
                ESP_LOGE(TAG, "Unable to read ADC: %s",
                         esp_err_to_name(result));
            }
        }

        int length = snprintf(tx_buffer, sizeof(tx_buffer), "SEQ:%lu,POT:%d\n",
                              (unsigned long)sequence_number++,
                              potentiometer_value);
        if (length < 0 || length >= sizeof(tx_buffer)) {
            ESP_LOGE(TAG, "Telemetry message is too long");
        } else if (sendto(sock, tx_buffer, length, 0,
                          (struct sockaddr *)&destination,
                          sizeof(destination)) < 0) {
            ESP_LOGE(TAG, "Unable to send telemetry: errno %d", errno);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void app_main(void)
{
    esp_err_t result = nvs_flash_init();
    if (result == ESP_ERR_NVS_NO_FREE_PAGES ||
        result == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        result = nvs_flash_init();
    }
    ESP_ERROR_CHECK(result);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    gpio_reset_pin(LED_GPIO_PIN);
    ESP_ERROR_CHECK(gpio_set_direction(LED_GPIO_PIN, GPIO_MODE_INPUT_OUTPUT));

    adc_oneshot_unit_init_cfg_t adc_init_config = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(
        adc_oneshot_new_unit(&adc_init_config, &s_adc1_handle));

    adc_oneshot_chan_cfg_t adc_channel_config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(
        s_adc1_handle, POT_ADC_CHANNEL, &adc_channel_config));
    ESP_LOGI(TAG, "ADC initialized on GPIO 34");

    if (!wifi_init_sta()) {
        ESP_LOGE(TAG, "Wi-Fi connection failed");
        return;
    }

    xTaskCreate(udp_control_server_task, "udp_control_task", 4096, NULL, 5,
                NULL);
    xTaskCreate(udp_telemetry_broadcast_task, "udp_telemetry_task", 4096,
                NULL, 4, NULL);
    ESP_LOGI(TAG, "Ready! Test UDP Telemetry with: python udp_listener.py");
}
