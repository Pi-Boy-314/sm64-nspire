import sys, os
root = sys.argv[1]; ref = sys.argv[2]
def ppm(p):
    b = open(p, 'rb').read(); parts = b.split(maxsplit=4); w, h = int(parts[1]), int(parts[2]); return parts[4][:w*h*3]
for v in sys.argv[3:]:
    px = tot = big = same = ticks = 0
    for cfg in sorted(os.listdir(os.path.join(root, ref))):
        hv = open(os.path.join(root, v, cfg, 'sm64_bench_hashes.txt')).read().split('\n')
        hr = open(os.path.join(root, ref, cfg, 'sm64_bench_hashes.txt')).read().split('\n')
        same += sum(1 for a, b in zip(hv, hr) if a and a == b); ticks += sum(1 for a in hr if a)
        for t in ('060', '240', '420'):
            A = ppm(os.path.join(root, v, cfg, f'sm64_bench_{t}.ppm.tns')); B = ppm(os.path.join(root, ref, cfg, f'sm64_bench_{t}.ppm.tns'))
            for i in range(0, len(A), 3):
                d = max(abs(A[i+k] - B[i+k]) for k in range(3))
                if d: px += 1; tot += d; big += d > 16
    print(f"{v:4s} vs {ref}: frames identical {same}/{ticks}; in 24 dumped frames: {px} px differ, {big} by >16, total abs diff {tot}")
