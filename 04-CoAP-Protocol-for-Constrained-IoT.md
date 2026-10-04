# 10.4 โปรโตคอล CoAP (Constrained Application Protocol ) สำหรับระบบสมองกลฝังตัวทรัพยากรจำกัด 

## 10.4.1 จุดกำเนิดและแนวคิดของ CoAP 

ในการพัฒนาเทคโนโลยี IoT อุปกรณ์ส่วนใหญ่เป็นระบบสมองกลฝังตัวขนาดเล็กที่มีข้อจำกัดด้านทรัพยากรอย่างมาก (**Resource-Constrained Devices**) เช่น มีหน่วยความจำ RAM เพียงไม่กี่ร้อยกิโลไบต์, Flash Memory ขนาดจำกัด, ซีพียูความเร็วต่ำ และมักทำงานบนเครือข่ายไร้สายพลังงานต่ำที่มีแบนด์วิดท์จำกัด (Low-power and Lossy Networks - LLNs)

หากเรานำโปรโตคอล **TCP** และ **HTTP** มาใช้งานในการรับส่งข้อมูล จะต้องใช้หน่วยความจำและแบนด์วิดท์เครือข่ายสูงมาก (ทั้งขนาดส่วนหัวระดับหลายร้อยไบต์ และภาระการเชื่อมต่อแบบ Persistent Connection) ในทางกลับกัน หากเปลี่ยนไปใช้ **UDP Socket** ธรรมดา แม้จะได้ความเร็วและประหยัดแบนด์วิดท์ แต่ก็ขาดโครงสร้างระดับแอปพลิเคชัน (ไม่มีสถาปัตยกรรมแบบ REST, ไม่มี URI ระบุทรัพยากร และไม่การันตีความถูกต้องของข้อมูล)

***คำถามสำคัญคือ** มีโปรโตคอลระดับแอปพลิเคชันที่ทำงานบน UDP แต่มีรูปแบบสถาปัตยกรรมแบบ REST เหมือนกับ HTTP หรือไม่*

คำตอบคือ **CoAP (Constrained Application Protocol)** ซึ่งได้รับการกำหนดขึ้นโดยคณะทำงาน IETF ในมาตรฐาน **RFC 7252** เพื่อเป็นโปรโตคอลเว็บสำหรับอุปกรณ์ IoT โดยเฉพาะ

<p align="center">
<!-- [รูปภาพ: การเปรียบเทียบ Protocol Stack ระหว่าง HTTP/TCP และ CoAP/UDP] -->
<!-- <img src="Images/http_vs_coap_stack.svg" width="600"> -->
</p>

### คุณลักษณะเด่นของ CoAP (อ้างอิง Chapter 8.3.4 ในหนังสือ)
1. **ออกแบบตามสถาปัตยกรรม REST ของ HTTP** 
   
   ทรัพยากรบนเซิร์ฟเวอร์ถูกระบุด้วย URI เช่น `coap://192.168.3.80/light` และรองรับ 4 เมธอดมาตรฐาน ได้แก่ **GET, PUT, POST, และ DELETE**
1. **ส่วนหัวแบบไบนารีกะทัดรัด (Lightweight Binary Header)** 
   
   ขนาด Header พื้นฐานมีขนาดเพียง **4 ไบต์** เท่านั้น ซึ่งเล็กกว่า Text Header ของ HTTP หลายสิบเท่า
1. **ส่งข้อมูลแบบ Non-persistent Connection (ประหยัดพลังงาน)** 
   
   ทำงานบน UDP พอร์ตมาตรฐาน **5683** (หรือพอร์ต **5684** สำหรับ CoAPS ที่เข้ารหัสด้วย DTLS) ช่วยให้อุปกรณ์หลับในโหมด Deep-sleep ได้ทันทีหลังส่งข้อมูลเสร็จ
1. **รองรับทั้งการส่งแบบรับประกัน (Reliable) และไม่รับประกัน**
   
   สามารถเลือกได้ว่าจะส่งแบบรอการตอบรับ (CON) หรือส่งแบบยิงทิ้ง (NON)
1. **รองรับการส่งแบบ Multicast และ Broadcast** 
   
   สามารถส่งคำสั่งเพียงครั้งเดียวเพื่อควบคุมหลอดไฟทุกดวงในห้องพร้อมกันได้
1. **การสื่อสารแบบสองทิศทางอิสระ**
   
   ทั้ง Client และ Server สามารถเป็นผู้เริ่มต้นส่งคำขอ (Initiate Request) หาอีกฝ่ายได้อย่างอิสระ

---

## 10.4.2 ตารางเปรียบเทียบระหว่าง HTTP และ CoAP

| มิติการเปรียบเทียบ                        | HyperText Transfer Protocol (HTTP)                                 | Constrained Application Protocol (CoAP)                                  |
| :---------------------------------------- | :----------------------------------------------------------------- | :----------------------------------------------------------------------- |
| **ชั้น Transport Layer**                  | **TCP** (Connection-Oriented)                                      | **UDP** (Connectionless)                                                 |
| **ภาระส่วนหัว (Header Overhead)**         | **สูงมาก**: ข้อความเป็น Text String (100–500 ไบต์ขึ้นไป)           | **ต่ำมาก**: เข้ารหัสแบบไบนารีขนาดกะทัดรัด (**4 ไบต์**)                   |
| **การใช้พลังงาน (Power Consumption)**     | **สูง**: ต้องรักษาการเชื่อมต่อระยะยาว (Long/Persistent Connection) | **ต่ำมาก**: เชื่อมต่อแบบสั้น (Short Connection) ส่งเสร็จแล้วหลับได้ทันที |
| **การค้นหาทรัพยากร (Resource Discovery)** | **ไม่รองรับในตัว**: ต้องอาศัยโปรโตคอลภายนอกช่วย                    | **รองรับในตัว**: ผ่าน URI พิเศษ `/.well-known/core` (RFC 6690)           |
| **รูปแบบการแจ้งเตือน (Event Streaming)**  | ต้องทำ Polling ซ้ำๆ หรือใช้ WebSocket/SSE                          | **รองรับในตัวผ่าน CoAP Observe (RFC 7641)**                              |
| **ความปลอดภัย (Security)**                | TLS (HTTPS)                                                        | DTLS (CoAPS)                                                             |

---

## 10.4.3 โครงสร้างแพ็กเก็ต CoAP

หัวใจสำคัญที่ทำให้ CoAP ประหยัดแบนด์วิดท์อย่างยิ่งยวด คือ **Fixed Header ขนาดเพียง 4 ไบต์** (32 บิต) ตามโครงสร้างมาตรฐาน RFC 7252

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Ver| T |  TKL  |      Code     |          Message ID           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Token (if any, TKL bytes) ...                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Options (if any) ...                                        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|1 1 1 1 1 1 1 1|    Payload (if any) ...                       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

### คำอธิบายฟิลด์ในส่วนหัว
1. **Ver (Version - 2 บิต)** เวอร์ชันของโปรโตคอล CoAP (ปัจจุบันมีค่าเป็น `1`)
2. **T (Type - 2 บิต)**  กำหนดรูปแบบความน่าเชื่อถือของแพ็กเก็ต
   * `0` = **CON (Confirmable)**  ต้องการแพ็กเก็ตตอบรับ (ACK) ยืนยันว่าถึงปลายทาง หากไม่ได้รับจะส่งซ้ำแบบ Exponential Backoff
   * `1` = **NON (Non-confirmable)**  ส่งแบบไม่รอ ACK (คล้าย UDP ดั้งเดิม) เหมาะสำหรับส่งข้อมูลเซนเซอร์สม่ำเสมอ
   * `2` = **ACK (Acknowledgement)**  แพ็กเก็ตตอบรับยืนยันว่าได้รับข้อความ CON แล้ว
   * `3` = **RST (Reset)**  ปฏิเสธข้อความ หรือแจ้งว่าไม่สามารถประมวลผลข้อความนี้ได้
1. **TKL (Token Length - 4 บิต)**  ระบุความยาวของฟิลด์ Token (0–8 ไบต์) ซึ่งใช้สำหรับจับคู่ Request กับ Response เข้าด้วยกัน
2. **Code (8 บิต)**  รหัสคำสั่งหรือรหัสสถานะตอบกลับ แบ่งเป็น Class 3 บิต และ Detail 5 บิต
   * `0.01` = **GET**, `0.02` = **POST**, `0.03` = **PUT**, `0.04` = **DELETE**
   * `2.04` = **Changed** (เทียบเท่ากับ HTTP 204 No Content นิยมใช้ตอบกลับเมื่อคำสั่ง PUT สำเร็จ)
   * `2.05` = **Content** (เทียบเท่ากับ HTTP 200 OK)
   * `4.04` = **Not Found**, `5.00` = **Internal Server Error**
1. **Message ID (16 บิต)**  หมายเลขสุ่มกำกับแพ็กเก็ต ใช้ตรวจจับแพ็กเก็ตที่ส่งซ้ำซ้อน (Duplicate Detection) และจับคู่ข้อความ ACK กับ CON
2. **Payload Marker (`0xFF`)**  ไบต์คั่นที่มีค่าบิต `1111 1111` เพื่อบอกว่าข้อมูลหลังจากนี้คือเนื้อหา Payload จริง

---

## 4. การสร้าง CoAP Server ด้วยคอมโพเนนต์ ESP-IDF (`libcoap`)

การพัฒนา CoAP Server บน ESP-IDF จะใช้คอมโพเนนต์ทางการชื่อ **`libcoap`** ซึ่งจัดการโครงสร้างแพ็กเก็ตให้อัตโนมัติ ผู้พัฒนาเพียงแค่กำหนด URI และผูกเข้ากับฟังก์ชัน Callback

```c
#include <string.h>
#include "esp_log.h"
#include "coap3/coap.h"

static const char *TAG = "COAP_SERVER";
static char light_status_buf[100] = "{\"status\": true}";

// 1. ฟังก์ชัน Callback สำหรับคำขอ GET ผ่าน CoAP
static void esp_coap_get(coap_context_t *ctx, coap_resource_t *resource,
                        coap_session_t *session, coap_pdu_t *request,
                        coap_binary_t *token, coap_string_t *query,
                        coap_pdu_t *response)
{
    ESP_LOGI(TAG, "Handling CoAP GET request for resource 'light'");
    
    // ส่งข้อมูลสถานะหลอดไฟในรูปแบบ Text/JSON กลับไปใน Response PDU
    coap_add_data_blocked_response(resource, session, request, response,
                                  token, COAP_MEDIATYPE_TEXT_PLAIN, 0,
                                  strlen(light_status_buf),
                                  (const uint8_t *)light_status_buf);
}

// 2. ฟังก์ชัน Callback สำหรับคำขอ PUT เพื่อควบคุมสถานะหลอดไฟ
static void esp_coap_put(coap_context_t *ctx, coap_resource_t *resource,
                        coap_session_t *session, coap_pdu_t *request,
                        coap_binary_t *token, coap_string_t *query,
                        coap_pdu_t *response)
{
    size_t size;
    const uint8_t *data;

    // แจ้งเตือนไปยัง Observers ทุกตัวที่ติดตาม Resource นี้อยู่ (CoAP Observe)
    coap_resource_notify_observers(resource, NULL);

    // ดึงข้อมูล Payload ที่ส่งมาจาก Client
    (void)coap_get_data(request, &size, &data);

    if (size > 0) {
        ESP_LOGI(TAG, "Received CoAP PUT payload: %.*s", (int)size, data);

        // อัปเดตสถานะหลอดไฟ
        if (strncmp((char *)data, light_status_buf, size) != 0) {
            memcpy(light_status_buf, data, size);
            light_status_buf[size] = '\0';
            
            // ตอบกลับด้วยรหัส 2.04 Changed (สำเร็จ)
            response->code = COAP_RESPONSE_CODE(204);
            ESP_LOGI(TAG, "Smart Light status updated to: %s", light_status_buf);
        } else {
            response->code = COAP_RESPONSE_CODE(204);
        }
    } else {
        // หากไม่มีข้อมูล Payload ส่งกลับรหัส 5.00 Error
        response->code = COAP_RESPONSE_CODE(500);
    }
}

// 3. ฟังก์ชันหลักในการสร้างและรัน CoAP Server Task
void esp_create_coap_server(void)
{
    coap_context_t *ctx = NULL;
    coap_address_t serv_addr;
    coap_resource_t *resource = NULL;

    while (1) {
        coap_endpoint_t *ep = NULL;
        unsigned int wait_ms;

        // กำหนดที่อยู่และพอร์ต 5683 (COAP_DEFAULT_PORT)
        coap_address_init(&serv_addr);
        serv_addr.addr.sin.sin_family = AF_INET;
        serv_addr.addr.sin.sin_port = htons(COAP_DEFAULT_PORT);

        // สร้าง CoAP Context
        ctx = coap_new_context(NULL);
        if (!ctx) {
            ESP_LOGE(TAG, "coap_new_context() failed");
            continue;
        }

        // สร้าง UDP Endpoint สำหรับรับฟังแพ็กเก็ต
        ep = coap_new_endpoint(ctx, &serv_addr, COAP_PROTO_UDP);
        if (!ep) {
            ESP_LOGE(TAG, "coap_new_endpoint() failed");
            goto clean_up;
        }

        // สร้าง Resource สำหรับ URI "light"
        resource = coap_resource_init(coap_make_str_const("light"), 0);
        if (!resource) {
            ESP_LOGE(TAG, "coap_resource_init() failed");
            goto clean_up;
        }

        // ลงทะเบียนฟังก์ชัน Handler สำหรับ GET และ PUT
        coap_register_handler(resource, COAP_REQUEST_GET, esp_coap_get);
        coap_register_handler(resource, COAP_REQUEST_PUT, esp_coap_put);

        // เปิดใช้งานฟีเจอร์ CoAP Observe สำหรับ Resource นี้
        coap_resource_set_get_observable(resource, 1);

        // เพิ่ม Resource เข้าสู่ CoAP Context
        coap_add_resource(ctx, resource);

        ESP_LOGI(TAG, "CoAP server listening on port %d for resource 'light'...", COAP_DEFAULT_PORT);

        wait_ms = COAP_RESOURCE_CHECK_TIME * 1000;

        // วนลูปรับและประมวลผลแพ็กเก็ต CoAP
        while (1) {
            int result = coap_run_once(ctx, wait_ms);
            if (result < 0) {
                break;
            } else if (result && (unsigned int)result < wait_ms) {
                wait_ms -= result;
            } else {
                wait_ms = COAP_RESOURCE_CHECK_TIME * 1000;
            }
        }

clean_up:
        if (ctx) {
            coap_free_context(ctx);
            ctx = NULL;
        }
        coap_cleanup();
    }
}
```

---

## 10.4.5 การทดสอบด้วยเครื่องมือ CoAP Client

นอกจากการทดสอบผ่านปลั๊กอิน **Copper (Cu)** บนบราวเซอร์ Chrome ที่นิยมใช้กัน ในปัจจุบันเราสามารถใช้เครื่องมือบรรทัดคำสั่ง (**`coap-client`**) และสคริปต์ **Python (`aiocoap`)** ในการทดสอบได้อย่างสะดวก

<p align="center">
<!-- [รูปภาพ: การเชื่อมต่อและทดสอบคำสั่ง GET/PUT ด้วยโปรแกรม CoAP Client] -->
<!-- <img src="Images/coap_client_test_flow.png" width="650"> -->
</p>

### 10.4.5.1 การทดสอบ GET เพื่อสอบถามสถานะ

ส่งคำสั่ง CoAP GET ไปยังบอร์ด ESP32
```bash
coap-client -m get coap://192.168.3.80/light
```

*ผลลัพธ์ตอบกลับจาก ESP32*
```text
(2.05 Content)
{"status": true}
```

### 10.4.5.2 การทดสอบ PUT เพื่อสั่งเปลี่ยนสถานะหลอดไฟ

ส่งคำสั่ง CoAP PUT พร้อมแนบ Payload ใหม่
```bash
coap-client -m put -e "{\"status\": false}" coap://192.168.3.80/light
```

*ผลลัพธ์ตอบกลับจาก ESP32*
```text
(2.04 Changed)
```

เมื่อใช้คำสั่ง `GET` ซ้ำอีกครั้ง ค่าสถานะที่ได้รับจะเปลี่ยนเป็น `{"status": false}` อย่างถูกต้อง

---

## 10.4.6 สรุป

1. **CoAP** เป็นโปรโตคอลระดับแอปพลิเคชันที่นำข้อดีของ **RESTful API (แบบ HTTP)** มารวมเข้ากับ **ความเร็วและเบาของ UDP** เพื่อแก้ปัญหาคอขวดของอุปกรณ์ IoT
2. ด้วยขนาดส่วนหัวเพียง **4 ไบต์** ทำให้ CoAP ประหยัดแบนด์วิดท์และลดการใช้พลังงานของภาคส่งสัญญาณวิทยุลงได้อย่างมาก เหมาะอย่างยิ่งกับอุปกรณ์ที่ใช้แบตเตอรี่
3. รองรับการทำงานแบบ **Observe (RFC 7641)** ที่ทำให้อุปกรณ์สามารถแจ้งเตือนการเปลี่ยนแปลงของเซนเซอร์ให้ผู้ติดตามทราบได้ทันทีโดยไม่ต้องส่งคำขอถามซ้ำๆ
4. ใน[ใบงานที่ 10.4 การทดสอบเปรียบเทียบสมรรถนะของโปรโตคอลเครือข่าย](09-Labsheet-10-4-Protocol-Benchmark-and-Network-Forensics.md) เราจะได้นำสคริปต์ **Benchmark Suite** มาทดสอบวัดความหน่วงเวลา (RTT) และเปรียบเทียบขนาด Header จริงของทั้ง **HTTP, UDP, และ CoAP** เชิงประจักษ์
