import sys, re, os
# sum.py NAME...: million ARM instructions per drawn frame for armbench.sh runs
repo = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
root = os.environ.get('SP', os.path.join(repo, 'bench-work')) + '/armsim'
for v in sys.argv[1:]:
    out = [v.ljust(8)]
    for r in (120, 240):
        tot = ras = fr = 0; per = []
        for d in range(4):
            t = open(os.path.join(root, v, f'd{d}_{r}', 'sm64_bench.txt.tns')).read()
            g = lambda k: float(re.search(k + r'\s+([\d.]+)', t).group(1))
            tot += g('total'); ras += g(r'raster \(triangles\)'); fr += g(r'front \(DL, vertices\)'); per.append(g('total'))
        out.append(f'{r}p total {tot/4:6.2f} raster {ras/4:6.2f} front {fr/4:5.2f} [' + ' '.join(f'{x:.1f}' for x in per) + ']')
    print('  '.join(out))
