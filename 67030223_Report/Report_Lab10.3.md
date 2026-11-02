# Labsheet 10.3: CoAP Server & Lightweight Constrained IoT Control


#### 1. บันทึกผลการทดลอง / โค้ดที่พัฒนา

โค้ดภาษา C บน ESP-IDF (`Lab10-3_CoAP_Server.c`) พัฒนา CoAP Server บนพอร์ต UDP 5683 เพื่อเปิด Resource `/sensor/pot` (GET) และ `/actuator/led` (PUT) รองรับการควบคุมแบบ RESTful ผ่าน UDP:

```c
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "coap3/coap.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

#define LED_GPIO_PIN    GPIO_NUM_2
#define POT_ADC_CHANNEL ADC_CHANNEL_6

static adc_oneshot_unit_handle_t s_adc1_handle = NULL;

// Handler สำหรับ GET /sensor/pot
static void hnd_get_pot(coap_resource_t *resource,
                        coap_session_t *session,
                        const coap_pdu_t *request,
                        const coap_string_t *query,
                        coap_pdu_t *response)
{
    int pot_val = 0;
    if (s_adc1_handle != NULL) {
        adc_oneshot_read(s_adc1_handle, POT_ADC_CHANNEL, &pot_val);
    }
    char pot_str[32];
    snprintf(pot_str, sizeof(pot_str), "%d", pot_val);

    coap_pdu_set_code(response, COAP_RESPONSE_CODE_CONTENT); // 2.05 Content
    coap_add_data(response, strlen(pot_str), (const uint8_t *)pot_str);
    ESP_LOGI("CoAP", "Responded GET /sensor/pot: %s", pot_str);
}

// Handler สำหรับ PUT /actuator/led
static void hnd_put_led(coap_resource_t *resource,
                        coap_session_t *session,
                        const coap_pdu_t *request,
                        const coap_string_t *query,
                        coap_pdu_t *response)
{
    size_t size;
    const uint8_t *data;
    coap_get_data(request, &size, &data);

    if (size > 0) {
        if (data[0] == '1') {
            gpio_set_level(LED_GPIO_PIN, 1);
            ESP_LOGI("CoAP", "LED turned ON");
        } else if (data[0] == '0') {
            gpio_set_level(LED_GPIO_PIN, 0);
            ESP_LOGI("CoAP", "LED turned OFF");
        }
    }
    coap_pdu_set_code(response, COAP_RESPONSE_CODE_CHANGED); // 2.04 Changed
}

// CoAP Server Task
void coap_server_task(void *pvParameters)
{
    coap_context_t *ctx = NULL;
    coap_address_t serv_addr;

    coap_startup();
    coap_address_init(&serv_addr);
    serv_addr.addr.sin.sin_family = AF_INET;
    serv_addr.addr.sin.sin_addr.s_addr = INADDR_ANY;
    serv_addr.addr.sin.sin_port = htons(5683);

    ctx = coap_new_context(NULL);
    coap_new_endpoint(ctx, &serv_addr, COAP_PROTO_UDP);

    // ลงทะเบียน Resource /sensor/pot
    coap_resource_t *r_pot = coap_resource_init(coap_make_str_const("sensor/pot"), 0);
    coap_register_handler(r_pot, COAP_REQUEST_GET, hnd_get_pot);
    coap_add_resource(ctx, r_pot);

    // ลงทะเบียน Resource /actuator/led
    coap_resource_t *r_led = coap_resource_init(coap_make_str_const("actuator/led"), 0);
    coap_register_handler(r_led, COAP_REQUEST_PUT, hnd_put_led);
    coap_add_resource(ctx, r_led);

    ESP_LOGI("CoAP", "CoAP Server Ready on UDP 5683");

    while (1) {
        coap_io_process(ctx, 100);
    }
    coap_free_context(ctx);
    coap_cleanup();
    vTaskDelete(NULL);
}
```

#### 2. ผลลัพธ์จาก Serial Monitor / หน้าจอแสดงผล

ผลการรันสคริปต์ `test_coap.py` ผ่านไลบรารี `aiocoap` บนคอมพิวเตอร์:

```text
--- 1. Testing CoAP Resource Discovery (/.well-known/core) ---
Resource Directory (CoRE Link Format):
</sensor/pot>,</actuator/led>

--- 2. Testing CoAP GET /sensor/pot ---
Potentiometer Value: 2048 (Code: 2.05 Content)

--- 3. Testing CoAP PUT /actuator/led (Turn ON) ---
LED ON Response Code: 2.04 Changed

--- 4. Testing CoAP PUT /actuator/led (Turn OFF) ---
LED OFF Response Code: 2.04 Changed
```

ผลการทำงานบน Serial Monitor ฝั่ง ESP32:
```text
I (2450) CoAP: CoAP Server Ready on UDP 5683
I (5610) CoAP: Responded GET /sensor/pot: 2048
I (7820) CoAP: LED turned ON
I (9840) CoAP: LED turned OFF
```

#### 3. สรุปผลและการตอบคำถามท้ายใบงาน

- **สรุปผลการทดลอง:**
สามารถสร้าง CoAP Server (Constrained Application Protocol) บน ESP32 ด้วยไลบรารี libcoap ผ่านโพรโทคอล UDP พอร์ต 5683 ได้สำเร็จ โดยระบบรองรับสถาปัตยกรรม RESTful ในระดับอุปกรณ์ขนาดเล็ก (Resource-Constrained Devices) เปิดบริการ Resource /sensor/pot ด้วยเมธอด GET เพื่ออ่านค่าเซนเซอร์แอนะล็อก พร้อมตอบกลับด้วยรหัส 2.05 Content และ Resource /actuator/led ด้วยเมธอด PUT เพื่อรับเพย์โหลดควบคุมการเปิด-ปิดไฟ LED พร้อมตอบกลับด้วยรหัส 2.04 Changed นอกจากนี้ยังรองรับกลไกค้นหาทรัพยากรอัตโนมัติ (Resource Discovery) ผ่านเส้นทางมาตรฐาน /.well-known/core ในรูปแบบ CoRE Link Format ได้อย่างสมบูรณ์ ช่วยลดภาระ Overhead และประหยัดพลังงานเมื่อเทียบกับ HTTP แบบดั้งเดิม

- **คำถามท้ายการทดลอง:**
  1. **นำผลการ Query `/.well-known/core` มาแสดงในรายงาน พร้อมอธิบายรูปแบบ CoRE Link Format (RFC 6690) ว่าแสดงข้อมูลทรัพยากรอย่างไร**
     - **คำตอบ:** การอธิบายรูปแบบ CoRE Link Format (RFC 6690):

CoRE Link Format เป็นมาตรฐานที่พัฒนาขึ้นเพื่อให้ระบบหรือไคลเอนต์สามารถค้นหารายการทรัพยากร (Resource Discovery) บนอุปกรณ์ฝังตัวขนาดเล็กได้อัตโนมัติ
1.โครงสร้างพื้นฐานของพาธ URI (<URI-Reference>):
แต่ละทรัพยากรจะถูกระบุเส้นทางโดยครอบด้วยเครื่องหมาย Bracket < > เช่น </sensor/pot> และ </actuator/led>

2.แอตทริบิวต์กำกับทรัพยากร (Target Attributes - ส่วนขยาย):
สามารถระบุคุณสมบัติเพิ่มเติมต่อท้ายแต่ละพาธด้วยเครื่องหมายเซมิโคลอน (;) เพื่อบอกประเภทและพฤติกรรม

3.จุดเด่นเชิงวิศวกรรม: มีรูปแบบกะทัดรัด (Lightweight) ใช้ข้อความขนาดสั้นเพื่อลดขนาดไบต์บนแพ็กเก็ต UDP ให้เหลือน้อยที่สุด ช่วยประหยัดแบนด์วิดท์และลดภาระการแยกวิเคราะห์ (Parsing) ของไมโครคอนโทรลเลอร์ที่มีทรัพยากรจำกัด

2. **อธิบายความแตกต่างของแพ็กเก็ต CoAP ระหว่าง CON (Confirmable) และ NON (Non-confirmable) เมื่อทดสอบในเครือข่ายที่มีการรบกวนสัญญาณ**
     - **คำตอบ:** CON (Confirmable):

ต้องมี ACK ตอบกลับ
สัญญาณรบกวน/แพ็กเก็ตหาย จะส่งซ้ำอัตโนมัติ (Retransmit) จนกว่าจะสำเร็จ ข้อมูลไม่สูญหายแต่ Latency เพิ่มขึ้น เหมาะกับคำสั่งสำคัญ (เช่น คุมรีเลย์/สวิตช์)

NON (Non-confirmable):
ไม่มี ACK (Fire-and-forget)
สัญญาณรบกวน/แพ็กเก็ตหาย จะทิ้งทันที ไม่ส่งซ้ำ ประหยัดแบนด์วิดท์ เหมาะกับการสตรีมข้อมูลเซนเซอร์ความถี่สูงที่ตกหล่นได้บ้าง

3. **ทำไม CoAP จึงเหมาะสมกับโปรโตคอลการสื่อสารบนเครือข่ายเช่น Thread, Zigbee IP หรือ NB-IoT มากกว่า HTTP?**
     - **คำตอบ:** Header จิ๋วมาก (4 ไบต์): ไบนารีขนาดเล็กกว่าข้อความยาวๆ ของ HTTP มหาศาล ประหยัดเน็ต/ค่าแอร์ไทม์

รันบน UDP: ไม่ต้องต่อ TCP Handshake ส่งเสร็จหลับต่อได้ทันที กินไฟแบตเตอรี่น้อยลง

ไม่แตกแพ็กเก็ต (No Fragmentation): ขนาดเล็กพอดีกับขีดจำกัดเฟรมส่งของ Thread/Zigbee (127 ไบต์) และ NB-IoT ไม่เสี่ยงข้อมูลหล่นหายจากการซอยแพ็กเก็ต

กินสเปกต่ำ: ตัวจัดการและโค้ดใช้ RAM/ROM บนชิปจิ๋วน้อยกว่ามาก