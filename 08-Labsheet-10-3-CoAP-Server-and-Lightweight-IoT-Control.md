# ใบงานการทดลองที่ 10.3 (Lab 10.3)
### การพัฒนา CoAP Server สำหรับระบบฝังตัวและการควบคุมอุปกรณ์ด้วยโปรโตคอลน้ำหนักเบา

> [!NOTE] **คำชี้แจง**
> ในใบงานนี้ นักศึกษาจะได้เรียนรู้การติดตั้งและพัฒนาเซิร์ฟเวอร์ด้วยโปรโตคอล **CoAP (Constrained Application Protocol - RFC 7252)** บน ESP32 ซึ่งเป็นโปรโตคอลมาตรฐานสากลสำหรับอุปกรณ์ IoT ที่รวมข้อดีด้านความเบาของ UDP เข้ากับสถาปัตยกรรม RESTful ของ HTTP พร้อมทั้งทดสอบคุณสมบัติ **Observe (RFC 7641)** เพื่อติดตามค่าเซนเซอร์โดยอัตโนมัติ

---

## 1. วัตถุประสงค์การทดลอง (Objectives)
1. เข้าใจโครงสร้างของ CoAP Packet และการทำงานของ CoAP Endpoints (Resources)
2. สามารถพัฒนา CoAP Server บน ESP-IDF เพื่อเปิดให้บริการ Resource `/actuator/led` และ `/sensor/pot` บนพอร์ต UDP 5683 ได้
3. สามารถทดสอบส่งคำสั่ง CoAP แบบ **Confirmable (CON)** และ **Non-confirmable (NON)** ได้
4. สามารถทดสอบกลไก **Resource Discovery (`/.well-known/core`)** เพื่อค้นหารายการทรัพยากรบน ESP32 ได้
5. สามารถเขียนสคริปต์ Python ด้วยไลบรารี `aiocoap` เพื่อทดลองใช้งานฟีเจอร์ **CoAP Observe** ในการรับข้อมูลเซนเซอร์แบบ Real-time Event-driven

---

## 2. โครงสร้างการแมป CoAP Endpoints

| Resource URI | Method | คำอธิบาย | ชนิด Payload | ตัวอย่างคำสั่ง CoAP Client |
| :--- | :---: | :--- | :---: | :--- |
| `/.well-known/core` | GET | แสดงรายการ Resource ทั้งหมดของโหนด | CoRE Link Format | `coap-client -m get coap://esp32-node.local/.well-known/core` |
| `/sensor/pot` | GET | อ่านค่าอนาล็อก Potentiometer | Text / JSON | `coap-client -m get coap://esp32-node.local/sensor/pot` |
| `/actuator/led` | PUT | สั่งเปิด/ปิดไฟ LED (1 หรือ 0) | Text (`1`/`0`) | `coap-client -m put -e "1" coap://esp32-node.local/actuator/led` |

---

## 3. ขั้นตอนการทดลอง (Deconstructed Activities)

### กิจกรรมที่ 3.1: สร้างโปรเจกต์และเพิ่ม CoAP Component
```powershell
idf.py create-project Lab10-3_CoAP_Server
cd Lab10-3_CoAP_Server
idf.py set-target esp32
```

ใน ESP-IDF มีคอมโพเนนต์ `libcoap` (หรือ `coap`) ให้เลือกใช้งานผ่าน `idf.py menuconfig` หรือระบุใน `main/idf_component.yml`:
```yaml
dependencies:
  espressif/coap: "^4.3.4"
```

---

### กิจกรรมที่ 3.2: การสร้างและลงทะเบียน CoAP Resource
ในไฟล์ `main/main.c`:

```c
#include "coap3/coap.h"

// 1. Handler สำหรับ GET /sensor/pot
static void hnd_get_pot(coap_resource_t *resource,
                        coap_session_t *session,
                        const coap_pdu_t *request,
                        const coap_string_t *query,
                        coap_pdu_t *response)
{
    char pot_str[32];
    int pot_val = 2048; // แทนที่ด้วยค่าอ่านจริงจาก GPIO 34
    snprintf(pot_str, sizeof(pot_str), "%d", pot_val);

    coap_pdu_set_code(response, COAP_RESPONSE_CODE_CONTENT);
    coap_add_data(response, strlen(pot_str), (const uint8_t *)pot_str);
}

// 2. Handler สำหรับ PUT /actuator/led
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
            gpio_set_level(GPIO_NUM_2, 1);
            ESP_LOGI("CoAP", "LED turned ON via CoAP PUT");
        } else if (data[0] == '0') {
            gpio_set_level(GPIO_NUM_2, 0);
            ESP_LOGI("CoAP", "LED turned OFF via CoAP PUT");
        }
    }
    coap_pdu_set_code(response, COAP_RESPONSE_CODE_CHANGED); // เทียบเท่า 204 Changed
}
```

---

### กิจกรรมที่ 3.3: การทดสอบด้วย Python aiocoap Script
ติดตั้งไลบรารีบนคอมพิวเตอร์:
```powershell
pip install aiocoap
```

สร้างไฟล์ `test_coap.py`:
```python
import asyncio
from aiocoap import *

async def main():
    protocol = await Context.create_client_context()
    
    # 1. ทดสอบ GET /.well-known/core
    request = Message(code=Code.GET, uri='coap://esp32-node.local/.well-known/core')
    response = await protocol.request(request).response
    print("Resource Directory:\n", response.payload.decode())

    # 2. ทดสอบ PUT /actuator/led (สั่งเปิดไฟ)
    request_led = Message(code=Code.PUT, payload=b"1", uri='coap://esp32-node.local/actuator/led')
    response_led = await protocol.request(request_led).response
    print("LED Control Result:", response_led.code)

asyncio.run(main())
```

---

## 4. บันทึกผลการทดลองและคำถามท้ายบท (Lab Report & Questions)
1. นำผลการ Query `/.well-known/core` มาแสดงในรายงาน พร้อมอธิบายรูปแบบ **CoRE Link Format (RFC 6690)**
2. อธิบายความแตกต่างของแพ็กเก็ต CoAP ระหว่าง **CON (Confirmable)** และ **NON (Non-confirmable)** เมื่อทดสอบในเครือข่ายที่มีการรบกวนสัญญาณ
3. ทำไม CoAP จึงเหมาะสมกับโปรโตคอลการสื่อสารบนเครือข่ายเช่น Thread, Zigbee IP หรือ NB-IoT มากกว่า HTTP?
