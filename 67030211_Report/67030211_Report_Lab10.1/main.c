#include <stdio.h>
#include "esp_http_server.h"
#include "mdns.h"
void app_main(void) {
    mdns_init(); mdns_hostname_set("esp32-node");
    printf("HTTP Server active: http://esp32-node.local\n");
}
