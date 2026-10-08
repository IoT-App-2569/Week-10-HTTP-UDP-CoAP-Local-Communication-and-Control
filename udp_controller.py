import socket
import time

ESP32_IP = "127.0.0.1"  # หรือ IP ของโฮสต์
CMD_PORT = 3333

client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
client.settimeout(2.0)

print(f"Sending control commands to {ESP32_IP}:{CMD_PORT}...")
commands = [b"LED_ON", b"LED_OFF"] * 5
rtt_list = []

for i, cmd in enumerate(commands, 1):
    try:
        t_start = time.perf_counter()
        client.sendto(cmd, (ESP32_IP, CMD_PORT))
        resp, _ = client.recvfrom(128)
        t_end = time.perf_counter()
        
        rtt_ms = (t_end - t_start) * 1000
        rtt_list.append(rtt_ms)
        print(f"Round {i:02d}: Sent '{cmd.decode()}' -> Reply '{resp.decode()}' | RTT: {rtt_ms:.2f} ms")
    except socket.timeout:
        print(f"Round {i:02d}: Request timed out!")
    time.sleep(0.5)

if rtt_list:
    avg_rtt = sum(rtt_list) / len(rtt_list)
    print(f"\nAverage UDP RTT Latency: {avg_rtt:.2f} ms (Min: {min(rtt_list):.2f} ms, Max: {max(rtt_list):.2f} ms)")
