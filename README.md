# Super Mario 64 Nspire Port

This is a port of SM64 to the TI-Nspire CX lineup. 

It is fully playable, albeit painfully slow, especially on a CX I. Play at your own risk of dying of old age.

![Emulated, at full resolution](https://user-images.githubusercontent.com/82989402/235566001-9cb138d9-3ce4-4ab1-86ed-3aff23919145.png)
![Game beaten with 100% completion, on actual hardware.](https://user-images.githubusercontent.com/82989402/235566406-f7c80234-1ef9-44b4-8bd6-a956d846431e.jpeg)

## Installation Instructions

See [Installing](./INSTALLING.md)

## Settings

Settings are stored in `sm64_config.txt.tns`, next to the game. The game creates it the first time it runs.

**On the calculator:** the first screen after starting the game shows the controls and the settings.

| Key | Does |
|---|---|
| Up / Down | choose a setting |
| Left / Right | change it |
| 1, 2, 3 | apply the Performance, Balanced or Quality preset |
| Enter | save and play |
| Esc | quit |

**On a computer:** copy `sm64_config.txt.tns` from the calculator with the TI-Nspire software (or TI-Nspire Computer Link) and open it in any plain text editor, such as Notepad or TextEdit. The file explains every setting. Save it and send it back to the same folder as the game, keeping the name. The game only rewrites the file when you save from the settings screen, so your edits and notes are kept.

| Setting | What it does |
|---|---|
| `enable_120p_mode` | `true` draws at 160x120 and stretches it to the screen. This is much faster, but blocky, and small text is hard to read. `false` is the full 320x240. |
| `frameskip` (0 to 30) | The most frames in a row the game may skip drawing to keep its real speed. `0` draws every frame, but the game runs in slow motion when the calculator can't keep up. Higher values keep it nearer real speed but look jumpier. |
| `draw_sky` | `false` shows a black sky. This is faster, since the sky covers much of the screen. |
| `enable_fog` | Distance fog in the levels that use it. It's closer to the original look, but fogged areas draw more slowly. |

### Presets

| | Performance | Balanced (default) | Quality |
|---|---|---|---|
| `enable_120p_mode` | `true` | `true` | `false` |
| `frameskip` | 6 | 4 | 2 |
| `draw_sky` | `false` | `true` | `true` |
| `enable_fog` | `false` | `false` | `true` |

Performance gives the most speed and Quality the best picture. Any mix of values works too.

### Cheats

All cheats are off by default. They don't modify your save file, but stars and coins you collect while using them are saved as usual. Turn them on in the settings screen, or set them to `true` in the file.

| Setting | What it does |
|---|---|
| `cheat_infinite_health` | Mario's health refills every frame. Enemies, fire, falls and lack of air can't hurt him. Bottomless pits still cost a life. |
| `cheat_infinite_lives` | Lives stay at 99, so you never get a game over. |
| `cheat_moon_jump` | Hold A (the `menu` key) in the air and Mario keeps rising. Let go to fall. |

Older config files are upgraded automatically on first start, keeping their values. The old `key_*` and `texture_filtering` lines did nothing and are dropped.


## Controls

Controls cannot be edited via the config file, if you need to change the controls yourself you'll have to adjust the [according file](./src/pc/controller_entry_point.c)

The controls are shown on the start screen, they are:

Start: enter

A Button: menu


B Button: del

Z Trigger: doc

Analog Controls: Touchpad

C Buttons (up, down left, right): 8, 2, 4, 6, accordingly

Escape exits the game, and pressing Control brings up a profiling screen, which shows your current FPS.


## Credits
 - https://github.com/n64decomp/sm64: Original decompilation
 - https://github.com/fgsfdsfgs/sm64-port: Original fork; Software renderer adapted from here
 - https://github.com/ndless-nspire/Ndless: Nspire compile and runtime capabilities
 - https://github.com/nspire-emus/firebird: Emulator used extensively for testing and debugging purposes
 - https://github.com/Pi-Boy-314: Project idea
 - The almighty God from Whom all blessings flow


These people did all the hard work; I mostly just put the pieces together. Go check out their stuff too.
