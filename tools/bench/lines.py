import sys, subprocess, collections
ann, elf = sys.argv[1:3]; top = int(sys.argv[3]) if len(sys.argv) > 3 else 20
rows = []
for l in open(ann):
    p = l.split(None, 4)
    if len(p) >= 4 and p[1].replace('.', '').isdigit() and p[2].isdigit():
        c = float(p[1]) * int(p[2])
        if c > 0: rows.append((p[0], c))
out = subprocess.run(['arm-none-eabi-addr2line', '-i', '-e', elf] + ['0x' + a for a, _ in rows], capture_output=True, text=True).stdout
# with -i, each address may print several lines; use -a to delimit
out = subprocess.run(['arm-none-eabi-addr2line', '-a', '-i', '-e', elf] + ['0x' + a for a, _ in rows], capture_output=True, text=True).stdout.split('\n')
by = collections.Counter(); cur = None; cyc = dict(rows); first = False
for line in out:
    if line.startswith('0x'): cur = line[2:].lstrip('0') or '0'; first = True; continue
    if cur and first and line:
        key = next((a for a in cyc if a.lstrip('0') == cur), None)
        if key: by[line.split('/')[-1]] += cyc[key]
        first = False
for k, v in by.most_common(top): print(f"{v/1e6:6.2f} Mcyc  {k}")
