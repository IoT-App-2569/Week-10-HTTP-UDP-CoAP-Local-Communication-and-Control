# ใบงานการทดลองที่ 10.2
## การสื่อสารข้อมูลแบบเรียลไทม์ด้วย UDP Socket และการควบคุมอุปกรณ์บน ESP32

---

## 1. การทดสอบรับส่งข้อมูล Telemetry แบบ UDP Broadcast (`udp_listener.py`)

รันสคริปต์ `udp_listener.py` เพื่อดักฟังแพ็กเก็ตบรอดแคสต์พอร์ต 3334 และตรวจสอบการสูญหายของแพ็กเก็ต (Packet Loss) จาก Sequence Number

```bash
python3 -u "udp_listener.py"
```

## ผลลัพธ์
```
PS C:\Users\hp\Lab10-2_UDP_Telemetry_Socket\Lab10-2_UDP_Telemetry_Socket> python python\udp_listener.py
Listening for UDP Broadcast on port 3334... (Ctrl+C เพื่อหยุดและดูสถิติ)
[10.21.109.134] Seq: 6954   | Potentiometer: 2559  | Total Lost: 0
[10.21.109.134] Seq: 6955   | Potentiometer: 2581  | Total Lost: 0
[10.21.109.134] Seq: 6956   | Potentiometer: 2582  | Total Lost: 0
[10.21.109.134] Seq: 6957   | Potentiometer: 2559  | Total Lost: 0
[10.21.109.134] Seq: 6958   | Potentiometer: 2582  | Total Lost: 0
[10.21.109.134] Seq: 6959   | Potentiometer: 2559  | Total Lost: 0
[10.21.109.134] Seq: 6960   | Potentiometer: 2559  | Total Lost: 0
[10.21.109.134] Seq: 6961   | Potentiometer: 2585  | Total Lost: 0
[10.21.109.134] Seq: 6962   | Potentiometer: 2559  | Total Lost: 0
[10.21.109.134] Seq: 6963   | Potentiometer: 2585  | Total Lost: 0
[10.21.109.134] Seq: 6964   | Potentiometer: 2559  | Total Lost: 0
[10.21.109.134] Seq: 6965   | Potentiometer: 2559  | Total Lost: 0
[10.21.109.134] Seq: 6966   | Potentiometer: 2582  | Total Lost: 0
[10.21.109.134] Seq: 6967   | Potentiometer: 2559  | Total Lost: 0
[10.21.109.134] Seq: 6968   | Potentiometer: 2559  | Total Lost: 0
[10.21.109.134] Seq: 6969   | Potentiometer: 2559  | Total Lost: 0
[10.21.109.134] Seq: 6970   | Potentiometer: 2586  | Total Lost: 0
[10.21.109.134] Seq: 6971   | Potentiometer: 2559  | Total Lost: 0
[10.21.109.134] Seq: 6972   | Potentiometer: 2584  | Total Lost: 0
[10.21.109.134] Seq: 6973   | Potentiometer: 2559  | Total Lost: 0
[10.21.109.134] Seq: 6974   | Potentiometer: 2559  | Total Lost: 0
[10.21.109.134] Seq: 6975   | Potentiometer: 2559  | Total Lost: 0
[10.21.109.134] Seq: 6976   | Potentiometer: 2559  | Total Lost: 0
[10.21.109.134] Seq: 6977   | Potentiometer: 2559  | Total Lost: 0
[PACKET LOSS DETECTED] Lost 1 packets! (Expected 6978, got 6979)
[10.21.109.134] Seq: 6979   | Potentiometer: 2559  | Total Lost: 1
[10.21.109.134] Seq: 6980   | Potentiometer: 2559  | Total Lost: 1
[10.21.109.134] Seq: 6981   | Potentiometer: 2583  | Total Lost: 1
[10.21.109.134] Seq: 6982   | Potentiometer: 2582  | Total Lost: 1
[10.21.109.134] Seq: 6983   | Potentiometer: 2581  | Total Lost: 1
[10.21.109.134] Seq: 6984   | Potentiometer: 2582  | Total Lost: 1
[10.21.109.134] Seq: 6985   | Potentiometer: 2559  | Total Lost: 1
[10.21.109.134] Seq: 6986   | Potentiometer: 2559  | Total Lost: 1
[10.21.109.134] Seq: 6987   | Potentiometer: 2582  | Total Lost: 1
[10.21.109.134] Seq: 6988   | Potentiometer: 2559  | Total Lost: 1
[10.21.109.134] Seq: 6989   | Potentiometer: 2582  | Total Lost: 1
[PACKET LOSS DETECTED] Lost 1 packets! (Expected 6990, got 6991)
[10.21.109.134] Seq: 6991   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 6992   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 6993   | Potentiometer: 2582  | Total Lost: 2
[10.21.109.134] Seq: 6994   | Potentiometer: 2581  | Total Lost: 2
[10.21.109.134] Seq: 6995   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 6996   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 6997   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 6998   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 6999   | Potentiometer: 2582  | Total Lost: 2
[10.21.109.134] Seq: 7000   | Potentiometer: 2587  | Total Lost: 2
[10.21.109.134] Seq: 7001   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 7002   | Potentiometer: 2581  | Total Lost: 2
[10.21.109.134] Seq: 7003   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 7004   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 7005   | Potentiometer: 2581  | Total Lost: 2
[10.21.109.134] Seq: 7006   | Potentiometer: 2580  | Total Lost: 2
[10.21.109.134] Seq: 7007   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 7008   | Potentiometer: 2587  | Total Lost: 2
[10.21.109.134] Seq: 7009   | Potentiometer: 2582  | Total Lost: 2
[10.21.109.134] Seq: 7010   | Potentiometer: 2582  | Total Lost: 2
[10.21.109.134] Seq: 7011   | Potentiometer: 2579  | Total Lost: 2
[10.21.109.134] Seq: 7012   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 7013   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 7014   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 7015   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 7016   | Potentiometer: 2585  | Total Lost: 2
[10.21.109.134] Seq: 7017   | Potentiometer: 2581  | Total Lost: 2
[10.21.109.134] Seq: 7018   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 7019   | Potentiometer: 2581  | Total Lost: 2
[10.21.109.134] Seq: 7020   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 7021   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 7022   | Potentiometer: 2559  | Total Lost: 2
[10.21.109.134] Seq: 7023   | Potentiometer: 2579  | Total Lost: 2
[10.21.109.134] Seq: 7024   | Potentiometer: 2581  | Total Lost: 2
[10.21.109.134] Seq: 7025   | Potentiometer: 2579  | Total Lost: 2
[10.21.109.134] Seq: 7026   | Potentiometer: 2559  | Total Lost: 2

--- Statistics ---
Received: 71 packets
Lost: 2 packets
Packet Loss Rate: 2.74%
```
