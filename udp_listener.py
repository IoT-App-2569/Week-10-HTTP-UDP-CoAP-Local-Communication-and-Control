import socket

UDP_PORT = 3334

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind(("", UDP_PORT))

print(f"Listening for UDP Broadcast on port {UDP_PORT}...")

last_seq = None
packet_count = 0
lost_packets = 0

try:
    while True:
        data, addr = sock.recvfrom(128)
        msg = data.decode("utf-8").strip()
        packet_count += 1
        
        # ถอดรหัส SEQ และ POT
        parts = dict(item.split(":") for item in msg.split(","))
        current_seq = int(parts.get("SEQ", 0))
        pot_val = int(parts.get("POT", 0))

        if last_seq is not None:
            diff = current_seq - last_seq
            if diff > 1:
                lost = diff - 1
                lost_packets += lost
                print(f"[PACKET LOSS DETECTED] Lost {lost} packets! (Expected {last_seq + 1}, got {current_seq})")

        last_seq = current_seq
        print(f"[{addr[0]}] Seq: {current_seq:<6} | Potentiometer: {pot_val:<5} | Total Lost: {lost_packets}")

except KeyboardInterrupt:
    if packet_count > 0:
        loss_rate = (lost_packets / (packet_count + lost_packets)) * 100
        print(f"\n--- Statistics ---")
        print(f"Received: {packet_count} packets")
        print(f"Lost: {lost_packets} packets")
        print(f"Packet Loss Rate: {loss_rate:.2f}%")
