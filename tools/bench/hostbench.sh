#!/bin/sh
# hostbench.sh NAME BIN: runs the 8 fog-off configs + d0 fog-on configs into $SP/cmp/NAME
SP=${SP:-$HOME/SM64-Nspire/bench-work}
for c in d0_120_fogfalse d0_240_fogfalse d1_120_fogfalse d1_240_fogfalse d2_120_fogfalse d2_240_fogfalse d3_120_fogfalse d3_240_fogfalse d0_120_fogtrue d0_240_fogtrue; do (
  d=$SP/cmp/$1/$c; rm -rf $d; mkdir -p $d; cd $d
  cp $SP/cmp/fog/$c/sm64_config.txt.tns .
  $2 > run.log 2>&1 < /dev/null || echo "FAIL $1 $c" ) &
done; wait
