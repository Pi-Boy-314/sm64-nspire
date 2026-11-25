import sys, os
root = sys.argv[1]
def hashes(p):
    return [l.split() for l in open(p) if l.strip()]
def ppm(p):
    b = open(p, 'rb').read()
    parts = b.split(maxsplit=4)  # P6 w h max data
    assert parts[0] == b'P6', parts[0]
    w, h = int(parts[1]), int(parts[2])
    return w, h, parts[4][:w*h*3]
def pix(a, b):
    w, h, da = ppm(a); _, _, db = ppm(b)
    n = w * h; ndiff = 0; mx = 0; tot = 0
    for i in range(n):
        d = max(abs(da[3*i+k] - db[3*i+k]) for k in range(3))
        if d: ndiff += 1; tot += d; mx = max(mx, d)
    return ndiff, n, mx, (tot / ndiff if ndiff else 0)
pairs = [(x, y) for x, y in [tuple(s.split(':')) for s in sys.argv[2:]]]
for x, y in pairs:
    print(f"== {x} vs {y}")
    for cfg in sorted(os.listdir(os.path.join(root, x))):
        hx = hashes(os.path.join(root, x, cfg, 'sm64_bench_hashes.txt'))
        hy = hashes(os.path.join(root, y, cfg, 'sm64_bench_hashes.txt'))
        same = sum(1 for a, b in zip(hx, hy) if a == b)
        first = next((a[0] for a, b in zip(hx, hy) if a != b), '-')
        out = f"{cfg:20s} ticks {len(hx)}/{len(hy)} identical {same:3d} first-diff {first:>4s} |"
        for t in ('060', '240', '420'):
            fa = os.path.join(root, x, cfg, f'sm64_bench_{t}.ppm.tns'); fb = os.path.join(root, y, cfg, f'sm64_bench_{t}.ppm.tns')
            if os.path.exists(fa) and os.path.exists(fb):
                nd, n, mx, avg = pix(fa, fb)
                out += f" t{t}: {100*nd/n:5.2f}% px, max {mx:3d}, avg {avg:4.1f} |"
        print(out)
