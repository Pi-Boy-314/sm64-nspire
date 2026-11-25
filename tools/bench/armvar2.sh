#!/bin/sh
# armvar.sh NAME ELF: runs the 4 demos on QEMU for each settings variant into $SP/armvar/NAME/VARIANT_dN
. "$(dirname "$0")/common.sh"
run() { # variant "settings lines"
  for dm in 0 1 2 3; do (
    d=$SP/armvar/$NAME/$1_d$dm; mkdir -p $d; cd $d
    printf '%b' "$2" > sm64_config.txt.tns
    printf 'frameskip 0\nenable_fog false\nbench_demo %s\nbench_warmup 30\nbench_ticks 600\nbench_dump_frames false\n' $dm >> sm64_config.txt.tns
    timeout 300 qemu-system-arm -M versatilepb -cpu arm926 -m 256M -display none -monitor none -serial none -nodefaults -audio none \
      -icount shift=0,align=off,sleep=off -semihosting-config enable=on,target=native -kernel $ELF > run.log 2>&1 || echo "FAIL $1 $dm" ) &
  done; wait
}
NAME=$1; ELF=$2
U='resolution 80x60\ndraw_sky false\ndraw_shadows false\nflat_colors true\n'
run up_nohud   "${U}sharp_hud false\n"
run up_sharp   "${U}sharp_hud true\n"
run bal_nohud  'resolution 160x120\ndraw_sky true\nsharp_hud false\n'
run bal_sharp  'resolution 160x120\ndraw_sky true\nsharp_hud true\n'
