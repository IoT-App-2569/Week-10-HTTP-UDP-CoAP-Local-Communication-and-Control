import warnings
warnings.filterwarnings("ignore")
import asyncio, sys, time, statistics
from aiocoap import Context, Message, Code, CON, NON
ip=sys.argv[1]; n=int(sys.argv[2])
async def one(ctx, mt):
    m=Message(code=Code.GET, mtype=mt, uri=f"coap://{ip}/sensor/pot")
    t=time.perf_counter()
    try:
        r=await asyncio.wait_for(ctx.request(m).response, 3.0)
        return (time.perf_counter()-t)*1000, r
    except Exception as e:
        return None, None
async def main():
    ctx=await Context.create_client_context()
    for name,mt in (("CON",CON),("NON",NON)):
        m=Message(code=Code.GET, mtype=mt, uri=f"coap://{ip}/sensor/pot")
        ok=[];fail=0;rt=None
        for i in range(n):
            ms,r=await one(ctx,mt)
            if ms is None: fail+=1
            else: ok.append(ms); rt=r.mtype
            await asyncio.sleep(0.1)
        ok.sort()
        print(f"{name}: sent={n} ok={len(ok)} fail={fail} loss={fail/n*100:.1f}% response_type={rt} avg={statistics.mean(ok):.1f} median={statistics.median(ok):.1f} min={ok[0]:.1f} max={ok[-1]:.1f} ms")
asyncio.run(main())
