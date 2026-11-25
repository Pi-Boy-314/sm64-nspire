# Benchmark helpers

Scripts for checking that renderer changes keep the picture identical and for measuring speed.
Reference configs and frames live outside the repo in `$SP` (default `~/SM64-Nspire/bench-work`),
because the dumped frames contain game imagery. Never commit them.

| Script | Use |
|---|---|
| `hostbench.sh NAME BIN` | Runs the host benchmark (`make HOST=1`) on 10 configs (4 demos x 120p/240p fog off, demo 0 fog on), into `$SP/cmp/NAME`. Configs come from `$SP/cmp/fog`. |
| `agg.py $SP/cmp REF NAME...` | Compares runs against a reference run: identical frame hashes and differing pixels. `qol2` is the 64d9bbb baseline. |
| `cmp.py DIR` | Pixel and hash comparison inside one result folder. |
| `composite.py` | Puts PPM frames side by side for looking at differences. |
| `armbench.sh NAME ELF` | Runs the 4 demos at 120p and 240p on QEMU (`make ARMSIM=1`) with exact instruction counts, into `$SP/armsim/NAME`. Configs from `$SP/armsim/base`. |
| `sum.py NAME...` | Million instructions per frame for `armbench.sh` runs. |
| `armvar.sh` / `armvar2.sh NAME ELF` | Same, for named settings variants (Ultra Performance pieces, sharp HUD on and off). |
| `lines.py ANN ELF [N]` | Top source lines from a `tools/armsim/profile.sh` annotation. |

Run `source ~/SM64-Nspire/env.sh` before building.
