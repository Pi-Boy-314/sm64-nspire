#!/bin/sh
# armbench.sh NAME ELF: runs 4 demos x 120p/240p on QEMU into $SP/armsim/NAME
SP=${SP:-$HOME/SM64-Nspire/bench-work}
for r in 120 240; do for dm in 0 1 2 3; do (
  d=$SP/armsim/$1/d${dm}_$r; mkdir -p $d; cd $d
  cp $SP/armsim/base/d${dm}_$r/sm64_config.txt.tns .
  timeout 300 qemu-system-arm -M versatilepb -cpu arm926 -m 256M -display none -monitor none -serial none -nodefaults -audio none \
    -icount shift=0,align=off,sleep=off -semihosting-config enable=on,target=native -kernel $2 > run.log 2>&1 || echo "FAIL $1 $dm $r" ) &
done; wait; done
