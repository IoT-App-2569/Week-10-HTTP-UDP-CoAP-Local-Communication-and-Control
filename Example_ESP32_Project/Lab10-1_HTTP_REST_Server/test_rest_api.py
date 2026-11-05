import requests
import time
import sys

BASE_URL = "http://esp32-node.local"
if len(sys.argv) > 1:
    BASE_URL = sys.argv[1].rstrip("/")

print(f"=== Testing ESP32 RESTful API on {BASE_URL} ===\n")

# 1. GET /api/status
try:
    print("1. Testing GET /api/status...")
    r = requests.get(f"{BASE_URL}/api/status", timeout=5)
    print(f"Status Code: {r.status_code}")
    print(f"Response Headers: {dict(r.headers)}")
    print(f"Response Body: {r.text}\n")
except Exception as e:
    print(f"Error connecting to {BASE_URL}: {e}\n")

# 2. POST /api/led (Turn ON)
try:
    print("2. Turning LED ON...")
    r = requests.post(f"{BASE_URL}/api/led", json={"state": True}, timeout=5)
    print(f"Response: {r.text}")
    time.sleep(1)
except Exception as e:
    print(f"Error: {e}\n")

# 3. Adjust Virtual Potentiometer to 3500
try:
    print("3. Adjusting Virtual Potentiometer to 3500 via POST /api/pot...")
    r = requests.post(f"{BASE_URL}/api/pot", json={"pot_raw": 3500}, timeout=5)
    print(f"Response: {r.text}\n")
except Exception as e:
    print(f"Error: {e}\n")

# 4. GET /api/status to verify changes
try:
    print("4. Verifying status after adjustment...")
    r = requests.get(f"{BASE_URL}/api/status", timeout=5)
    print(f"Response Body: {r.text}\n")
except Exception as e:
    print(f"Error: {e}\n")

# 5. POST /api/led (Turn OFF)
try:
    print("5. Turning LED OFF...")
    r = requests.post(f"{BASE_URL}/api/led", json={"state": False}, timeout=5)
    print(f"Response: {r.text}\n")
except Exception as e:
    print(f"Error: {e}\n")
