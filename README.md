# Super Mario 64 Nspire Port

This is a port of SM64 to the TI-Nspire CX lineup. 

It is fully playable, albeit painfully slow, especially on a CX I. Play at your own risk of dying of old age.

![Emulated, at full resolution](https://user-images.githubusercontent.com/82989402/235566001-9cb138d9-3ce4-4ab1-86ed-3aff23919145.png)
![Game beaten with 100% completion, on actual hardware.](https://user-images.githubusercontent.com/82989402/235566406-f7c80234-1ef9-44b4-8bd6-a956d846431e.jpeg)

## Installation Instructions

See [Installing](./INSTALLING.md)

## Settings

Settings are stored in `sm64_config.txt.tns`, next to the game. The game creates it the first time it runs.

**On the calculator:** the settings screen appears when the game starts. During play, press `tab` (the settings key, which you can change) to open it again. Changes made during play apply straight away, including resolution.

The screen has three pages: **Settings**, **Cheats** and **Controls**.

| Key | Does |
|---|---|
| tab | next page |
| Up / Down | choose a row |
| Left / Right | change it (on the Controls page: set a new key) |
| 1, 2, 3 | apply the Performance, Balanced or Quality preset |
| 0, then 0 again | reset settings, cheats and keys to their defaults |
| Enter | save and play (or go back to the game) |
| Esc | at the start: quit. During play: undo your changes and go back. |

The screen remembers the page and row you were on while the game is running.

**On a computer:** copy `sm64_config.txt.tns` from the calculator with the TI-Nspire software (or TI-Nspire Computer Link) and open it in any plain text editor, such as Notepad or TextEdit. The file explains every setting. Save it and send it back to the same folder as the game, keeping the name. The game only rewrites the file when you save from the settings screen, so your edits and notes are kept.

| Setting | What it does |
|---|---|
| `enable_120p_mode` | `true` draws at 160x120 and stretches it to the screen. This is much faster, but blocky, and small text is hard to read. `false` is the full 320x240. |
| `frameskip` (`auto` or 0 to 30) | The most frames in a row the game may skip drawing to keep its real speed. `0` draws every frame, but the game runs in slow motion when the calculator can't keep up. Higher values keep it nearer real speed but look jumpier. `auto` skips as many as needed for full speed (up to 10). |
| `draw_sky` | `false` shows a black sky. This is faster, since the sky covers much of the screen. |
| `enable_fog` | Distance fog in the levels that use it. It's closer to the original look, but fogged areas draw more slowly. |
| `show_fps` | Shows a counter in the bottom-left corner: frames drawn per second, and game speed (100% is full speed, lower is slow motion). Handy for choosing a frameskip. Not part of the presets. |

### Presets

| | Performance | Balanced (default) | Quality |
|---|---|---|---|
| `enable_120p_mode` | `true` | `true` | `false` |
| `frameskip` | `auto` | 4 | 2 |
| `draw_sky` | `false` | `true` | `true` |
| `enable_fog` | `false` | `false` | `true` |

Performance gives the most speed and Quality the best picture. Any mix of values works too.

### Cheats

All cheats are off by default. They don't modify your save file, but stars and coins you collect while using them are saved as usual. Turn them on in the Cheats page, or set them to `true` in the file.

| Setting | What it does |
|---|---|
| `cheat_infinite_health` | Mario's health refills every frame. Enemies, fire, falls and lack of air can't hurt him. Bottomless pits still cost a life. |
| `cheat_infinite_lives` | Lives stay at 99, so you never get a game over. |
| `cheat_moon_jump` | Hold the A button in the air and Mario keeps rising. Let go to fall. |
| `cheat_super_speed` | Mario covers twice the ground when walking, running or sliding. |
| `cheat_keep_caps` | Wing, metal and vanish caps don't run out until you leave the level or lose the cap. |
| `cheat_skip_intro` | A new save file starts right outside the castle, without Peach's letter and Lakitu's flight. |
| `cheat_exit_course_anytime` | The pause menu always offers "Exit course", even in mid-air or swimming. |

Older config files are upgraded automatically on first start, keeping their values. The old `texture_filtering` line did nothing and is dropped, as are old numeric `key_*` lines.


## Controls

Every button can be moved to another key on the Controls page of the settings screen, or with the `key_*` lines in the config file, which list the key names. Esc (quit) and ctrl (speed statistics) are fixed.

| N64 | Default key |
|---|---|
| Stick | arrows / touchpad (8 directions) |
| A | `menu` |
| B | `del` |
| Z | `doc` |
| R | `shift` |
| Start | `enter` |
| C up, down, left, right | `8`, `2`, `4`, `6` |
| Settings screen | `tab` |

Escape exits the game, and pressing Control brings up a profiling screen with detailed timings. For a constant FPS counter, turn on `show_fps`.


## Credits
 - https://github.com/n64decomp/sm64: Original decompilation
 - https://github.com/fgsfdsfgs/sm64-port: Original fork; Software renderer adapted from here
 - https://github.com/ndless-nspire/Ndless: Nspire compile and runtime capabilities
 - https://github.com/nspire-emus/firebird: Emulator used extensively for testing and debugging purposes
 - https://github.com/Pi-Boy-314: Project idea
 - The almighty God from Whom all blessings flow


These people did all the hard work; I mostly just put the pieces together. Go check out their stuff too.
