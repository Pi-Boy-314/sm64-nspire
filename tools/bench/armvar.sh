#!/bin/sh
# armvar.sh NAME ELF: runs the 4 demos on QEMU for each settings variant into $SP/armvar/NAME/VARIANT_dN
SP=${SP:-$HOME/SM64-Nspire/bench-work}
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
run a_120sky   'resolution 160x120\ndraw_sky true\ndraw_shadows true\nflat_colors false\n'
run b_120nosky 'resolution 160x120\ndraw_sky false\ndraw_shadows true\nflat_colors false\n'
run c_60nosky  'resolution 80x60\ndraw_sky false\ndraw_shadows true\nflat_colors false\n'
run d_60noshad 'resolution 80x60\ndraw_sky false\ndraw_shadows false\nflat_colors false\n'
run e_ultra    'resolution 80x60\ndraw_sky false\ndraw_shadows false\nflat_colors true\n'
run f_120flat  'resolution 160x120\ndraw_sky false\ndraw_shadows false\nflat_colors true\n'
