#!/bin/sh
# Runs the ARMSIM benchmark build (make ARMSIM=1) on QEMU's versatilepb board, which has an
# ARM926EJ-S like the TI-Nspire CX. Run it from a folder holding sm64_config.txt.tns; results go
# to sm64_bench.txt.tns, as on a calculator. QEMU's -icount makes the timer count instructions,
# so the "ms" figures there are millions of instructions, not time on a calculator.
# Usage: run.sh [path/to/sm64.us.f3dex2]
here=$(cd "$(dirname "$0")" && pwd)
elf=${1:-$here/../../build/us_nsp_armsim/sm64.us.f3dex2}
exec qemu-system-arm -M versatilepb -cpu arm926 -m 256M \
    -display none -monitor none -serial none -nodefaults -audio none \
    -icount shift=0,align=off,sleep=off \
    -semihosting-config enable=on,target=native \
    -kernel "$elf"
