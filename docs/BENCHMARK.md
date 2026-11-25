# Benchmark build

A self-running build for measuring performance changes. It boots straight into one of the
game's attract-mode demos (fixed input recordings), draws every tick, and reports where the
time goes. The demos play back exactly the same way every run, so results from two builds
can be compared directly.

## Build and run

```
make BENCH=1
```

The output is `build/us_nsp_bench/sm64.us.f3dex2.tns`; it builds in its own folder, so it
doesn't overwrite the normal build. Copy it to the calculator and run it. Don't press
anything until the results screen appears (ESC aborts). It takes about a minute or two on a
CX II.

The results are shown on screen and saved next to the game as `sm64_bench.txt.tns`.

## Options (in `sm64_config.txt.tns`)

| Option | Default | Meaning |
|---|---|---|
| `bench_demo` | 0 | Which attract demo to play, `0` to `3` |
| `bench_warmup` | 30 | Ticks drawn but not measured (skips the fade-in) |
| `bench_ticks` | 600 | Max ticks measured; stops earlier if the demo ends |
| `bench_dump_frames` | false | Save ticks 60/240/420 as `sm64_bench_NNN.ppm.tns` images |
| `resolution`, `draw_sky`, `enable_fog`, ... | | Same as in the normal game (see [SETTINGS.md](SETTINGS.md)) |

`frameskip` is ignored; every tick is drawn.

## Reading the results

- **logic**: level script, objects, Mario, collision, camera.
- **scene**: `render_game()`, i.e. walking the scene graph, matrix math, building display lists.
- **gfx front**: interpreting display lists, transforming and lighting vertices.
- **gfx raster**: drawing triangles into the frame buffer.
- **present**: converting to RGB565, the 120p upscale, and `lcd_blit`.

Skipped frames still pay **logic + scene**; only **gfx** and **present** are skipped. The
"est. per drawn frame" lines combine these for frameskip 0 and 4, and the percentage is game
speed relative to the real 30 ticks a second.

**frame hash** is a checksum of every frame as shown on the LCD. If a change is supposed to
make things faster without changing the picture, the hash must stay the same. The run is
only repeatable from a clean boot with the same config, so compare hashes from runs with
identical settings.

Timings from the Firebird emulator don't match real hardware, so only use them to compare
builds against each other. Real numbers have to come from a real calculator.

## Checking output on a Mac or PC (`make HOST=1`)

`make HOST=1` builds a headless native version of the same NSP code (renderer included) with
clang, using the stand-in Ndless headers in `tools/host_shim/`. Its output is
`build/us_nsp_host/sm64.us.f3dex2`. Run it from a folder that has a `sm64_config.txt.tns`; it
plays the demo and writes the same files as the calculator build, plus `sm64_bench_hashes.txt`
with one hash per frame, so you can find the first frame where two builds differ. Its timings
say nothing about the calculator; use it to check that a change draws the same picture.

## Counting instructions without a calculator (`make ARMSIM=1`)

`make ARMSIM=1` builds the benchmark with the same compiler and flags as the calculator build,
but for QEMU's `versatilepb` board, which has the same ARM926EJ-S core. Needs QEMU
(`brew install qemu` on macOS, `sudo apt-get install qemu-system-arm` on Linux). From a folder holding `sm64_config.txt.tns`, run
`tools/armsim/run.sh`; results go to `sm64_bench.txt.tns` as usual. QEMU's `-icount` ties the
timer to the instruction count, so every "ms" figure is millions of ARM instructions. Runs are
exactly repeatable.

For a per-function breakdown, run `tools/armsim/profile.sh` from the same folder (needs
QEMU's glib, which Homebrew installs with it, or `libglib2.0-dev` on Linux, and
`arm-none-eabi-objdump` from the Ndless toolchain on your PATH). It builds a small QEMU plugin, runs the demo for
100 and 400 ticks, and prints per-tick instructions and estimated ARM926 cycles for the hottest
functions; add `--annotate NAME` to see one function instruction by instruction.

Instruction counts are a good way to compare two builds, but they aren't time: they leave out
cache misses, slow memory and multi-cycle instructions such as 64-bit multiplies. Also, the LCD
blit costs nothing here.

To run all four demos and compare two builds in one go, use the scripts in
[tools/bench](../tools/bench/README.md).
