#!/usr/bin/env python3
"""Run the clamped 3x3 blur on the Verilated RAPTOR GPU and diff against blur_ref.

Standalone co-sim check (no QEMU): proves the harness drives the real RTL
correctly before the QEMU device is attached.
"""
import os
import random
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TOOLS = os.path.join(ROOT, "rtl/RAPTOR-SoC/simt_gpu/tools")
sys.path.insert(0, TOOLS)
import assemble  # noqa: E402
import blur_ref  # noqa: E402

SRC, DST = 0x2000, 0x6000


def run(size, seed):
    W = H = size
    rng = random.Random(seed)
    frame = [rng.getrandbits(24) for _ in range(W * H)]
    with tempfile.TemporaryDirectory() as d:
        jobs = []
        for name, kind in (("i", ".gen_blur"), ("b", ".gen_blur_border")):
            prog, meta = assemble.assemble(
                f"{kind} {W} {H}\n.src {SRC:#x}\n.dst {DST:#x}\n")[:2]
            path = os.path.join(d, f"prog_{name}.hex")
            with open(path, "w") as f:
                f.writelines(f"{w & 0xFFFF:04x}\n" for w in prog)
            jobs += [path, str(meta["threads"])]
        fpath, opath = os.path.join(d, "frame.hex"), os.path.join(d, "out.hex")
        with open(fpath, "w") as f:
            f.writelines(f"{p:08x}\n" for p in frame)
        r = subprocess.run([os.path.join(ROOT, "cosim/build/run_blur"), fpath, opath,
                            hex(SRC), hex(DST)] + jobs, capture_output=True, text=True)
        sys.stdout.write(r.stdout)
        if r.returncode:
            sys.exit(f"harness failed: {r.stderr}")
        out = [int(x, 16) for x in open(opath).read().split()]
    ref = blur_ref.box_blur_clamped(frame, W, H)
    bad = [i for i in range(W * H) if out[i] != ref[i]]
    print(f"{W}x{H}: {len(bad)} mismatches of {W*H} pixels")
    return len(bad)


if __name__ == "__main__":
    total = sum(run(s, 1000 + s) for s in (16, 32, 64))
    sys.exit(1 if total else 0)
