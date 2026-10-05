#!/usr/bin/env python3
"""
================================================================================
IoT Local Communication & Control Benchmark Tool
Week 10: HTTP vs UDP vs CoAP Protocol Performance Analyzer
================================================================================
Author: IoT Systems Engineering Laboratory
Purpose: Benchmarking Round-Trip Time (RTT), Jitter, Packet Reliability,
         and Protocol Overhead for ESP32 Local Control.
================================================================================
"""

import sys
import time
import math
import socket
import argparse
import asyncio
from typing import List, Dict, Any, Optional

# Protocol Specific Imports with Graceful Fallbacks
try:
    import urllib.request
    import json
except ImportError:
    pass

try:
    from aiocoap import Context, Message, Code
except ImportError:
    pass


class BenchmarkStats:
    """Calculates statistical metrics for benchmark rounds."""
    def __init__(self, latencies: List[float], sent_bytes: int, recv_bytes: int, total_rounds: int):
        self.total_rounds = total_rounds
        self.success_rounds = len(latencies)
        self.failed_rounds = total_rounds - self.success_rounds
        self.latencies = latencies
        self.sent_bytes = sent_bytes
        self.recv_bytes = recv_bytes

        if self.success_rounds > 0:
            self.min_rtt = min(latencies)
            self.max_rtt = max(latencies)
            self.mean_rtt = sum(latencies) / self.success_rounds
            variance = sum((x - self.mean_rtt) ** 2 for x in latencies) / self.success_rounds
            self.std_dev = math.sqrt(variance)
            self.packet_loss_pct = (self.failed_rounds / self.total_rounds) * 100.0
        else:
            self.min_rtt = 0.0
            self.max_rtt = 0.0
            self.mean_rtt = 0.0
            self.std_dev = 0.0
            self.packet_loss_pct = 100.0


def print_banner():
    banner = r"""
================================================================================
   ESP32 Local Protocol Benchmark: HTTP vs UDP vs CoAP
   Industrial IoT Systems Engineering - Real-time Performance Lab
================================================================================
"""
    print(banner)


# ------------------------------------------------------------------------------
# 1. HTTP REST Benchmark
# ------------------------------------------------------------------------------
def benchmark_http(target: str, rounds: int, timeout: float) -> BenchmarkStats:
    print(f"\n[*] Starting HTTP REST Benchmark -> http://{target}/api/led ({rounds} rounds)...")
    url = f"http://{target}/api/led"
    latencies: List[float] = []
    total_tx_bytes = 0
    total_rx_bytes = 0

    headers = {
        "Content-Type": "application/json",
        "Connection": "keep-alive"
    }

    for i in range(1, rounds + 1):
        state = (i % 2 == 1)
        payload = json.dumps({"state": state}).encode("utf-8")
        req = urllib.request.Request(url, data=payload, headers=headers, method="POST")

        t_start = time.perf_counter()
        try:
            with urllib.request.urlopen(req, timeout=timeout) as response:
                resp_data = response.read()
                t_end = time.perf_counter()
                rtt = (t_end - t_start) * 1000.0  # ms
                latencies.append(rtt)
                total_tx_bytes += len(payload) + 180  # approx HTTP POST header size
                total_rx_bytes += len(resp_data) + 120  # approx HTTP 200 header size
                print(f"  [HTTP Round {i:03d}/{rounds:03d}] RTT: {rtt:6.2f} ms | Status: {response.status}")
        except Exception as e:
            print(f"  [HTTP Round {i:03d}/{rounds:03d}] FAILED: {e}")

        time.sleep(0.02)  # 20ms pacing

    return BenchmarkStats(latencies, total_tx_bytes, total_rx_bytes, rounds)


# ------------------------------------------------------------------------------
# 2. UDP Raw Socket Benchmark
# ------------------------------------------------------------------------------
def benchmark_udp(target: str, rounds: int, timeout: float, port: int = 3333) -> BenchmarkStats:
    print(f"\n[*] Starting UDP Socket Benchmark -> {target}:{port} ({rounds} rounds)...")
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(timeout)
    latencies: List[float] = []
    total_tx_bytes = 0
    total_rx_bytes = 0

    for i in range(1, rounds + 1):
        cmd = "LED_ON" if (i % 2 == 1) else "LED_OFF"
        tx_data = cmd.encode("utf-8")

        t_start = time.perf_counter()
        try:
            sock.sendto(tx_data, (target, port))
            total_tx_bytes += len(tx_data) + 8  # 8 bytes UDP header
            rx_data, _ = sock.recvfrom(128)
            t_end = time.perf_counter()
            rtt = (t_end - t_start) * 1000.0  # ms
            latencies.append(rtt)
            total_rx_bytes += len(rx_data) + 8
            print(f"  [UDP Round  {i:03d}/{rounds:03d}] RTT: {rtt:6.2f} ms | Recv: {rx_data.decode('utf-8', errors='ignore')}")
        except socket.timeout:
            print(f"  [UDP Round  {i:03d}/{rounds:03d}] TIMEOUT (Packet Dropped)")
        except Exception as e:
            print(f"  [UDP Round  {i:03d}/{rounds:03d}] ERROR: {e}")

        time.sleep(0.02)

    sock.close()
    return BenchmarkStats(latencies, total_tx_bytes, total_rx_bytes, rounds)


# ------------------------------------------------------------------------------
# 3. CoAP (RFC 7252) Benchmark
# ------------------------------------------------------------------------------
async def _async_benchmark_coap(target: str, rounds: int, timeout: float, port: int = 5683) -> BenchmarkStats:
    print(f"\n[*] Starting CoAP Benchmark -> coap://{target}:{port}/actuator/led ({rounds} rounds)...")
    protocol = await Context.create_client_context()
    latencies: List[float] = []
    total_tx_bytes = 0
    total_rx_bytes = 0

    for i in range(1, rounds + 1):
        val = b"1" if (i % 2 == 1) else b"0"
        request = Message(code=Code.PUT, payload=val, uri=f"coap://{target}:{port}/actuator/led")

        t_start = time.perf_counter()
        try:
            response = await asyncio.wait_for(protocol.request(request).response, timeout=timeout)
            t_end = time.perf_counter()
            rtt = (t_end - t_start) * 1000.0  # ms
            latencies.append(rtt)
            total_tx_bytes += len(val) + 4 + 8  # CoAP 4-byte header + UDP 8-byte
            total_rx_bytes += 4 + 8  # CoAP response header + UDP
            print(f"  [CoAP Round {i:03d}/{rounds:03d}] RTT: {rtt:6.2f} ms | Code: {response.code}")
        except asyncio.TimeoutError:
            print(f"  [CoAP Round {i:03d}/{rounds:03d}] TIMEOUT (No ACK)")
        except Exception as e:
            print(f"  [CoAP Round {i:03d}/{rounds:03d}] ERROR: {e}")

        await asyncio.sleep(0.02)

    return BenchmarkStats(latencies, total_tx_bytes, total_rx_bytes, rounds)


def benchmark_coap(target: str, rounds: int, timeout: float) -> BenchmarkStats:
    if "aiocoap" not in sys.modules:
        try:
            import aiocoap
        except ImportError:
            print("[!] Error: 'aiocoap' is not installed. Please run: pip install aiocoap")
            return BenchmarkStats([], 0, 0, rounds)

    return asyncio.run(_async_benchmark_coap(target, rounds, timeout))


# ------------------------------------------------------------------------------
# 4. Result Presentation & Comparison Matrix
# ------------------------------------------------------------------------------
def display_results(results: Dict[str, BenchmarkStats]):
    print("\n" + "=" * 80)
    print("                    BENCHMARK RESULTS & METRICS SUMMARY")
    print("=" * 80)
    print(f"{'Protocol':<12} | {'Success':<9} | {'Loss %':<7} | {'Min (ms)':<9} | {'Mean (ms)':<10} | {'Max (ms)':<9} | {'Jitter (ms)':<11}")
    print("-" * 80)

    for proto_name, stats in results.items():
        if stats.success_rounds > 0:
            print(f"{proto_name:<12} | {stats.success_rounds}/{stats.total_rounds:<7} | {stats.packet_loss_pct:>5.1f}% | {stats.min_rtt:>8.2f} | {stats.mean_rtt:>9.2f} | {stats.max_rtt:>8.2f} | {stats.std_dev:>10.2f}")
        else:
            print(f"{proto_name:<12} | {stats.success_rounds}/{stats.total_rounds:<7} | {stats.packet_loss_pct:>5.1f}% | {'N/A':>8} | {'N/A':>9} | {'N/A':>8} | {'N/A':>10}")

    print("=" * 80)

    # Architectural Overhead Comparison Table
    print("\n" + "=" * 80)
    print("             THEORETICAL PROTOCOL OVERHEAD ANALYSIS (Per LED Command)")
    print("=" * 80)
    print(f"{'Layer / Attribute':<30} | {'HTTP REST (POST)':<17} | {'UDP Socket':<13} | {'CoAP (PUT)':<12}")
    print("-" * 80)
    print(f"{'Transport Layer (L4)':<30} | {'TCP (20-32 bytes)':<17} | {'UDP (8 bytes)':<13} | {'UDP (8 bytes)':<12}")
    print(f"{'Application Header (L7)':<30} | {'~150-250 bytes':<17} | {'0 bytes':<13} | {'4-8 bytes':<12}")
    print(f"{'Command Payload Size':<30} | {'14 bytes (JSON)':<17} | {'6 bytes (ASCII)':<13} | {'1 byte (char)':<12}")
    print(f"{'Est. Total Bytes per Request':<30} | {'~190-300 bytes':<17} | {'~14 bytes':<13} | {'~13-17 bytes':<12}")
    print(f"{'Payload Efficiency Ratio':<30} | {'~4.5 - 7.0 %':<17} | {'~42.8 %':<13} | {'~7.7 - 25.0 %':<12}")
    print("=" * 80)
    print("\n[✔] Benchmark Complete! Copy the metrics above into your Lab 10.4 Report.\n")


# ------------------------------------------------------------------------------
# Main Entry Point
# ------------------------------------------------------------------------------
def main():
    print_banner()

    parser = argparse.ArgumentParser(description="IoT Protocol Benchmark Suite (HTTP vs UDP vs CoAP)")
    parser.add_argument("--target", "-t", type=str, default="192.168.1.41",
                        help="Target ESP32 IP address or hostname (default: 192.168.1.41)")
    parser.add_argument("--protocol", "-p", type=str, choices=["http", "udp", "coap", "all", "interactive"],
                        default="interactive", help="Protocol to benchmark: http, udp, coap, all, or interactive")
    parser.add_argument("--rounds", "-n", type=int, default=50,
                        help="Number of test rounds per protocol (default: 50)")
    parser.add_argument("--timeout", type=float, default=2.0,
                        help="Per-request timeout in seconds (default: 2.0s)")

    args = parser.parse_args()
    target = args.target
    rounds = args.rounds
    timeout = args.timeout

    proto = args.protocol
    if proto == "interactive":
        print(f"Target ESP32 Device: {target}")
        print(f"Number of Rounds:   {rounds}\n")
        print("Select Benchmark Option:")
        print("  [1] Benchmark HTTP REST Server  (Lab 10.1 Firmware)")
        print("  [2] Benchmark UDP Socket Server (Lab 10.2 Firmware)")
        print("  [3] Benchmark CoAP Server       (Lab 10.3 Firmware)")
        print("  [4] Benchmark ALL 3 Protocols Sequentially")
        print("  [0] Exit")

        choice = input("\nEnter your choice [1-4]: ").strip()
        if choice == "1":
            proto = "http"
        elif choice == "2":
            proto = "udp"
        elif choice == "3":
            proto = "coap"
        elif choice == "4":
            proto = "all"
        else:
            print("Exiting.")
            sys.exit(0)

    results: Dict[str, BenchmarkStats] = {}

    if proto in ["http", "all"]:
        results["HTTP (REST)"] = benchmark_http(target, rounds, timeout)

    if proto in ["udp", "all"]:
        results["UDP (Socket)"] = benchmark_udp(target, rounds, timeout)

    if proto in ["coap", "all"]:
        results["CoAP (RFC7252)"] = benchmark_coap(target, rounds, timeout)

    display_results(results)


if __name__ == "__main__":
    main()
