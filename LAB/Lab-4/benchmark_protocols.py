#!/usr/bin/env python3
"""
IoT Local Communication and Control Benchmark Tool
Week 10: HTTP vs UDP vs CoAP Protocol Performance Analyzer

This script benchmarks the firmware projects from Labs 10.1, 10.2, and 10.3.
It is intended to be run from a computer connected to the same network as the
ESP32.
"""

import argparse
import asyncio
import json
import math
import socket
import sys
import time
import urllib.request
from typing import Dict, List

try:
    from aiocoap import Code, Context, Message
except ImportError:
    Code = None
    Context = None
    Message = None


DEFAULT_TARGET = "10.14.195.186"
DEFAULT_ROUNDS = 50
DEFAULT_TIMEOUT = 2.0
UDP_CONTROL_PORT = 3333
COAP_PORT = 5683


class BenchmarkStats:
    """Calculates statistical metrics for benchmark rounds."""

    def __init__(
        self,
        latencies: List[float],
        sent_bytes: int,
        received_bytes: int,
        total_rounds: int,
    ) -> None:
        self.total_rounds = total_rounds
        self.success_rounds = len(latencies)
        self.failed_rounds = total_rounds - self.success_rounds
        self.latencies = latencies
        self.sent_bytes = sent_bytes
        self.received_bytes = received_bytes

        if self.success_rounds:
            self.min_rtt = min(latencies)
            self.max_rtt = max(latencies)
            self.mean_rtt = sum(latencies) / self.success_rounds
            variance = sum(
                (latency - self.mean_rtt) ** 2 for latency in latencies
            ) / self.success_rounds
            self.jitter = math.sqrt(variance)
            self.packet_loss_pct = self.failed_rounds / total_rounds * 100
        else:
            self.min_rtt = 0.0
            self.max_rtt = 0.0
            self.mean_rtt = 0.0
            self.jitter = 0.0
            self.packet_loss_pct = 100.0


def benchmark_http(target: str, rounds: int, timeout: float) -> BenchmarkStats:
    print(
        f"\n[*] Starting HTTP REST Benchmark -> "
        f"http://{target}/api/led ({rounds} rounds)..."
    )
    url = f"http://{target}/api/led"
    latencies: List[float] = []
    total_sent = 0
    total_received = 0
    headers = {
        "Content-Type": "application/json",
        "Connection": "keep-alive",
    }

    for round_number in range(1, rounds + 1):
        state = round_number % 2 == 1
        payload = json.dumps({"state": state}).encode("utf-8")
        request = urllib.request.Request(
            url, data=payload, headers=headers, method="POST"
        )

        start = time.perf_counter()
        try:
            with urllib.request.urlopen(request, timeout=timeout) as response:
                response_data = response.read()
                elapsed = (time.perf_counter() - start) * 1000
                latencies.append(elapsed)
                total_sent += len(payload) + 180
                total_received += len(response_data) + 120
                print(
                    f"  [HTTP Round {round_number:03d}/{rounds:03d}] "
                    f"RTT: {elapsed:6.2f} ms | Status: {response.status}"
                )
        except (OSError, ValueError) as error:
            print(
                f"  [HTTP Round {round_number:03d}/{rounds:03d}] "
                f"FAILED: {error}"
            )

        time.sleep(0.02)

    return BenchmarkStats(latencies, total_sent, total_received, rounds)


def benchmark_udp(
    target: str,
    rounds: int,
    timeout: float,
    port: int = UDP_CONTROL_PORT,
) -> BenchmarkStats:
    print(
        f"\n[*] Starting UDP Socket Benchmark -> "
        f"{target}:{port} ({rounds} rounds)..."
    )
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(timeout)
    latencies: List[float] = []
    total_sent = 0
    total_received = 0

    try:
        for round_number in range(1, rounds + 1):
            command = b"LED_ON" if round_number % 2 == 1 else b"LED_OFF"
            start = time.perf_counter()
            try:
                sock.sendto(command, (target, port))
                total_sent += len(command) + 8
                response, _ = sock.recvfrom(128)
                elapsed = (time.perf_counter() - start) * 1000
                latencies.append(elapsed)
                total_received += len(response) + 8
                print(
                    f"  [UDP Round {round_number:03d}/{rounds:03d}] "
                    f"RTT: {elapsed:6.2f} ms | "
                    f"Recv: {response.decode('utf-8', errors='replace')}"
                )
            except socket.timeout:
                print(
                    f"  [UDP Round {round_number:03d}/{rounds:03d}] "
                    "TIMEOUT (Packet Dropped)"
                )
            except OSError as error:
                print(
                    f"  [UDP Round {round_number:03d}/{rounds:03d}] "
                    f"ERROR: {error}"
                )

            time.sleep(0.02)
    finally:
        sock.close()

    return BenchmarkStats(latencies, total_sent, total_received, rounds)


async def benchmark_coap_async(
    target: str,
    rounds: int,
    timeout: float,
    port: int = COAP_PORT,
) -> BenchmarkStats:
    if Context is None or Code is None or Message is None:
        print("[!] Error: 'aiocoap' is not installed. Run: pip install aiocoap")
        return BenchmarkStats([], 0, 0, rounds)

    print(
        f"\n[*] Starting CoAP Benchmark -> "
        f"coap://{target}:{port}/actuator/led ({rounds} rounds)..."
    )
    protocol = await Context.create_client_context()
    latencies: List[float] = []
    total_sent = 0
    total_received = 0

    try:
        for round_number in range(1, rounds + 1):
            value = b"1" if round_number % 2 == 1 else b"0"
            request = Message(
                code=Code.PUT,
                payload=value,
                uri=f"coap://{target}:{port}/actuator/led",
            )
            start = time.perf_counter()
            try:
                response = await asyncio.wait_for(
                    protocol.request(request).response,
                    timeout=timeout,
                )
                elapsed = (time.perf_counter() - start) * 1000
                latencies.append(elapsed)
                total_sent += len(value) + 4 + 8
                total_received += 4 + 8
                print(
                    f"  [CoAP Round {round_number:03d}/{rounds:03d}] "
                    f"RTT: {elapsed:6.2f} ms | Code: {response.code}"
                )
            except asyncio.TimeoutError:
                print(
                    f"  [CoAP Round {round_number:03d}/{rounds:03d}] "
                    "TIMEOUT (No ACK)"
                )
            except (OSError, ValueError) as error:
                print(
                    f"  [CoAP Round {round_number:03d}/{rounds:03d}] "
                    f"ERROR: {error}"
                )

            await asyncio.sleep(0.02)
    finally:
        await protocol.shutdown()

    return BenchmarkStats(latencies, total_sent, total_received, rounds)


def benchmark_coap(
    target: str,
    rounds: int,
    timeout: float,
) -> BenchmarkStats:
    return asyncio.run(benchmark_coap_async(target, rounds, timeout))


def display_results(results: Dict[str, BenchmarkStats]) -> None:
    print("\n" + "=" * 100)
    print("BENCHMARK RESULTS & METRICS SUMMARY")
    print("=" * 100)
    print(
        f"{'Protocol':<16} | {'Success':<10} | {'Loss %':<8} | "
        f"{'Min (ms)':<10} | {'Mean (ms)':<11} | {'Max (ms)':<10} | "
        f"{'Jitter (ms)':<12}"
    )
    print("-" * 100)

    for protocol_name, stats in results.items():
        if stats.success_rounds:
            print(
                f"{protocol_name:<16} | "
                f"{stats.success_rounds}/{stats.total_rounds:<8} | "
                f"{stats.packet_loss_pct:>6.1f}% | "
                f"{stats.min_rtt:>9.2f} | {stats.mean_rtt:>10.2f} | "
                f"{stats.max_rtt:>9.2f} | {stats.jitter:>11.2f}"
            )
        else:
            print(
                f"{protocol_name:<16} | "
                f"{stats.success_rounds}/{stats.total_rounds:<8} | "
                f"{stats.packet_loss_pct:>6.1f}% | "
                f"{'N/A':>9} | {'N/A':>10} | {'N/A':>9} | {'N/A':>11}"
            )

    print("=" * 100)
    print("\nTHEORETICAL PROTOCOL OVERHEAD ANALYSIS (Per LED Command)")
    print("-" * 100)
    print(
        f"{'Layer / Attribute':<30} | {'HTTP REST (POST)':<18} | "
        f"{'UDP Socket':<14} | {'CoAP (PUT)':<14}"
    )
    print("-" * 100)
    print(
        f"{'Transport Layer (L4)':<30} | {'TCP (20-32 bytes)':<18} | "
        f"{'UDP (8 bytes)':<14} | {'UDP (8 bytes)':<14}"
    )
    print(
        f"{'Application Header (L7)':<30} | {'~150-250 bytes':<18} | "
        f"{'0 bytes':<14} | {'4-8 bytes':<14}"
    )
    print(
        f"{'Command Payload Size':<30} | {'14 bytes (JSON)':<18} | "
        f"{'6 bytes (ASCII)':<14} | {'1 byte (char)':<14}"
    )
    print(
        f"{'Est. Total Bytes per Request':<30} | {'~190-300 bytes':<18} | "
        f"{'~14 bytes':<14} | {'~13-17 bytes':<14}"
    )
    print(
        f"{'Payload Efficiency Ratio':<30} | {'~4.5 - 7.0 %':<18} | "
        f"{'~42.8 %':<14} | {'~7.7 - 25.0 %':<14}"
    )
    print("=" * 100)


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Benchmark HTTP, UDP, and CoAP on an ESP32."
    )
    parser.add_argument(
        "--target",
        "-t",
        default=DEFAULT_TARGET,
        help=f"ESP32 IP address or hostname (default: {DEFAULT_TARGET})",
    )
    parser.add_argument(
        "--protocol",
        "-p",
        choices=("http", "udp", "coap", "all", "interactive"),
        default="interactive",
        help="Protocol to benchmark (default: interactive)",
    )
    parser.add_argument(
        "--rounds",
        "-n",
        type=int,
        default=DEFAULT_ROUNDS,
        help=f"Number of rounds per protocol (default: {DEFAULT_ROUNDS})",
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=DEFAULT_TIMEOUT,
        help=f"Per-request timeout in seconds (default: {DEFAULT_TIMEOUT})",
    )
    args = parser.parse_args()

    protocol = args.protocol
    if protocol == "interactive":
        print(f"Target ESP32 Device: {args.target}")
        print(f"Number of Rounds:   {args.rounds}\n")
        print("[1] Benchmark HTTP REST Server  (Lab 10.1 Firmware)")
        print("[2] Benchmark UDP Socket Server (Lab 10.2 Firmware)")
        print("[3] Benchmark CoAP Server       (Lab 10.3 Firmware)")
        print("[4] Benchmark ALL 3 Protocols Sequentially")
        print("[0] Exit")
        choice = input("\nEnter your choice [1-4]: ").strip()
        protocol = {
            "1": "http",
            "2": "udp",
            "3": "coap",
            "4": "all",
        }.get(choice, "")
        if not protocol:
            print("Exiting.")
            return

    results: Dict[str, BenchmarkStats] = {}
    if protocol in ("http", "all"):
        results["HTTP (REST)"] = benchmark_http(
            args.target, args.rounds, args.timeout
        )
    if protocol in ("udp", "all"):
        results["UDP (Socket)"] = benchmark_udp(
            args.target, args.rounds, args.timeout
        )
    if protocol in ("coap", "all"):
        results["CoAP (RFC7252)"] = benchmark_coap(
            args.target, args.rounds, args.timeout
        )

    display_results(results)


if __name__ == "__main__":
    main()
