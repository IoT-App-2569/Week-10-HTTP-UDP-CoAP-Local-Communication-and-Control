import socket
import sys
import time

# ระบุ IP ของบอร์ด ESP32 (สามารถส่งเป็น argument ได้ เช่น python udp_controller.py 192.168.1.50)
ESP32_IP = sys.argv[1] if len(sys.argv) > 1 else "192.168.1.181"
CMD_PORT = 3333

client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
client.settimeout(2.0)

print(f"กำลังส่งคำสั่งควบคุมไปยัง ESP32 ที่ {ESP32_IP}:{CMD_PORT}...")

commands = [b"LED_ON", b"LED_OFF"] * 5  # ส่งสลับเปิด-ปิด 10 ครั้ง
rtt_list = []

for i, cmd in enumerate(commands, 1):
    try:
        t_start = time.perf_counter()
        client.sendto(cmd, (ESP32_IP, CMD_PORT))
        resp, _ = client.recvfrom(128)
        t_end = time.perf_counter()
        
        rtt_ms = (t_end - t_start) * 1000
        rtt_list.append(rtt_ms)
        print(f"รอบที่ {i:02d}: ส่ง '{cmd.decode()}' -> ได้รับตอบกลับ '{resp.decode()}' | เวลา RTT: {rtt_ms:.2f} ms")
    except socket.timeout:
        print(f"รอบที่ {i:02d}: รอนานเกินเวลา (Timeout) ไม่ได้รับการตอบกลับ")
    time.sleep(0.5)

if rtt_list:
    avg_rtt = sum(rtt_list) / len(rtt_list)
    print(f"\nผลสรุป RTT เฉลี่ยของ UDP: {avg_rtt:.2f} ms (ต่ำสุด: {min(rtt_list):.2f} ms, สูงสุด: {max(rtt_list):.2f} ms)")
