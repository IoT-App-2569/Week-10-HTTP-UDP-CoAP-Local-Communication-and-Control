import socket
import time

ESP32_IP = "192.168.1.33"

print("LAB 10.2 - UDP TELEMETRY & CONTROL (ACTUAL ESP32)")
print("67030142 Piyawit Pholsri | TechCom")
print("=" * 66)

listener = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
listener.bind(("192.168.1.147", 3334))
listener.settimeout(3)

print("\n[1] UDP telemetry broadcast :3334")
last_seq = None
received = 0
lost = 0
while received < 10:
    data, addr = listener.recvfrom(128)
    message = data.decode().strip()
    fields = dict(item.split(":") for item in message.split(","))
    seq = int(fields["SEQ"])
    pot = int(fields["POT"])
    if last_seq is not None and seq > last_seq + 1:
        lost += seq - last_seq - 1
    last_seq = seq
    received += 1
    print(f"  RX {received:02d} from {addr[0]} | SEQ={seq:<6} POT={pot:<4}")

total = received + lost
print(f"  Result: received={received}, lost={lost}, packet loss={lost / total * 100:.2f}%")

print("\n[2] UDP LED command/ACK + RTT :3333")
client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
client.settimeout(2)
rtts = []
for index, command in enumerate((b"LED_ON", b"LED_OFF") * 3, 1):
    started = time.perf_counter()
    client.sendto(command, (ESP32_IP, 3333))
    reply, _ = client.recvfrom(128)
    rtt = (time.perf_counter() - started) * 1000
    rtts.append(rtt)
    print(f"  {index:02d} {command.decode():7s} -> {reply.decode():11s} RTT={rtt:6.2f} ms")
    time.sleep(0.2)

print(f"  RTT min/avg/max = {min(rtts):.2f}/{sum(rtts) / len(rtts):.2f}/{max(rtts):.2f} ms")
print("\nPASS: Actual UDP telemetry and LED control verified.")
input("\nPress Enter to close...")
