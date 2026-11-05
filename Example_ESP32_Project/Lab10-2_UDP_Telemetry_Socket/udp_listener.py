import argparse
import socket
import time

parser = argparse.ArgumentParser(description="UDP telemetry listener (default stays open until Ctrl+C)")
parser.add_argument("--host", default="", help="Bind address; default listens on all interfaces")
parser.add_argument("--port", type=int, default=3334)
parser.add_argument("--duration", type=float, default=0, help="Stop after this many seconds; 0 means run until Ctrl+C")
args = parser.parse_args()

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind((args.host, args.port))
sock.settimeout(0.5)
print(f"Listening for UDP telemetry on {args.host or '0.0.0.0'}:{args.port}...")
if args.duration > 0:
    print(f"Auto-stop after {args.duration:g} seconds")

last_seq = None
packet_count = 0
lost_packets = 0
start_time = time.monotonic()
try:
    while args.duration <= 0 or time.monotonic() - start_time < args.duration:
        try:
            data, addr = sock.recvfrom(128)
        except socket.timeout:
            continue
        msg = data.decode("utf-8").strip()
        packet_count += 1
        try:
            parts = dict(item.split(":", 1) for item in msg.split(","))
            current_seq = int(parts.get("SEQ", 0))
            pot_val = int(parts.get("POT", 0))
        except (ValueError, TypeError):
            print(f"[{addr[0]}] Ignored malformed packet: {msg}")
            continue

        if last_seq is not None:
            diff = current_seq - last_seq
            if diff > 1:
                lost = diff - 1
                lost_packets += lost
                print(f"[PACKET LOSS DETECTED] Lost {lost} packets! (Expected {last_seq + 1}, got {current_seq})")
        last_seq = current_seq
        print(f"[{addr[0]}] Seq: {current_seq:<6} | Potentiometer: {pot_val:<5} | Total Lost: {lost_packets}")
except KeyboardInterrupt:
    pass
finally:
    if packet_count > 0:
        loss_rate = (lost_packets / (packet_count + lost_packets)) * 100
    else:
        loss_rate = 0.0
    print("\n--- Statistics ---")
    print(f"Received: {packet_count} packets")
    print(f"Lost: {lost_packets} packets")
    print(f"Packet Loss Rate: {loss_rate:.2f}%")
    sock.close()