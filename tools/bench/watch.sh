#!/bin/sh
# watch.sh [DEMO...]: plays attract demos (default 0 1 2 3) from the host build in an ffplay window,
# at 30 fps and full resolution. Needs `make HOST=1` first, and ffplay (part of ffmpeg). Close the window to skip to the next demo.
. "$(dirname "$0")/common.sh"
BIN=$REPO/build/us_nsp_host/sm64.us.f3dex2
D=$(mktemp -d); cd "$D" || exit 1
for demo in ${@:-0 1 2 3}; do
  bench_config "$demo" 240 false false | sed "s/^bench_ticks .*/bench_ticks 100000/" > sm64_config.txt.tns
  mkfifo live; rm -f live.raw
  SM64_LIVE=live "$BIN" > run.log 2>&1 &
  ffplay -loglevel error -window_title "SM64 Nspire demo $demo" -x 960 -y 720 \
    -f rawvideo -pixel_format rgba -video_size 320x240 -framerate 30 -autoexit live
  wait; rm -f live
done
rm -rf "$D"
