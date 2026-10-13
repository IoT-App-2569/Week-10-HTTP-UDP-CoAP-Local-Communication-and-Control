# คำตอบและบันทึกผลการทดลอง — สัปดาห์ที่ 10

Local Control Protocols (HTTP, UDP, CoAP) & Edge Device Discovery

| รหัสนักศึกษา | 67030098 |
| :--- | :--- |
| Branch | `67030098-HW` |

---

## รายการไฟล์แบบบันทึกผล

| ไฟล์ | ใบงานต้นฉบับ | โค้ดที่เกี่ยวข้อง | เนื้อหา |
| :--- | :--- | :--- | :--- |
| [Answer-10-1-mDNS-and-HTTP-REST-Server.md](Answer-10-1-mDNS-and-HTTP-REST-Server.md) | [06](../06-Labsheet-10-1-mDNS-Discovery-and-HTTP-REST-Server.md) | `Code/Lab10-1_HTTP_REST_Server/` | ผล ping mDNS, `GET /api/status`, `POST /api/led`, วิเคราะห์ขนาด HTTP Header + 3 คำถาม |
| [Answer-10-2-UDP-Broadcast-and-Realtime-Telemetry.md](Answer-10-2-UDP-Broadcast-and-Realtime-Telemetry.md) | [07](../07-Labsheet-10-2-UDP-Broadcast-and-Realtime-Telemetry.md) | `Code/Lab10-2_UDP_Telemetry_Socket/` | สถิติ Packet Loss 1 นาที, ตาราง RTT 10 รอบ, เทียบ UDP กับ HTTP + 3 คำถาม |
| [Answer-10-3-CoAP-Server-and-Lightweight-Control.md](Answer-10-3-CoAP-Server-and-Lightweight-Control.md) | [08](../08-Labsheet-10-3-CoAP-Server-and-Lightweight-IoT-Control.md) | `Code/Lab10-3_CoAP_Server/` | `/.well-known/core`, `/sensor/pot`, `/actuator/led`, CON vs NON, Observe + 3 คำถาม |
| [Answer-10-4-Protocol-Benchmark-and-Network-Forensics.md](Answer-10-4-Protocol-Benchmark-and-Network-Forensics.md) | [09](../09-Labsheet-10-4-Protocol-Benchmark-and-Network-Forensics.md) | `Code/Lab10-4_Protocol_Benchmark/` | Payload Efficiency, ตาราง RTT 50 รอบ, RAM Footprint, Pros & Cons, 3 กรณีศึกษา |

---

## ลำดับการทำงานที่แนะนำ

1. **Lab 10.1** — แฟลชเฟิร์มแวร์ HTTP REST Server แล้วบันทึกผล `curl -i` ไว้ (จะต้องใช้อ้างอิงใน Lab 10.2 ข้อ 1 และ Lab 10.4)
2. **Lab 10.2** — แฟลชเฟิร์มแวร์ UDP แล้ววัด RTT และ Packet Loss เทียบกับค่าจาก Lab 10.1
3. **Lab 10.3** — แฟลชเฟิร์มแวร์ CoAP แล้วทดสอบ Discovery / CON / NON / Observe
4. **Lab 10.4** — รัน `benchmark_protocols.py` ทีละโปรโตคอล (ต้องสลับเฟิร์มแวร์ตามที่ทดสอบ) แล้วสรุปภาพรวม

> [!NOTE]
> ค่า `free_heap` สำหรับตาราง RAM Footprint ใน Lab 10.4 ให้อ่านจาก `idf.py monitor` หรือจาก JSON response ของ `GET /api/status` ขณะที่เฟิร์มแวร์แต่ละตัวทำงานอยู่
