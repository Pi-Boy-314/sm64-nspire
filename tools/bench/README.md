# Benchmark helpers

Scripts for checking that a change keeps the picture identical, and for measuring its speed. For a walkthrough, see [Testing your changes](../../INSTRUCTIONS.md#testing-your-changes).

The scripts make their own benchmark configs and write results to `bench-work/` in the repo, which git ignores. Set `SP` to use another folder (then use `$SP/cmp` with `agg.py`). Results contain frames of the game, so never commit or upload them.

Run them from the repo root, for example `python3 tools/bench/sum.py before after`.

| Script | Use |
|---|---|
| `hostbench.sh NAME [BIN]` | Runs the computer build (`make HOST=1`) on 10 configs, into `$SP/cmp/NAME`: the 4 demos at 120p and 240p with fog off, and demo 0 at both with fog on. |
| `python3 agg.py bench-work/cmp REF NAME...` | Compares runs with a reference run: how many frames are identical, and how many pixels differ in the saved frames. |
| `python3 cmp.py DIR` | Hash and pixel comparison within one result folder. |
| `python3 composite.py A.ppm B.ppm OUT.ppm SCALE` | Puts two saved frames side by side, to see a difference. |
| `armbench.sh NAME [ELF]` | Runs the 4 demos at 120p and 240p in QEMU (`make ARMSIM=1`) with exact instruction counts, into `$SP/armsim/NAME`. |
| `python3 sum.py NAME...` | Millions of instructions per frame for `armbench.sh` runs. |
| `armvar.sh` / `armvar2.sh NAME ELF` | Runs the 4 demos in QEMU for named settings variants (pieces of Ultra Performance, Sharp HUD on and off), into `$SP/armvar/NAME`. |
| `python3 lines.py ANN ELF [N]` | Top source lines, from a file holding the output of `tools/armsim/profile.sh --annotate`. Needs the Ndless toolchain on PATH. |
| `watch.sh [DEMO...]` | Plays the demos from the computer build in a window. Needs ffmpeg's `ffplay`. |
| `common.sh` | Shared settings and the config writer used by the scripts above. |
