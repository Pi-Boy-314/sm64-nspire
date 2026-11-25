# Settings and controls

Settings are stored in `sm64_config.txt.tns`, next to the game. The game creates it the first time it runs.

## Changing settings

**On the calculator:** the settings screen appears when the game starts. During play, press `tab` (the settings key, which you can change) to open it again. Changes made during play apply straight away, including resolution.

The screen has three pages: **Settings**, **Cheats** and **Controls**. The Settings page lists the presets at the top, with the active one highlighted, then two groups: **Speed** (frame skip and the FPS counter) and **Picture**. The box below the list explains the selected row. On the Controls page it also says what the button does in the game.

| Key | Does |
|---|---|
| tab | next page |
| Up / Down | choose a row |
| Left / Right | change it (on the Controls page: then press the new key; Esc cancels) |
| 1 to 5 | apply a preset, from 1 (fastest) to 5 (best-looking) |
| 0, then 0 again | reset settings, cheats and keys to their defaults (saved only when you press Enter) |
| Enter | save and play (or go back to the game) |
| Esc | at the start: quit. During play: undo your changes and go back. |

The screen remembers the page and row you were on while the game is running.

**On a computer:** copy `sm64_config.txt.tns` from the calculator with the TI-Nspire software (or TI-Nspire Computer Link) and open it in any plain text editor, such as Notepad or TextEdit. The file explains every setting. Save it and send it back to the same folder as the game, keeping the name. The game rewrites the whole file when you save changes on the settings screen. Your values are kept, but notes you added are not.

## Speed

| Setting (screen / file) | What it does |
|---|---|
| Frame skip / `frameskip` | The game runs 30 times a second, but the calculator usually can't draw that often. Skipping the drawing of some frames keeps the game at its real speed. **Auto (full speed)** (`auto`, the default in every preset) skips only as many frames as needed for full speed, at most 10 in a row. **Off (slow motion)** (`0`) draws every frame, but the game runs in slow motion when the calculator can't keep up. **At most 1** to **At most 30** (`1` to `30`) skip up to that many frames in a row. Below 10, the picture jumps less than with Auto, but the game slows down when the calculator falls further behind. |
| FPS counter / `show_fps` | Shows frames drawn per second and game speed (100% is full speed, lower is slow motion) in the bottom-left corner. Presets don't change it. |

## Picture

| Setting (screen / file) | What it does |
|---|---|
| 3D resolution / `resolution` | Resolution of the 3D world. Lower ones are stretched to fill the screen. `320x240` is sharp but much slower. `160x120` is much faster (roughly 2 to 3 times) but blocky. `80x60` is the fastest and very blocky. The HUD (lives, coins, stars) follows Sharp HUD. |
| Textures / `textures` | **Flat (fastest)** (`flat`): walls, floors and other solid surfaces in one plain color each (their texture's average color). Much faster; looks like a very early 3D game. Trees, fences, coins, text and the sky keep their textures. **Normal** (`normal`): as in the original, with sharp square pixels. **Smooth (slow)** (`smooth`): blurred slightly, like on the N64, about 3 times slower at 320x240. |
| Sky / `draw_sky` | Off shows a plain black sky. Faster, since the sky covers much of the screen. |
| Fog / `enable_fog` | Distance fog in the levels that use it (Bob-omb Battlefield, for example). Closer to the original look, but fogged areas draw more slowly. |
| Shadows / `draw_shadows` | Shadows under Mario, enemies and items. Off is a little faster, but jumps are harder to judge. |
| Sharp HUD / `sharp_hud` | Keeps the HUD readable when the 3D world is below 320x240. **On + menus** (`on_and_menus`, the default): the HUD (lives, coins, stars, power meter), dialog boxes and the pause menu are drawn at the full 320x240, and so are the menu screens (title, file select, star select). Menu screens never use flat textures; they are simple, so they still run well. **On** (`on`): only the HUD, dialog boxes and pause menu. **Off** (`off`): everything at the 3D world's resolution, a little faster. Sharp HUD costs about 6% of speed at 160x120 and 13% at 80x60 in the demos, less in normal play. At 320x240 it only keeps menu screens free of flat textures. |

## Presets

Fastest first; keys 1 to 5 on the settings screen.

| | 1 Ultra Performance | 2 Performance | 3 Balanced (default) | 4 Quality | 5 Ultra Quality |
|---|---|---|---|---|---|
| Frame skip | Auto | Auto | Auto | Auto | Auto |
| 3D resolution | 80x60 | 160x120 | 160x120 | 320x240 | 320x240 |
| Textures | Flat | Normal | Normal | Normal | Smooth |
| Sky | Off | Off | On | On | On |
| Fog | Off | Off | Off | On | On |
| Shadows | Off | On | On | On | On |
| Sharp HUD | Off | On + menus | On + menus | On + menus | On + menus |

Ultra Performance goes for the most speed whatever it looks like: on a real calculator it runs the start of Bob-omb Battlefield at 10.7 fps, compared with 5.8 fps at 160x120 with the sky on. It turns Sharp HUD off, which saves about 13% at 80x60 but makes the HUD numbers and the file select screen blocky. Turn Sharp HUD on if you'd rather read them. Ultra Quality is Quality with smooth textures: the best picture, but very slow. Any mix of settings works too.

## Cheats

All cheats are off by default. They don't modify your save file, but stars and coins you collect while using them are saved as usual. Turn them on in the Cheats page, or set them to `true` in the file.

| Cheat (screen / file) | What it does |
|---|---|
| Infinite health / `cheat_infinite_health` | Mario's health refills every frame. Enemies, fire, falls and lack of air can't hurt him. Bottomless pits still cost a life. |
| Infinite lives / `cheat_infinite_lives` | Lives stay at 99, so you never get a game over. |
| Moon jump / `cheat_moon_jump` | Hold the A button (`menu` by default) in the air and Mario keeps rising. Let go to fall. |
| Super speed / `cheat_super_speed` | Mario covers twice the ground when walking, running or sliding. |
| Keep caps / `cheat_keep_caps` | Wing, metal and vanish caps don't run out until you leave the level or lose the cap. |
| Skip intro / `cheat_skip_intro` | A new save file starts right outside the castle, without Peach's letter and Lakitu's flight. |
| Exit course anytime / `cheat_exit_course_anytime` | The pause menu always offers "Exit course", even in mid-air or swimming. |

## Controls

Every button can be moved to another key on the Controls page of the settings screen, or with the `key_*` lines in the config file, which list the key names. On the Controls page, a key already in use swaps with the old one. In the file, two buttons can share a key, but `key_settings` needs its own (if not, it goes back to `tab`). `esc` (quit) and `ctrl` (speed info) can't be moved.

| Button (screen / file) | Default key |
|---|---|
| Stick up, down, left, right / `key_stick_up` ... | arrows / touchpad (diagonals too) |
| A button / `key_a` | `menu` |
| B button / `key_b` | `del` |
| Z trigger / `key_z` | `doc` |
| R trigger / `key_r` | `shift` |
| Start / `key_start` | `enter` |
| C up, down, left, right / `key_c_up` ... | `8`, `2`, `4`, `6` |
| Settings / `key_settings` | `tab` |

`esc` asks whether to quit the game; press Enter to quit or any other key to keep playing. `ctrl` pauses on a speed info screen: frame rate, where each frame's time goes, your settings and play time; press any key to go back. To see the frame rate all the time, turn on the FPS counter.

## Older config files

Older config files are upgraded automatically on first start, keeping their values. `enable_120p_mode true` becomes `resolution 160x120` (`false` becomes `320x240`), `flat_colors true` becomes `textures flat`, `smooth_textures true` becomes `textures smooth`, and `sharp_hud true` / `false` becomes `sharp_hud on_and_menus` / `off`. `sharp_text` from v5 becomes `sharp_hud` (`hud` becomes `on`, `hud_and_menus` becomes `on_and_menus`). New settings are added with their defaults. The old `texture_filtering` line did nothing and is dropped, as are old numeric `key_*` lines.

