import warnings
warnings.filterwarnings("ignore")
import asyncio, sys, time, statistics, warnings
warnings.filterwarnings("ignore")
from aiocoap import Context, Message, Code, CON, NON
ip=sys.argv[1]; n=int(sys.argv[2])
async def one(ctx, mt, to):
    m=Message(code=Code.GET, mtype=mt, uri=f"coap://{ip}/sensor/pot")
    t=time.perf_counter()
    try:
        await asyncio.wait_for(ctx.request(m).response, to)
        return (time.perf_counter()-t)*1000
    except Exception:
        return None
async def main():
    ctx=await Context.create_client_context()
    for name,mt,to in (("NON",NON,6.0),("CON",CON,25.0)):
        res=await asyncio.gather(*[one(ctx,mt,to) for _ in range(n)])
        ok=sorted(x for x in res if x is not None)
        print(f"{name} burst {n} at once: ok={len(ok)} fail={n-len(ok)} loss={(n-len(ok))/n*100:.1f}% avg={statistics.mean(ok):.0f} median={statistics.median(ok):.0f} max={ok[-1]:.0f} ms" if ok else f"{name}: all failed")
        await asyncio.sleep(5)
asyncio.run(main())
