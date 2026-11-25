#!/bin/sh
# hostbench.sh NAME [BIN]: runs the host build (make HOST=1) on 10 configs into $SP/cmp/NAME:
# the 4 demos at 120p and 240p with fog off, and demo 0 at both with fog on.
# BIN defaults to build/us_nsp_host/sm64.us.f3dex2.
. "$(dirname "$0")/common.sh"
[ -n "$1" ] || { echo "usage: $0 NAME [BIN]"; exit 1; }
BIN=${2:-$REPO/build/us_nsp_host/sm64.us.f3dex2}
case $BIN in /*) ;; *) BIN=$PWD/$BIN ;; esac
[ -x "$BIN" ] || { echo "no host build at $BIN: run make HOST=1 first"; exit 1; }
NAME=$1
for c in 0_120_false 0_240_false 1_120_false 1_240_false 2_120_false 2_240_false 3_120_false 3_240_false 0_120_true 0_240_true; do (
  IFS=_; set -- $c; d=$SP/cmp/$NAME/d$1_$2_fog$3
  rm -rf "$d"; mkdir -p "$d"; cd "$d" || exit 1
  bench_config "$1" "$2" "$3" true > sm64_config.txt.tns
  "$BIN" > run.log 2>&1 < /dev/null || echo "FAIL $NAME d$1_$2_fog$3" ) &
done; wait
echo "results in $SP/cmp/$NAME"
