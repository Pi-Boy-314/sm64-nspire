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
| `bench_demo` | 0 | Which attract demo to play |
| `bench_warmup` | 30 | Ticks drawn but not measured (skips the fade-in) |
| `bench_ticks` | 600 | Max ticks measured; stops earlier if the demo ends |
| `bench_dump_frames` | false | Save ticks 60/240/420 as `sm64_bench_NNN.ppm.tns` images |
| `enable_120p_mode`, `draw_sky`, `enable_fog` | | Same as in the normal game |

`frameskip` is ignored; every tick is drawn.

## Reading the results

- **logic**: level script, objects, Mario, collision, camera.
- **scene**: `render_game()`, i.e. walking the scene graph, matrix math, building display lists.
- **gfx front**: interpreting display lists, transforming and lighting vertices.
- **gfx raster**: drawing triangles into the frame buffer.
- **present**: converting to RGB565, the 120p upscale, and `lcd_blit`.

Skipped frames still pay **logic + scene**; only **gfx** and **present** are skipped. The
"est. per drawn frame" lines combine these for frameskip 0 and 4, and the percentage is game
speed relative to the real 30 Hz.

**frame hash** is a checksum of every frame as shown on the LCD. If a change is supposed to
make things faster without changing the picture, the hash must stay the same. The run is
only repeatable from a clean boot with the same config, so compare hashes from runs with
identical settings.

Timings from the Firebird emulator don't match real hardware, so only use them to compare
builds against each other. Real numbers have to come from a real calculator.
