# รายงานผลการทดลอง ใบงานที่ 10.3
### การพัฒนา CoAP Server สำหรับระบบฝังตัวและการควบคุมอุปกรณ์ด้วยโปรโตคอลน้ำหนักเบา
**รหัสนักศึกษา:** 67030338  

---

## 1. วัตถุประสงค์การทดลอง
1. เข้าใจโครงสร้างของ CoAP Packet และการทำงานของ CoAP Endpoints (Resources)
2. สามารถพัฒนา CoAP Server บน ESP-IDF เพื่อเปิดให้บริการ Resource `/actuator/led` และ `/sensor/pot` บนพอร์ต UDP 5683 ได้
3. สามารถทดสอบส่งคำสั่ง CoAP แบบ Confirmable (CON) และ Non-confirmable (NON) ได้
4. สามารถทดสอบกลไก Resource Discovery (`/.well-known/core`) เพื่อค้นหารายการทรัพยากรบน ESP32 ได้
5. สามารถเขียนสคริปต์ Python ด้วยไลบรารี `aiocoap` เพื่อสื่อสารกับ CoAP Server ได้

---

## 2. การดัดแปลงระบบ Virtual Potentiometer (กรณีไม่มี Potentiometer จริง)
เนื่องจากไม่มีอุปกรณ์ Potentiometer ทางกายภาพ ตัวเฟิร์มแวร์จึงได้จำลองค่าเซนเซอร์ขึ้น:
- **โหมด Auto-Sweep:** ทำการปรับเปลี่ยนค่า `s_virtual_pot_val` ขึ้น-ลงอัตโนมัติ (0 - 4095) ทุกครั้งที่มีการอ่านค่าผ่าน `GET /sensor/pot` เพื่อจำลองการหมุนของลูกบิด
- **การปรับค่าผ่าน CoAP PUT:** รองรับการส่งคำสั่ง `PUT` พร้อมตัวเลขไปยัง `/sensor/pot` (เช่น ค่า `3500`) เพื่อกำหนดค่าคงที่ได้โดยตรง

---

## 3. บันทึกผลการทดลอง (บันทึกข้อมูลด้วยตนเอง)

### 3.1 ผลการทดสอบ CoAP Resource Discovery (`GET /.well-known/core`)
*(กรุณาแปะข้อความผลลัพธ์หรือภาพหน้าจอ Terminal ขณะ Query รายการ Resources)*

```text
C:\Users\09677\AppData\Local\Programs\Python\Python313\Lib\site-packages\aiocoap\cli\client.py:404: DeprecationWarning: Initializing messages with an mtype is deprecated. Instead, set transport_tuning=aiocoap.Reliable or aiocoap.Unreliable.
  request = aiocoap.Message(
</sensor/pot>,</actuator/led>
```
```

```
---

### 3.2 ผลการทดสอบอ่านค่าเซนเซอร์ (`GET /sensor/pot`)
*(กรุณาแปะข้อความผลลัพธ์หรือภาพหน้าจอ Terminal ขณะอ่านค่าเซนเซอร์)*

```text
C:\Users\09677\AppData\Local\Programs\Python\Python313\Lib\site-packages\aiocoap\cli\client.py:404: DeprecationWarning: Initializing messages with an mtype is deprecated. Instead, set transport_tuning=aiocoap.Reliable or aiocoap.Unreliable.
  request = aiocoap.Message(
2528


```
```
I (621395) COAP_LAB: GET /sensor/pot -> 2528
```
---

### 3.3 ผลการทดสอบควบคุม LED (`PUT /actuator/led`)
*(กรุณาแปะข้อความผลลัพธ์หรือภาพหน้าจอ Terminal ขณะสั่งเปิด ('1') และปิด ('0') หลอดไฟ LED)*

```text
PS C:\work-2026-1\Week-10-HTTP-UDP-CoAP-Local-Communication-and-Control> python -m aiocoap.cli.client -m PUT --payload 1 coap://192.168.1.41/actuator/led
C:\Users\09677\AppData\Local\Programs\Python\Python313\Lib\site-packages\aiocoap\cli\client.py:404: DeprecationWarning: Initializing messages with an mtype is deprecated. Instead, set transport_tuning=aiocoap.Reliable or aiocoap.Unreliable.
  request = aiocoap.Message(


```
```
I (709035) COAP_LAB: LED turned ON via CoAP PUT
```

---

## 4. ตอบคำถามท้ายการทดลอง

### คำถามข้อที่ 1
> **นำผลการ Query `/.well-known/core` มาแสดงในรายงาน พร้อมอธิบายรูปแบบ CoRE Link Format (RFC 6690) ว่าแสดงข้อมูลทรัพยากรอย่างไร**

**ตอบ:** CoRE Link Format (RFC 6690) ใช้แสดงรายการทรัพยากรในรูปแบบข้อความกะทัดรัด เช่น `</sensor/pot>,</actuator/led>` โดยแต่ละ URI จะถูกครอบด้วยเครื่องหมาย `< >` และคั่นด้วยจุลภาค (`,`) และสามารถระบุแอตทริบิวต์เพิ่มเติม เช่น ชนิดข้อมูล (`ct`) หรือการติดตาม (`obs`) ช่วยให้อุปกรณ์ Client สามารถค้นหา (Discover) บริการทั้งหมดบนโหนด IoT ได้โดยอัตโนมัติ

---

### คำถามข้อที่ 2
> **อธิบายความแตกต่างของแพ็กเก็ต CoAP ระหว่าง CON (Confirmable) และ NON (Non-confirmable) เมื่อทดสอบในเครือข่ายที่มีการรบกวนสัญญาณ**

**ตอบ:** แพ็กเก็ต **CON (Confirmable)** บังคับให้ผู้รับต้องตอบกลับด้วย ACK หากแพ็กเก็ตสูญหายจากสัญญาณรบกวน ฝั่งส่งจะทำ Retransmission อัตโนมัติด้วย Exponential Backoff จนกว่าจะสำเร็จ จึงมีความน่าเชื่อถือสูง ส่วน **NON (Non-confirmable)** เป็นการส่งแบบ Fire-and-Forget ไม่มีการตอบ ACK และไม่ส่งซ้ำ จึงมีความหน่วงต่ำและประหยัดพลังงาน แต่ข้อมูลอาจสูญหายได้

---

### คำถามข้อที่ 3
> **ทำไม CoAP จึงเหมาะสมกับโปรโตคอลการสื่อสารบนเครือข่ายเช่น Thread, Zigbee IP หรือ NB-IoT มากกว่า HTTP?**

**ตอบ:** CoAP มีขนาด Header แบบไบนารีคงที่เพียง 4 ไบต์ ทำงานบน UDP ทำให้ตัดภาระ TCP Handshake ออกทั้งหมด อีกทั้งรองรับทั้งการสั่งงานแบบเชื่อถือได้ (CON) และไม่เชื่อถือได้ (NON) จึงกินแบนด์วิดท์และพลังงานต่ำมาก เหมาะกับเครือข่ายที่มีข้อจำกัดด้านพลังงานและแบนด์วิดท์ เช่น Thread, Zigbee IP หรือ NB-IoT ต่างจาก HTTP ที่ใช้ TCP และมี Header แบบ Text ขนาดใหญ่

---

## 5. สรุปผลการทดลอง
**สรุปผลการทดลอง:**  
การทดลองนี้พิสูจน์ให้เห็นว่า CoAP รวมจุดเด่นของสถาปัตยกรรม RESTful เข้ากับความเร็วและประสิทธิภาพของ UDP ทำให้สามารถอ่านค่าเซนเซอร์ (`GET /sensor/pot`) และควบคุมอุปกรณ์ (`PUT /actuator/led`) ได้อย่างรวดเร็วและใช้ข้อมูลเพียงไม่กี่ไบต์ เหมาะอย่างยิ่งกับอุปกรณ์สมองกลฝังตัวที่มีทรัพยากรจำกัดในระบบ IoT ยุคใหม่
