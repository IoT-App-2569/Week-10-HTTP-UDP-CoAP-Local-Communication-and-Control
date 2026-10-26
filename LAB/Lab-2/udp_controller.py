import socket
import time


ESP32_IP = "10.14.195.186"
COMMAND_PORT = 3333


def main() -> None:
    client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    client.settimeout(2.0)

    print(f"Sending control commands to {ESP32_IP}:{COMMAND_PORT}...")
    commands = [b"LED_ON", b"LED_OFF"] * 5
    rtt_values = []

    try:
        for round_number, command in enumerate(commands, 1):
            try:
                start_time = time.perf_counter()
                client.sendto(command, (ESP32_IP, COMMAND_PORT))
                response, _ = client.recvfrom(128)
                end_time = time.perf_counter()

                rtt_ms = (end_time - start_time) * 1000
                rtt_values.append(rtt_ms)
                print(
                    f"Round {round_number:02d}: Sent "
                    f"'{command.decode()}' -> Reply "
                    f"'{response.decode()}' | RTT: {rtt_ms:.2f} ms"
                )
            except socket.timeout:
                print(f"Round {round_number:02d}: Request timed out!")

            time.sleep(0.5)
    finally:
        client.close()

    if rtt_values:
        average = sum(rtt_values) / len(rtt_values)
        print(
            f"\nAverage UDP RTT Latency: {average:.2f} ms "
            f"(Min: {min(rtt_values):.2f} ms, "
            f"Max: {max(rtt_values):.2f} ms)"
        )


if __name__ == "__main__":
    main()
