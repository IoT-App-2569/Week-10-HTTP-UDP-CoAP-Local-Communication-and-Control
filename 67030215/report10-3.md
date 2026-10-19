# ใบงานการทดลองที่ 10.3
## การพัฒนา CoAP Server สำหรับระบบฝังตัวและการควบคุมอุปกรณ์ด้วยโปรโตคอลน้ำหนักเบา (RFC 7252)

---

## 1. ผลการทดสอบการทำงานของ CoAP Client 

รันสคริปต์ `test_coap.py` ด้วยไลบรารี `aiocoap` เพื่อทดสอบ 

```bash
python3 -u "test_coap.py"
```
## ผลลัพธ์
```
PS C:\Users\hp\Lab10-3_CoAP_Server\Lab10-3_CoAP_Server> python test_coap.py 10.21.109.134

--- 1. Testing CoAP Resource Discovery (/.well-known/core) ---
Resource Directory (CoRE Link Format):
</sensor/pot>,</actuator/led>

--- 2. Testing CoAP GET /sensor/pot ---
Potentiometer Value: 2559 (Code: 2.05 Content)

--- 3. Testing CoAP PUT /actuator/led (Turn ON) ---
LED ON Response Code: 2.04 Changed

--- 4. Testing CoAP PUT /actuator/led (Turn OFF) ---
LED OFF Response Code: 2.04 Changed
```
