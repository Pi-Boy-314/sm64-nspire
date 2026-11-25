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

The screen has three pages: **Settings**, **Cheats** and **Controls**. The Settings page has two groups: **Speed** (frame skip and the FPS counter) and **Picture**.

| Key | Does |
|---|---|
| tab | next page |
| Up / Down | choose a row |
| Left / Right | change it (on the Controls page: set a new key) |
| 1 to 5 | apply a preset, from 1 (fastest) to 5 (best-looking) |
| 0, then 0 again | reset settings, cheats and keys to their defaults |
| Enter | save and play (or go back to the game) |
| Esc | at the start: quit. During play: undo your changes and go back. |

The screen remembers the page and row you were on while the game is running.

**On a computer:** copy `sm64_config.txt.tns` from the calculator with the TI-Nspire software (or TI-Nspire Computer Link) and open it in any plain text editor, such as Notepad or TextEdit. The file explains every setting. Save it and send it back to the same folder as the game, keeping the name. The game only rewrites the file when you save from the settings screen, so your edits and notes are kept.

**Speed**

| Setting (screen / file) | What it does |
|---|---|
| Frame skip / `frameskip` | The game runs 30 times a second, but the calculator usually can't draw that often. Skipping the drawing of some frames keeps the game at its real speed. **Auto** (`auto`, the default in every preset) skips only as many frames as needed for full speed, at most 10 in a row. **Off** (`0`) draws every frame, but the game runs in slow motion when the calculator can't keep up. 1 to 30 skips at most that many frames in a row. |
| FPS counter / `show_fps` | Shows frames drawn per second and game speed (100% is full speed, lower is slow motion) in the bottom-left corner. Not part of the presets. |

**Picture**

| Setting (screen / file) | What it does |
|---|---|
| 3D resolution / `resolution` | Resolution of the 3D world, stretched to fill the screen. `320x240` is sharp but much slower. `160x120` is much faster (roughly 2 to 3 times) but blocky. `80x60` is the fastest and very blocky. The HUD and text follow Sharp text. |
| Textures / `textures` | **Normal** (`normal`): as in the original. **Smooth** (`smooth`): filtered like on the N64, about 3 times slower at 320x240. **Flat** (`flat`): walls, floors and other solid surfaces in one plain colour each (their texture's average colour). Much faster; looks like a very early 3D game. Trees, fences, coins, text and the sky keep their textures. |
| Sky / `draw_sky` | Off shows a plain black sky. Faster, since the sky covers much of the screen. |
| Fog / `enable_fog` | Distance fog in the levels that use it (Bob-omb Battlefield, for example). Closer to the original look, but fogged areas draw more slowly. |
| Shadows / `draw_shadows` | Shadows under Mario, enemies and items. Off is a little faster, but jumps are harder to judge. |
| Sharp text / `sharp_text` | Keeps text readable when the 3D world is below 320x240. **HUD + menus** (`hud_and_menus`, the default): the HUD (lives, coins, stars), dialog boxes and the pause menu are drawn at the full 320x240, and so are whole menu screens without Mario (title, file select, star select), which also use normal textures; menus are simple, so they stay smooth. **HUD** (`hud`): only the HUD, dialog boxes and pause menu. **Off** (`off`): everything at the 3D world's resolution, a little faster. Sharp HUD costs about 6% at 160x120 and 13% at 80x60 in the demos, less in normal play. No effect at 320x240. |

### Presets

Fastest first; keys 1 to 5 on the settings screen.

| | 1 Ultra Performance | 2 Performance | 3 Balanced (default) | 4 Quality | 5 Ultra Quality |
|---|---|---|---|---|---|
| Frame skip | Auto | Auto | Auto | Auto | Auto |
| 3D resolution | 80x60 | 160x120 | 160x120 | 320x240 | 320x240 |
| Textures | Flat | Normal | Normal | Normal | Smooth |
| Sky | Off | Off | On | On | On |
| Fog | Off | Off | Off | On | On |
| Shadows | Off | On | On | On | On |
| Sharp text | HUD + menus | HUD + menus | HUD + menus | HUD + menus | HUD + menus |

Ultra Performance goes for the most speed whatever it looks like: on a real calculator it took the start of Bob-omb Battlefield from 5.8 to 10.7 fps (compared with 160x120 with the sky on, before Sharp text existed; set it to Off for a little more). Ultra Quality is Quality with smooth textures: the best picture, but very slow. Any mix of settings works too.

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

Older config files are upgraded automatically on first start, keeping their values. `enable_120p_mode true` becomes `resolution 160x120` (`false` becomes `320x240`), `flat_colors true` becomes `textures flat`, `smooth_textures true` becomes `textures smooth`, and `sharp_hud true` / `false` becomes `sharp_text hud_and_menus` / `off`. New settings are added with their defaults. The old `texture_filtering` line did nothing and is dropped, as are old numeric `key_*` lines.


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
