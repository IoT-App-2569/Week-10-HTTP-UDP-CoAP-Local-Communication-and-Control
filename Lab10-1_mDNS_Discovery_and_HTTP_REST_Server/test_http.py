#!/usr/bin/env python3
"""
Test script for Lab 10-1 HTTP REST Server
Tests GET /api/status and POST /api/led (ON / OFF)
"""
import time
import requests

ESP32_HOST = "esp32-node.local"  # หรือใช้ IP เช่น "192.168.1.181"
BASE_URL = f"http://{ESP32_HOST}"

def test_get_status():
    print(f"\n--- 1. Testing GET {BASE_URL}/api/status ---")
    try:
        t0 = time.perf_counter()
        res = requests.get(f"{BASE_URL}/api/status", timeout=3.0)
        t1 = time.perf_counter()
        print(f"Status Code: {res.status_code}")
        print(f"Headers:\n{res.headers}")
        print(f"Body: {res.text}")
        print(f"Latency: {(t1 - t0)*1000:.2f} ms")
    except Exception as e:
        print(f"Error: {e}")

def test_post_led(state: bool):
    print(f"\n--- 2. Testing POST {BASE_URL}/api/led (state={state}) ---")
    try:
        t0 = time.perf_counter()
        res = requests.post(
            f"{BASE_URL}/api/led",
            json={"state": state},
            headers={"Content-Type": "application/json"},
            timeout=3.0
        )
        t1 = time.perf_counter()
        print(f"Status Code: {res.status_code}")
        print(f"Body: {res.text}")
        print(f"Latency: {(t1 - t0)*1000:.2f} ms")
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    test_get_status()
    time.sleep(1)
    test_post_led(True)   # Turn LED ON
    time.sleep(2)
    test_post_led(False)  # Turn LED OFF
    time.sleep(1)
    test_get_status()
