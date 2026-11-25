# Shared by the bench scripts (sourced, not run).
# REPO: the repository root. SP: where results go (default: bench-work/ in the repo, which git
# ignores). Results contain frames of the game, so never commit or upload them.
REPO=$(cd "$(dirname "$0")/../.." && pwd)
SP=${SP:-$REPO/bench-work}

# bench_config DEMO RES FOG DUMP: prints a benchmark sm64_config.txt.tns.
# DEMO 0-3 (attract demo), RES 120 or 240, FOG and DUMP true/false. Old setting names on
# purpose: the game upgrades them, and keeping them keeps old results comparable.
bench_config() {
  if [ "$2" = 120 ]; then lowres=true; else lowres=false; fi
  printf 'draw_sky true\ntexture_filtering false\nenable_fog %s\nenable_120p_mode %s\nframeskip 0\nbench_demo %s\nbench_warmup 30\nbench_ticks 600\nbench_dump_frames %s\n' \
    "$3" "$lowres" "$1" "$4"
}
