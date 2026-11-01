# Lab 10.4 Protocol Benchmark Results

**Student:** 67030142 Piyawit Pholsri  
**Section:** TechCom  
**Network:** 2.4 GHz local Wi-Fi  
**Target:** ESP32 at `192.168.1.33`  
**Test:** `benchmark_protocols.py`, 50 rounds per firmware

## Actual benchmark results

| Protocol | Success | Loss | Min RTT | Mean RTT | Max RTT | Jitter |
|---|---:|---:|---:|---:|---:|---:|
| HTTP REST | 49/50 | 2.0% | 58.80 ms | 123.96 ms | 1091.39 ms | 147.65 ms |
| UDP Socket | 48/50 | 4.0% | 2.56 ms | 6.08 ms | 21.86 ms | 3.89 ms |
| CoAP | 50/50 | 0.0% | 6.49 ms | 9.80 ms | 49.30 ms | 5.90 ms |

## Protocol overhead

| Item | HTTP REST | UDP Socket | CoAP |
|---|---:|---:|---:|
| Transport header | TCP 20–32 bytes | UDP 8 bytes | UDP 8 bytes |
| Application header | about 150–250 bytes | 0 bytes | 4–8 bytes |
| Command payload | 14 bytes | 6 bytes | 1 byte |
| Estimated total | about 190–300 bytes | about 14 bytes | about 13–17 bytes |

## Conclusion

UDP had the lowest mean RTT and overhead. CoAP was slightly slower but achieved 0% loss and provides standard resource-based IoT communication. HTTP was easiest to integrate with web applications but had the highest latency, jitter, and overhead.

Raw outputs are stored in `Evidence/Lab10-4-HTTP-Benchmark.txt`, `Evidence/Lab10-4-UDP-Benchmark.txt`, and `Evidence/Lab10-4-CoAP-Benchmark.txt`.
