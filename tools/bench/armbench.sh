#!/bin/sh
# armbench.sh NAME [ELF]: runs the 4 demos at 120p and 240p on QEMU into $SP/armsim/NAME.
# ELF defaults to build/us_nsp_armsim/sm64.us.f3dex2 (make ARMSIM=1). Summarise with sum.py.
. "$(dirname "$0")/common.sh"
[ -n "$1" ] || { echo "usage: $0 NAME [ELF]"; exit 1; }
NAME=$1; ELF=${2:-$REPO/build/us_nsp_armsim/sm64.us.f3dex2}
case $ELF in /*) ;; *) ELF=$PWD/$ELF ;; esac
[ -f "$ELF" ] || { echo "no ARMSIM build at $ELF: run make ARMSIM=1 first"; exit 1; }
for r in 120 240; do for dm in 0 1 2 3; do (
  d=$SP/armsim/$NAME/d${dm}_$r; mkdir -p "$d"; cd "$d" || exit 1
  bench_config $dm $r false false > sm64_config.txt.tns
  timeout 300 qemu-system-arm -M versatilepb -cpu arm926 -m 256M -display none -monitor none -serial none -nodefaults -audio none \
    -icount shift=0,align=off,sleep=off -semihosting-config enable=on,target=native -kernel "$ELF" > run.log 2>&1 || echo "FAIL $NAME $dm $r" ) &
done; wait; done
echo "results in $SP/armsim/$NAME; summary: python3 tools/bench/sum.py $NAME"
