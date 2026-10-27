#!/usr/bin/env python3
"""
Lab 10.2: UDP Unicast Two-Way Controller & RTT Benchmark
Student: นายอัฐวุฒิ บัวเลียน (67030359)
"""
import argparse
import math
import socket
import time

parser = argparse.ArgumentParser(description="UDP LED Controller & RTT Tester")
parser.add_argument("--target", "-t", type=str, default="172.20.10.6", help="ESP32 IP Address")
parser.add_argument("--port", "-p", type=int, default=3333, help="UDP Control Port")
parser.add_argument("--rounds", "-n", type=int, default=50, help="Number of control rounds")
args = parser.parse_args()

ESP32_IP = args.target
CMD_PORT = args.port
ROUNDS = args.rounds

client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
client.settimeout(2.0)

print(f"Sending control commands to {ESP32_IP}:{CMD_PORT}...")

rtt_list = []

for i in range(1, ROUNDS + 1):
    cmd = b"LED_ON" if (i % 2 == 1) else b"LED_OFF"
    try:
        t_start = time.perf_counter()
        client.sendto(cmd, (ESP32_IP, CMD_PORT))
        resp, _ = client.recvfrom(128)
        t_end = time.perf_counter()

        rtt_ms = (t_end - t_start) * 1000.0
        rtt_list.append(rtt_ms)
        print(f"Round {i:02d}: Sent '{cmd.decode()}' -> Reply '{resp.decode()}' | RTT: {rtt_ms:6.2f} ms")
    except socket.timeout:
        print(f"Round {i:02d}: Request timed out!")
    time.sleep(0.05)

if rtt_list:
    avg_rtt = sum(rtt_list) / len(rtt_list)
    variance = sum((x - avg_rtt) ** 2 for x in rtt_list) / len(rtt_list)
    jitter = math.sqrt(variance)
    loss_pct = ((ROUNDS - len(rtt_list)) / ROUNDS) * 100.0
    print("\n" + "=" * 68)
    print(f"Average UDP RTT Latency: {avg_rtt:.2f} ms (Min: {min(rtt_list):.2f} ms, Max: {max(rtt_list):.2f} ms, Jitter: {jitter:.2f} ms)")
    print(f"Packet Delivery: {len(rtt_list)}/{ROUNDS} ACK received ({loss_pct:.1f}% Packet Loss)")
    print("=" * 68)
