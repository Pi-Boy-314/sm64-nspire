# Super Mario 64 for TI-Nspire CX

A port of Super Mario 64 to the TI-Nspire CX and CX II graphing calculators, built with [Ndless](https://github.com/ndless-nspire/Ndless). The entire game is playable from start to finish.

This is a fork of [rayzerbrain/sm64-nsp](https://github.com/rayzerbrain/sm64-nsp), the original port. It makes the game faster and easier to use: a quicker renderer, a settings screen you can open during play, speed presets, and fixes for memory leaks and crashes. See [What's new in this fork](#whats-new-in-this-fork).

![The game in an emulator, at full resolution](https://user-images.githubusercontent.com/82989402/235566001-9cb138d9-3ce4-4ab1-86ed-3aff23919145.png)
![The game completed with all 120 stars, on a real calculator](https://user-images.githubusercontent.com/82989402/235566406-f7c80234-1ef9-44b4-8bd6-a956d846431e.jpeg)

## Getting started

You build the game yourself from your own copy of the Super Mario 64 ROM. This repository contains no game data. Never share a built `.tns` file: it contains the game's data.

**[INSTRUCTIONS.md](INSTRUCTIONS.md)** covers each step:

1. Set up Linux, WSL on Windows, or macOS.
2. Build the Ndless toolchain.
3. Add your ROM and build the game.
4. Copy the game to the calculator and play.

It also explains how to test changes and how to fix common problems.

## Performance

The game is still slow, but much faster than the original port. The game logic always runs at full speed (30 ticks per second). By default, the calculator skips drawing the frames it can't keep up with.

- **On a real calculator,** the Ultra Performance preset runs the start of Bob-omb Battlefield at about 10.7 FPS. The Balanced preset (160×120, sky on) runs it at 5.8 FPS. Both were measured with Sharp HUD off, which Ultra Performance uses.
- **At the same settings,** the renderer needs about 2.8 times fewer instructions per frame than the original port at 160×120, and 3.9 times fewer at 320×240. These figures come from an emulator: see [Testing your changes](INSTRUCTIONS.md#testing-your-changes).

To trade picture quality for speed, pick a preset on the settings screen (keys `1` to `5`).

## Controls

| N64 | Calculator |
|---|---|
| Control Stick | Arrow keys or touchpad |
| A | `menu` |
| B | `del` |
| Z | `doc` |
| R | `shift` |
| Start | `enter` |
| C-Up, C-Down, C-Left, C-Right | `8`, `2`, `4`, `6` |
| Settings screen | `tab` |
| Quit | `esc`, then `enter` |
| Speed info | `ctrl` |

Every key can be changed on the Controls page of the settings screen. [docs/SETTINGS.md](docs/SETTINGS.md) lists every setting, preset and cheat.

## What's new in this fork

- **Faster renderer:** integer math instead of slow software floating point, fast paths for the most common surfaces, and no blending work on surfaces that don't need it.
- **Settings screen:** shown at startup and opened during play with `tab`. Changes take effect immediately, including resolution.
- **Presets** from Ultra Performance to Ultra Quality, and **automatic frame skip** that keeps the game at full speed.
- **New picture options:** 80×60 resolution, flat-color textures, a shadows toggle, and Sharp HUD, which keeps the HUD and menus readable at low resolution.
- **Remappable controls**, **cheats** (infinite health, moon jump and more), and an **FPS counter**.
- **Speed info screen** (`ctrl`): frame rate, where each frame's time goes, and the current settings.
- **Fixes:** memory leaks (other programs failed to load after quitting, and full resolution could run out of memory) and a crash when pressing `ctrl` after leaving the settings screen.
- **Builds on macOS** as well as Linux and WSL.
- **Testing tools** that check a change leaves the picture identical and measure its speed, without a calculator.

## Repository layout

Most folders come from the [Super Mario 64 decompilation](https://github.com/n64decomp/sm64) and contain the game itself. The calculator-specific parts are:

| Path | Contents |
|---|---|
| `src/pc/gfx/gfx_backend.c` | Software renderer: triangles, textures, fog |
| `src/pc/gfx/gfx_frontend.c` | Converts the game's N64 display lists into triangles |
| `src/pc/gfx/gfx_nsp.c` | Calculator screen, keyboard and main loop |
| `src/pc/settings_menu.c` | Settings, speed info and quit screens |
| `src/pc/configfile.c` | Reads and writes `sm64_config.txt.tns` |
| `src/pc/benchmark.c` | Self-running benchmark build |
| `tools/rom/` | Checks your ROM and extracts the game's data from it |
| `tools/host_shim/` | Stand-in Ndless headers for the computer test build |
| `tools/armsim/` | Runs the benchmark in QEMU and profiles it |
| `tools/bench/` | Scripts that compare frames and summarize speed |
| `docs/` | Settings reference, benchmark guide, decompilation change log |

## Credits

- [rayzerbrain](https://github.com/rayzerbrain): the original TI-Nspire port this fork builds on.
- [n64decomp/sm64](https://github.com/n64decomp/sm64): the Super Mario 64 decompilation.
- [fgsfdsfgs/sm64-port](https://github.com/fgsfdsfgs/sm64-port): the PC port the software renderer was adapted from.
- [Ndless](https://github.com/ndless-nspire/Ndless): compiling and running programs on the TI-Nspire.
- [Firebird](https://github.com/nspire-emus/firebird): the TI-Nspire emulator used to test the original port.
- [Pi-Boy-314](https://github.com/Pi-Boy-314): the idea for the original port, and this fork.
- Claude (Anthropic's AI assistant): co-author of this fork's speed-ups, settings screens, tools and documentation.
- The almighty God, from Whom all blessings flow.

## Legal

This project is not affiliated with or endorsed by Nintendo or Texas Instruments. It contains no game data: you need your own legally obtained copy of Super Mario 64.
