#!/usr/bin/env python3
"""Per-function profile of the ARMSIM build from tbprof block counts (see profile.sh).

usage: profile.py ELF PROF_LONG [PROF_SHORT TICKS] [--top N] [--annotate FUNC] [--objdump PATH]

With PROF_SHORT and TICKS, the short run's counts are subtracted from the long run's and the
result divided by TICKS (the difference in measured ticks), giving per-tick figures with boot,
level loading and warmup cancelled out.

Cycles are an estimate from ARM926EJ-S instruction timings with every access hitting the
cache: taken branches 3, conditional branches 2 (taken or not is unknown), MUL/MLA 2,
UMULL/SMULL/UMLAL/SMLAL 3, LDM/STM one per register, loads +1 or +2 when the next instruction
uses the result, loads into pc +4. Real hardware adds cache misses and SDRAM waits.
"""
import argparse
import bisect
import collections
import re
import subprocess

COND = ('eq', 'ne', 'cs', 'hs', 'cc', 'lo', 'mi', 'pl', 'vs', 'vc', 'hi', 'ls', 'ge', 'lt',
        'gt', 'le', 'al')
DATA = {'and', 'eor', 'sub', 'rsb', 'add', 'adc', 'sbc', 'rsc', 'orr', 'mov', 'bic', 'mvn',
        'lsl', 'lsr', 'asr', 'ror', 'rrx'}
COMPARE = {'tst', 'teq', 'cmp', 'cmn'}
MUL2 = {'mul', 'mla', 'mls'}
MUL3 = {'umull', 'umlal', 'smull', 'smlal'}
DSP1 = {'smulbb', 'smulbt', 'smultb', 'smultt', 'smulwb', 'smulwt', 'smlabb', 'smlabt',
        'smlatb', 'smlatt', 'smlawb', 'smlawt', 'qadd', 'qsub', 'qdadd', 'qdsub', 'clz'}
DSP2 = {'smlalbb', 'smlalbt', 'smlaltb', 'smlaltt'}
LOAD_WORD = {'ldr', 'ldrt'}
LOAD_SUB = {'ldrb', 'ldrh', 'ldrsb', 'ldrsh', 'ldrbt'}
STORE = {'str', 'strb', 'strh', 'strt', 'strbt'}
MULTI_LOAD = {'ldm', 'ldmia', 'ldmib', 'ldmda', 'ldmdb', 'ldmfd', 'ldmea', 'pop'}
MULTI_STORE = {'stm', 'stmia', 'stmib', 'stmda', 'stmdb', 'stmfd', 'stmea', 'push'}
BRANCH = {'b', 'bl', 'bx', 'blx'}
KNOWN = (DATA | COMPARE | MUL2 | MUL3 | DSP1 | DSP2 | LOAD_WORD | LOAD_SUB | STORE | MULTI_LOAD
         | MULTI_STORE | BRANCH | {'ldrd', 'strd', 'swp', 'swpb', 'mrs', 'msr', 'svc', 'swi',
                                   'nop', 'pld', 'mcr', 'mrc', 'cdp', 'ldc', 'stc', 'udf', 'bkpt'})
REG = re.compile(r'\b(r1[0-5]|r[0-9]|sl|fp|ip|sp|lr|pc)\b')
REG_SHIFT = re.compile(r'\b(lsl|lsr|asr|ror) (r1[0-5]|r[0-9]|sl|fp|ip|sp|lr)\b')


def decode(mn):
    """Returns (base mnemonic, conditional?)."""
    if mn in KNOWN:
        return mn, False
    if mn.endswith('s') and mn[:-1] in KNOWN:
        return mn[:-1], False
    for c in COND:
        if mn.endswith(c):
            b = mn[:-len(c)]
            if b in KNOWN:
                return b, c != 'al'
            if b.endswith('s') and b[:-1] in KNOWN:
                return b[:-1], c != 'al'
    return mn, False


def reg_count(ops):
    m = re.search(r'\{([^}]*)\}', ops)
    if not m:
        return 1
    n = 0
    for part in m.group(1).split(','):
        part = part.strip()
        if '-' in part:
            a, b = part.split('-')
            n += int(b.strip()[1:]) - int(a.strip()[1:]) + 1
        elif part:
            n += 1
    return n


def sources(insn):
    """Registers an instruction reads (approximate)."""
    base, ops = insn
    regs = REG.findall(ops)
    if base in STORE or base in MULTI_STORE or base in COMPARE or base in ('strd', 'bx', 'blx'):
        return set(regs)
    return set(regs[1:])


def cycles(insn, nxt, nxt2):
    base, ops = insn
    dest = REG.findall(ops)[:1]
    if base in BRANCH:
        return 3
    if base in MUL3:
        return 3
    if base in MUL2 or base in DSP2:
        return 2
    if base in LOAD_WORD or base in LOAD_SUB:
        if dest == ['pc']:
            return 5
        c = 1
        latency = 1 if base in LOAD_WORD else 2
        if nxt is not None and dest and dest[0] in sources(nxt):
            c += latency
        elif latency == 2 and nxt2 is not None and dest and dest[0] in sources(nxt2):
            c += 1
        return c
    if base in ('ldrd', 'strd', 'swp', 'swpb'):
        return 2
    if base in MULTI_LOAD:
        return max(reg_count(ops), 2) + (4 if 'pc' in ops else 0)
    if base in MULTI_STORE:
        return max(reg_count(ops), 2)
    c = 1
    if base in DATA and REG_SHIFT.search(ops):
        c += 1
    if base in DATA and dest == ['pc']:
        c += 2
    return c


def read_prof(path):
    blocks = {}
    with open(path) as f:
        for line in f:
            s, e, n, c = line.split()
            key = (int(s, 16), int(e, 16))
            blocks[key] = blocks.get(key, 0) + int(c)
    return blocks


def disassemble(objdump, elf):
    insns = {}  # addr -> (base, ops, conditional, mnemonic as printed)
    funcs = []  # (start addr, name)
    head = re.compile(r'^([0-9a-f]+) <(.+)>:$')
    line_re = re.compile(r'^\s*([0-9a-f]+):\t(\S+)\t?(.*)$')
    out = subprocess.run([objdump, '-d', '--no-show-raw-insn', elf], capture_output=True,
                         text=True, check=True).stdout
    for line in out.splitlines():
        m = head.match(line)
        if m:
            funcs.append((int(m.group(1), 16), m.group(2)))
            continue
        m = line_re.match(line)
        if m:
            base, cond = decode(m.group(2))
            insns[int(m.group(1), 16)] = (base, m.group(3).split(';')[0].split('@')[0].strip(), cond,
                                          m.group(2))
    funcs.sort()
    return insns, funcs


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('elf')
    ap.add_argument('long')
    ap.add_argument('short', nargs='?')
    ap.add_argument('ticks', nargs='?', type=float, default=1.0)
    ap.add_argument('--top', type=int, default=30)
    ap.add_argument('--annotate', action='append', default=[])
    ap.add_argument('--objdump', default='arm-none-eabi-objdump')
    a = ap.parse_args()

    counts = read_prof(a.long)
    if a.short:
        for k, v in read_prof(a.short).items():
            counts[k] = counts.get(k, 0) - v
    insns, funcs = disassemble(a.objdump, a.elf)
    addrs = sorted(insns)

    per_insn = collections.Counter()
    for (s, e), c in counts.items():
        if c <= 0:
            continue
        i = bisect.bisect_left(addrs, s)
        while i < len(addrs) and addrs[i] <= e:
            per_insn[addrs[i]] += c
            i += 1

    starts = [f[0] for f in funcs]
    fn_insns = collections.Counter()
    fn_cycles = collections.Counter()
    insn_cycles = {}
    for addr, c in per_insn.items():
        i = bisect.bisect_left(addrs, addr)
        cur = insns[addr][:2]
        nxt = insns[addrs[i + 1]][:2] if i + 1 < len(addrs) else None
        nxt2 = insns[addrs[i + 2]][:2] if i + 2 < len(addrs) else None
        cyc = cycles(cur, nxt, nxt2)
        if insns[addr][2] and cur[0] == 'b':
            cyc = 2
        insn_cycles[addr] = cyc
        name = funcs[bisect.bisect_right(starts, addr) - 1][1]
        fn_insns[name] += c
        fn_cycles[name] += c * cyc

    t = a.ticks
    ti, tc = sum(fn_insns.values()), sum(fn_cycles.values())
    print(f'per tick: {ti / t / 1e6:.2f} M instructions, ~{tc / t / 1e6:.2f} M cycles '
          f'(cache hits only), {tc / ti:.2f} cycles/instruction')
    print(f'{"function":34s} {"%cyc":>6s} {"Mcyc":>7s} {"Minsn":>7s} {"CPI":>5s}')
    for name, c in fn_cycles.most_common(a.top):
        print(f'{name[:34]:34s} {100 * c / tc:6.2f} {c / t / 1e6:7.3f} '
              f'{fn_insns[name] / t / 1e6:7.3f} {c / fn_insns[name]:5.2f}')

    for want in a.annotate:
        rng = [(s, funcs[i + 1][0] if i + 1 < len(funcs) else 1 << 32)
               for i, (s, n) in enumerate(funcs) if n == want]
        for s, e in rng:
            print(f'\n== {want} (count per tick, est. cycles each)')
            for addr in addrs:
                if s <= addr < e:
                    c = per_insn.get(addr, 0)
                    _, ops, _, mn = insns[addr]
                    cyc = insn_cycles.get(addr, '')
                    print(f'{addr:8x} {c / t:12.1f} {cyc!s:>2s}  {mn:8s} {ops}')


if __name__ == '__main__':
    main()
