"""วัด RTT ของคำสั่งควบคุม LED ผ่าน HTTP / UDP / CoAP

วิธีใช้:  python3 rtt_benchmark.py <http|udp|coap> <IP ของ ESP32> [จำนวนรอบ]
ต้องแฟลชเฟิร์มแวร์ที่ตรงกับโปรโตคอลลงบอร์ดก่อน (Lab10-1 = http, Lab10-2 = udp, Lab10-3 = coap)
"""
import asyncio
import http.client
import socket
import statistics
import sys
import time
import warnings

warnings.filterwarnings("ignore")

INTERVAL = 0.2  # หน่วงระหว่างรอบ (วินาที)


def bench_http(ip, n):
    rtts, fail = [], 0
    for i in range(n):
        body = '{"state": %s}' % ("true" if i % 2 == 0 else "false")
        t = time.perf_counter()
        try:
            conn = http.client.HTTPConnection(ip, 80, timeout=3)  # เปิด TCP ใหม่ทุกรอบ
            conn.request("POST", "/api/led", body, {"Content-Type": "application/json"})
            conn.getresponse().read()
            conn.close()
            rtts.append((time.perf_counter() - t) * 1000)
        except Exception:
            fail += 1
        time.sleep(INTERVAL)
    return rtts, fail


def bench_udp(ip, n):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.settimeout(2.0)
    rtts, fail = [], 0
    for i in range(n):
        cmd = b"LED_ON" if i % 2 == 0 else b"LED_OFF"
        t = time.perf_counter()
        s.sendto(cmd, (ip, 3333))
        try:
            s.recvfrom(128)
            rtts.append((time.perf_counter() - t) * 1000)
        except socket.timeout:
            fail += 1
        time.sleep(INTERVAL)
    return rtts, fail


def bench_coap(ip, n):
    from aiocoap import CON, Code, Context, Message

    async def run():
        ctx = await Context.create_client_context()
        rtts, fail = [], 0
        for i in range(n):
            payload = b"1" if i % 2 == 0 else b"0"
            msg = Message(code=Code.PUT, mtype=CON, payload=payload, uri=f"coap://{ip}/actuator/led")
            t = time.perf_counter()
            try:
                await asyncio.wait_for(ctx.request(msg).response, 25.0)
                rtts.append((time.perf_counter() - t) * 1000)
            except Exception:
                fail += 1
            await asyncio.sleep(INTERVAL)
        return rtts, fail

    return asyncio.run(run())


def main():
    mode, ip = sys.argv[1], sys.argv[2]
    n = int(sys.argv[3]) if len(sys.argv) > 3 else 50
    rtts, fail = {"http": bench_http, "udp": bench_udp, "coap": bench_coap}[mode](ip, n)
    ok = len(rtts)
    print(f"{mode.upper()} rounds_ok={ok}/{n} packet_loss={fail / n * 100:.1f}%")
    if ok:
        sd = statistics.stdev(rtts) if ok > 1 else 0.0
        print(f"min={min(rtts):.2f} mean={statistics.mean(rtts):.2f} "
              f"median={statistics.median(rtts):.2f} max={max(rtts):.2f} stddev={sd:.2f} ms")


if __name__ == "__main__":
    main()
