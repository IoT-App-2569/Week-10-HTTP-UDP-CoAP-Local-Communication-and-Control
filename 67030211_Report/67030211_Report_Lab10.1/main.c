#include <stdio.h>
#include "esp_http_server.h"
#include "mdns.h"

static esp_err_t get_handler(httpd_req_t *req) {
    const char* resp = "{\"device\":\"esp32-node\",\"temp\":26.5,\"led\":\"OFF\"}";
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

void app_main(void) {
    mdns_init();
    mdns_hostname_set("esp32-node");
    mdns_instance_name_set("ESP32 Local HTTP & mDNS Node");
    mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);

    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_start(&server, &config);
    httpd_uri_t uri_get = { .uri = "/api/status", .method = HTTP_GET, .handler = get_handler };
    httpd_register_uri_handler(server, &uri_get);
    printf("mDNS registered: http://esp32-node.local/api/status\n");
}
