import socket


UDP_PORT = 3334


def main() -> None:
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind(("", UDP_PORT))

    print(f"Listening for UDP Broadcast on port {UDP_PORT}...")

    last_sequence = None
    packet_count = 0
    lost_packets = 0

    try:
        while True:
            data, address = sock.recvfrom(128)
            message = data.decode("utf-8").strip()
            packet_count += 1

            parts = dict(item.split(":", 1) for item in message.split(","))
            current_sequence = int(parts["SEQ"])
            potentiometer_value = int(parts["POT"])

            if last_sequence is not None:
                difference = current_sequence - last_sequence
                if difference > 1:
                    lost = difference - 1
                    lost_packets += lost
                    print(
                        f"[PACKET LOSS DETECTED] Lost {lost} packets! "
                        f"(Expected {last_sequence + 1}, got {current_sequence})"
                    )

            last_sequence = current_sequence
            print(
                f"[{address[0]}] Seq: {current_sequence:<6} | "
                f"Potentiometer: {potentiometer_value:<5} | "
                f"Total Lost: {lost_packets}"
            )
    except KeyboardInterrupt:
        if packet_count > 0:
            loss_rate = lost_packets / (packet_count + lost_packets) * 100
            print("\n--- Statistics ---")
            print(f"Received: {packet_count} packets")
            print(f"Lost: {lost_packets} packets")
            print(f"Packet Loss Rate: {loss_rate:.2f}%")
    finally:
        sock.close()


if __name__ == "__main__":
    main()
