# Instructions

How to build Super Mario 64 for the TI-Nspire CX, put it on your calculator, test changes, and fix common problems.

**Contents**

1. [What you need](#1-what-you-need)
2. [Set up your computer](#2-set-up-your-computer)
3. [Build the Ndless toolchain](#3-build-the-ndless-toolchain)
4. [Get the code and add your ROM](#4-get-the-code-and-add-your-rom)
5. [Build the game](#5-build-the-game)
6. [Put it on the calculator](#6-put-it-on-the-calculator)
7. [Play and change settings](#7-play-and-change-settings)
8. [Updating to a newer version](#8-updating-to-a-newer-version)
9. [Testing your changes](#testing-your-changes)
10. [Common problems and fixes](#common-problems-and-fixes)

> **Keep the game files private.** You build the game from your own copy of the ROM, so every built `.tns` file contains Nintendo's game data. Don't upload, share or commit `.tns` files, the ROM, or benchmark frames (`bench-work/`).

---

## 1. What you need

- **A TI-Nspire CX or CX II** (the CX II is faster) running an OS version that [Ndless](https://github.com/ndless-nspire/Ndless) supports. Check the Ndless page before updating your calculator's OS: a newer OS may not work with Ndless yet.
- **A Super Mario 64 ROM** in `.z64` format. Only the US version is tested with this fork.
- **A computer** running Linux, Windows with WSL, or macOS, with about 3 GB free for the toolchain and build.
- **A USB cable** and software for sending files to the calculator, such as the TI-Nspire CX Student Software, or [TiLP](https://github.com/debrouxl/tilp_and_gfm) on Linux.

The first build takes a while, mostly because the toolchain is compiled from source. Expect 20 to 60 minutes in all.

## 2. Set up your computer

### Linux or WSL (Windows)

On Windows, set up WSL with Ubuntu first:

1. Right-click PowerShell in the Start menu and choose **Run as administrator**.
2. Run `wsl --install`, then restart the computer.
3. Open **Ubuntu** from the Start menu and pick a username and password.

Use the Ubuntu window for every command below. When Linux asks for your password (for `sudo`), nothing appears as you type. That's normal: type it and press Enter.

Install the build tools:

```bash
sudo apt-get update
```

```bash
sudo apt-get install -y git gcc g++ build-essential pkg-config binutils wget python3 python3-dev libgmp-dev libmpfr-dev libmpc-dev zlib1g-dev libboost-all-dev texinfo php
```

### macOS

1. Install the Xcode command line tools, and wait for the installer window to finish:

```bash
xcode-select --install
```

2. Install [Homebrew](https://brew.sh). At the end it prints "Next steps" with commands to run. Run them, or `brew` won't be found.
3. Install the build tools:

```bash
brew install make gmp mpfr libmpc boost wget python pkg-config texinfo php
```

macOS comes with GNU make 3.81, which is too old for this project. Homebrew's `make` is version 4. This line makes the terminal use it (PATH is the list of folders the terminal looks in for programs). It saves the change so every new terminal has it too:

```bash
echo 'export PATH="$(brew --prefix make)/libexec/gnubin:$PATH"' >> ~/.zshrc && source ~/.zshrc
```

Check it worked: `make --version` should print `GNU Make 4.` or newer.

## 3. Build the Ndless toolchain

This builds the ARM compiler that makes programs for the calculator. It takes the longest of any step. These commands are adapted from the [Hackspire guide](https://hackspire.org/index.php/C_and_assembly_development_introduction).

```bash
cd ~
```

```bash
git clone --recursive https://github.com/ndless-nspire/Ndless.git
```

```bash
cd ~/Ndless/ndless-sdk/toolchain && ./build_toolchain.sh
```

Then add the toolchain to your PATH, saved for every new terminal. On Linux and WSL:

```bash
echo 'export PATH="$PATH:$HOME/Ndless/ndless-sdk/toolchain/install/bin:$HOME/Ndless/ndless-sdk/bin"' >> ~/.bashrc && source ~/.bashrc
```

On macOS, the same with `~/.zshrc`:

```bash
echo 'export PATH="$PATH:$HOME/Ndless/ndless-sdk/toolchain/install/bin:$HOME/Ndless/ndless-sdk/bin"' >> ~/.zshrc && source ~/.zshrc
```

Now build the Ndless SDK:

```bash
cd ~/Ndless && make
```

Check it worked: `nspire-gcc --version` should print a version number.

## 4. Get the code and add your ROM

```bash
cd ~
```

```bash
git clone https://github.com/Pi-Boy-314/sm64-nspire.git
```

If you're building from a different fork, use its link instead: copy it from the green **Code** button.

Open the new folder: `explorer.exe ~/sm64-nspire` on WSL, `open ~/sm64-nspire` on macOS. Copy your ROM into it and **rename it to `baserom.us.z64`**. The build looks for exactly that name.

The ROM has to be the US version in `.z64` format. Files ending in `.n64` or `.v64` store the data in a different order: convert them to `.z64` with an N64 ROM converter first. The build checks the ROM and stops with a clear message if it's the wrong one.

## 5. Build the game

```bash
cd ~/sm64-nspire && make -j4
```

`-j4` builds 4 files at a time, which is faster. The first build extracts the game's assets from the ROM, so it takes several minutes.

When it finishes, the game is at:

```
build/us_nsp/sm64.us.f3dex2.tns
```

Run `make clean` to delete the build and start over. Your ROM and the extracted assets are kept.

## 6. Put it on the calculator

1. **Install Ndless** on the calculator, following the instructions on the [Ndless page](https://github.com/ndless-nspire/Ndless) for your model and OS version.
2. **Make a folder** called `sm64` on the calculator, in My Documents.
3. **Send `sm64.us.f3dex2.tns`** to that folder with the TI-Nspire software (or TiLP). To find it, open `build/us_nsp`: `explorer.exe ~/sm64-nspire/build/us_nsp` on WSL, `open ~/sm64-nspire/build/us_nsp` on macOS. You can rename it, for example to `sm64.tns`, but keep the `.tns` ending.
4. **Unplug the USB cable.** The game runs slower while it's connected.
5. **Open the file** from My Documents to start the game.

If the calculator restarts, for example after a crash or a battery change, Ndless stops running. Open the Ndless installer document on the calculator again before opening the game.

## 7. Play and change settings

The settings screen appears every time the game starts. Press `Enter` to play. During play, press `tab` to open it again. Changes apply straight away.

- Keys `1` to `5` pick a preset, from 1 (fastest) to 5 (best-looking). **3 Balanced** is the default. Try **1 Ultra Performance** if the game feels too slow.
- `tab` switches between the Settings, Cheats and Controls pages.
- `0` twice resets settings, cheats and keys to the defaults. Nothing is saved until you press `Enter`.
- During play, `ctrl` shows a speed info screen (press any key to go back), and `esc` asks whether to quit (Enter: yes).

The game saves your progress to `sm64_save.bin.tns` and your settings to `sm64_config.txt.tns`, both in the same folder as the game. Every setting, preset, cheat and control is explained in [docs/SETTINGS.md](docs/SETTINGS.md).

## 8. Updating to a newer version

1. Get the new code: `cd ~/sm64-nspire && git pull`
2. Build again with `make -j4`. Only the changed files are rebuilt.
3. Send the new `.tns` to the calculator, replacing the old one.

Keep `sm64_save.bin.tns` and `sm64_config.txt.tns` on the calculator: your progress and settings carry over. Config files from older versions are upgraded automatically the first time the new version starts.

---

## Testing your changes

There are three ways to test, from simplest to most precise. You can do the last two without a calculator.

| Test | What it tells you | Needs |
|---|---|---|
| [Play-test on a calculator](#play-test-on-a-calculator) | Whether it works and feels right | A calculator |
| [Picture check](#picture-check-make-host1) | Whether a change alters any frame | Your computer |
| [Speed check](#speed-check-make-armsim1) | How much faster or slower a change is | Your computer and QEMU |

### Play-test on a calculator

Install the build as in [step 6](#6-put-it-on-the-calculator) and check:

- The game starts, and the title screen, file select and star select look right.
- Enter a level (Bob-omb Battlefield is a good one). Dialog boxes, the HUD and the pause menu are readable.
- Open the settings with `tab` during play, change the resolution and a preset, and go back. The game continues without a crash, and `ctrl` still opens the speed info screen.
- Try presets 1 and 5, and fog on and off.
- Quit with `esc`, then `Enter`, and open another app (a Calculator document, say). It should open normally. If it doesn't, memory is leaking.
- Turn on the FPS counter (Settings page) and note the frame rate at the same spot before and after your change.

For repeatable speed numbers on a calculator, build the benchmark with `make BENCH=1`. It plays one of the game's demos by itself and reports where the time goes. See [docs/BENCHMARK.md](docs/BENCHMARK.md).

### Picture check (`make HOST=1`)

This builds the calculator's code as a normal program for your computer, with no screen. It plays the game's four attract demos and records a checksum of every frame, plus a few frames as images. If your change is only meant to make things faster, every frame must stay identical.

macOS has the needed `clang` with the Xcode tools. On Linux, install it with `sudo apt-get install clang`.

**1. Record the "before" frames** on the code without your change, for example the latest commit with your edits stashed:

```bash
git stash
```

```bash
make HOST=1 -j4 && tools/bench/hostbench.sh before
```

```bash
git stash pop
```

**2. Record the "after" frames** with your change:

```bash
make HOST=1 -j4 && tools/bench/hostbench.sh after
```

**3. Compare:**

```bash
python3 tools/bench/agg.py bench-work/cmp before after
```

It prints something like `after vs before: frames identical 5950/5950; in 24 dumped frames: 0 px differ`. Any differing frames mean the picture changed. If that wasn't intended, look at the differing frames with `tools/bench/cmp.py` and `tools/bench/composite.py`.

Results go to `bench-work/` in the repo, which git ignores. Set `SP` to keep them somewhere else. To watch the demos in a window, install `ffmpeg` and run `tools/bench/watch.sh`.

### Speed check (`make ARMSIM=1`)

This builds the benchmark with the calculator's own compiler and runs it in [QEMU](https://www.qemu.org), on an emulated board with the same ARM926 processor. QEMU is set to count instructions, so results are exact and repeatable. Every "ms" in the results means a million instructions.

Install QEMU with `brew install qemu` (macOS) or `sudo apt-get install qemu-system-arm` (Linux).

As in the picture check, measure the "before" code first, then your change:

```bash
git stash
```

```bash
make ARMSIM=1 -j4 && tools/bench/armbench.sh before
```

```bash
git stash pop
```

```bash
make ARMSIM=1 -j4 && tools/bench/armbench.sh after
```

Then compare:

```bash
python3 tools/bench/sum.py before after
```

It prints millions of instructions per frame at 120p and 240p, averaged over the four demos. Lower is faster.

Instruction counts are good for comparing two builds, but they are not time on a calculator. Memory and cache delays aren't counted, so confirm big changes on real hardware.

For a per-function breakdown, run `tools/armsim/profile.sh` from a folder holding an `sm64_config.txt.tns`. [docs/BENCHMARK.md](docs/BENCHMARK.md) has the details.

### Memory check (AddressSanitizer)

To catch out-of-bounds reads and writes, build the computer version with AddressSanitizer:

```bash
make HOST=1 BUILD_DIR_BASE=build_asan PLATFORM_CFLAGS='-DTARGET_NSP -DHOST_BUILD -DBENCHMARK -DGIT_HASH=\"asan\" -I tools/host_shim -g -DNO_SEGMENTED_MEMORY -fsanitize=address -fsanitize-recover=address -fno-omit-frame-pointer' PLATFORM_LDFLAGS='-lm -fsanitize=address'
```

Then run `build_asan/us_nsp_host/sm64.us.f3dex2` from a folder with a benchmark config, for example one made by `hostbench.sh`. One harmless read past the end of the star texture is known.

### Before you share a change

- The picture check shows identical frames, unless the change is meant to look different.
- `make -j4` (calculator) and `make HOST=1` both build without errors.
- Someone has play-tested it on a real calculator.
- No `.tns` files, ROM or `bench-work/` frames are in your commit. Check with `git status`.

---

## Common problems and fixes

### Building

| Problem | Fix |
|---|---|
| `Failed to open baserom.us.z64!` or `Failed to extract assets` | The ROM is missing or misnamed. Put it in the `sm64-nspire` folder, named exactly `baserom.us.z64`. |
| `baserom.us.z64 has the wrong hash!` | The ROM isn't the US `.z64` version, or it's byte-swapped (`.n64` / `.v64`). Convert it to `.z64`, or use a US ROM. |
| `nspire-gcc: command not found` or `arm-none-eabi-gcc: No such file` | The toolchain isn't on your PATH. Run the `echo 'export PATH=...` line from [step 3](#3-build-the-ndless-toolchain) for your system, then open a new terminal. |
| On macOS: `brew: command not found` | Run the "Next steps" commands Homebrew printed when it finished installing, then open a new terminal. |
| On macOS: odd errors, missing files, or the build skips asset extraction | You're using macOS's make 3.81. Check `make --version`. Run the `gnubin` line from [step 2](#macos). |
| `./build_toolchain.sh` fails partway through | Usually a missing package. Check the list in [step 2](#2-set-up-your-computer), install what's missing, and run it again. |
| The build fails after `git pull` | Run `make clean`, then `make -j4`. |
| Undefined symbols ending in `SegmentRomStart` when setting `PLATFORM_CFLAGS` yourself | Add `-DNO_SEGMENTED_MEMORY` back to your flags. |

### On the calculator

| Problem | Fix |
|---|---|
| Opening the game gives a compatibility or "not supported" error | Ndless isn't running, which is normal after a restart. Open the Ndless installer document again. If that doesn't help, restart with the reset button on the back, then install Ndless again. |
| The calculator freezes | Press the reset button on the back, open the Ndless installer again, and start the game. If it happens at the same spot every time, note the level and the settings you used. |
| The game is very slow | Press `tab`, then `1` for Ultra Performance, or `2`. Unplug the USB cable. A CX II is faster than an original CX. |
| The game doesn't appear in My Documents, or won't open | The file must end in `.tns`. If you renamed it, add `.tns` back. |
| The game runs in slow motion | Frame skip is off. Set it to Auto on the settings screen. |
| Other apps won't open after quitting, or full resolution crashes after a few levels | Older builds leaked memory. Restart the calculator and install this version. |
| Text is hard to read at low resolution | Set Sharp HUD to "On + menus" on the settings screen. |
| Keys do the wrong thing, or you can't get out of a key change | On the settings screen, press `0` twice to reset everything. You can also delete `sm64_config.txt.tns` and the game makes a new one. |
| Edits to `sm64_config.txt.tns` are ignored | The file must keep its exact name and sit in the same folder as the game. |
| Your save is gone after updating | The save is `sm64_save.bin.tns` in the game's folder. Keep it there when replacing the game, and keep a copy on your computer. |

### Testing tools

| Problem | Fix |
|---|---|
| `no host build at ...` | Run `make HOST=1` first. |
| `no ARMSIM build at ...` | Run `make ARMSIM=1` first. |
| `qemu-system-arm: command not found` | Install QEMU (see [Speed check](#speed-check-make-armsim1)). |
| A script prints `FAIL ...` | Read `run.log` in that run's folder under `bench-work/`. |
| `agg.py` stops with `FileNotFoundError` | One of the two runs is missing or failed. Run `hostbench.sh` again for that name. |
| `profile.sh` can't build its plugin | It needs QEMU's development files (glib). Homebrew's QEMU includes them. On Linux, install `libglib2.0-dev`. It also needs the Ndless toolchain on your PATH. |
