# Super Mario 64 for the TI-Nspire CX

Super Mario 64 running on a TI-Nspire CX or CX II graphing calculator, using [Ndless](https://github.com/ndless-nspire/Ndless). The whole game is playable from start to finish.

This is a fork of [rayzerbrain/sm64-nsp](https://github.com/rayzerbrain/sm64-nsp), the original port. The fork makes it faster and easier to use: a quicker renderer, a settings screen you can open while playing, speed presets, and fixes for memory leaks and crashes. See [What's new in this fork](#whats-new-in-this-fork).

![Emulated, at full resolution](https://user-images.githubusercontent.com/82989402/235566001-9cb138d9-3ce4-4ab1-86ed-3aff23919145.png)
![Game beaten with 100% completion, on actual hardware.](https://user-images.githubusercontent.com/82989402/235566406-f7c80234-1ef9-44b4-8bd6-a956d846431e.jpeg)

## Getting started

You build the game yourself from your own copy of the Super Mario 64 ROM. No game files are included here, and you should never share a built `.tns` file, since it contains the game's data.

**[INSTRUCTIONS.md](INSTRUCTIONS.md)** walks through it step by step:

1. Set up Linux, WSL on Windows, or macOS.
2. Build the Ndless toolchain.
3. Add your ROM and build the game.
4. Copy it to the calculator and play.

It also covers testing changes and fixes for common problems.

## How fast is it?

Still slow, but much less so than before. The game logic always runs at its real speed: by default the calculator skips drawing frames it can't keep up with.

- **On a real calculator**, the Ultra Performance preset runs the start of Bob-omb Battlefield at about 10.7 fps, against 5.8 fps at 160x120 with the sky on (the Balanced preset, measured before Sharp HUD was added).
- **At the same settings**, the renderer needs about 2.8 times fewer instructions per frame at 160x120, and 3.9 times fewer at 320x240, than the original port. These figures were measured in an emulator: see [Testing](INSTRUCTIONS.md#testing-your-changes).

Pick a preset on the settings screen (keys 1 to 5) to trade picture quality for speed.

## Controls

| N64 | Calculator key |
|---|---|
| Stick | arrows / touchpad |
| A | `menu` |
| B | `del` |
| Z | `doc` |
| R | `shift` |
| Start | `enter` |
| C up, down, left, right | `8`, `2`, `4`, `6` |
| Settings screen | `tab` |
| Quit | `esc`, then `enter` |
| Speed info | `ctrl` |

Every key can be changed on the Controls page of the settings screen. See [docs/SETTINGS.md](docs/SETTINGS.md) for every setting, preset and cheat.

## What's new in this fork

- **Faster renderer:** integer maths instead of slow software floating point, fast paths for the most common surfaces, and no extra work on surfaces that don't need blending.
- **Settings screen:** shown at start and opened during play with `tab`. Changes apply straight away, including resolution.
- **Presets** from Ultra Performance to Ultra Quality, and **automatic frame skip** that keeps the game at full speed.
- **New picture options:** an 80x60 resolution, flat-colour textures, a shadows toggle, and Sharp HUD, which keeps the HUD and menus readable at low resolution.
- **Remappable controls**, **cheats** (infinite health, moon jump and more), and an **FPS counter**.
- **Fixes:** memory leaks (other apps failed to load after quitting, and full resolution could run out of memory) and a crash when pressing `ctrl` after leaving the settings screen.
- **Builds on macOS** as well as Linux and WSL.
- **Testing tools** that check a change keeps the picture identical and measure its speed, without a calculator.

## Repository layout

Most folders come from the [SM64 decompilation](https://github.com/n64decomp/sm64) and hold the game itself. The calculator-specific parts are:

| Path | What's there |
|---|---|
| `src/pc/gfx/gfx_backend.c` | Software renderer: triangles, textures, fog |
| `src/pc/gfx/gfx_frontend.c` | Turns the game's N64 display lists into triangles |
| `src/pc/gfx/gfx_nsp.c` | Calculator screen, keyboard and main loop |
| `src/pc/settings_menu.c` | The settings screen |
| `src/pc/configfile.c` | Reading and writing `sm64_config.txt.tns` |
| `src/pc/benchmark.c` | The self-running benchmark build |
| `tools/host_shim/` | Stand-in Ndless headers for the computer test build |
| `tools/armsim/` | Runs the benchmark in QEMU and profiles it |
| `tools/bench/` | Scripts that compare frames and summarise speed |
| `docs/` | Settings reference, benchmark guide, decomp change log |

## Credits

- [rayzerbrain](https://github.com/rayzerbrain): the original Nspire port this fork builds on.
- [n64decomp/sm64](https://github.com/n64decomp/sm64): the original decompilation.
- [fgsfdsfgs/sm64-port](https://github.com/fgsfdsfgs/sm64-port): the PC port the software renderer was adapted from.
- [Ndless](https://github.com/ndless-nspire/Ndless): compiling and running programs on the Nspire.
- [Firebird](https://github.com/nspire-emus/firebird): the Nspire emulator used to test the original port.
- [Pi-Boy-314](https://github.com/Pi-Boy-314): the idea for the original port.

The original port's README also thanks "the almighty God from Whom all blessings flow".
