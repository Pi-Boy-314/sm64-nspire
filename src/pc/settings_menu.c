// settings_menu.c - settings screen on the calculator: settings, cheats and controls pages.
// Shown at start (before the renderer starts) and during play with the settings key.
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "settings_menu.h"
#include "configfile.h"
#include "controls.h"

#if defined(TARGET_NSP) && !defined(BENCHMARK)

#include <libndls.h>
#include <nspireio/nspireio.h>

#define LINE_W 52 // console is 53 columns; a full 53 would wrap
#define HELP_LINES 4

enum Page { PAGE_SETTINGS, PAGE_CHEATS, PAGE_CONTROLS, PAGE_COUNT };
static const char *const page_names[PAGE_COUNT] = { "Settings", "Cheats", "Controls" };

enum Special { PLAIN, RESOLUTION, FRAMESKIP, TEXTURES, SHARP_TEXT };

struct Item {
    const char *name;
    bool *flag;           // on/off items
    enum Special special; // items that aren't on/off
    const char *help;     // up to HELP_LINES lines
    const char *group;    // heading shown above this item, if any
};

static const struct Item settings_items[] = {
    { "Frame skip", NULL, FRAMESKIP,
      "Keeps the game at full speed by skipping the\n"
      "drawing of some frames. Auto skips only as many\n"
      "as needed (best for most). Off draws every frame,\n"
      "but slow motion when busy. 1-30: skip at most N.",
      "SPEED" },
    { "FPS counter", &configShowFps, PLAIN,
      "Shows frames drawn per second and game speed in\n"
      "the bottom-left corner. 100% is full speed, lower\n"
      "is slow motion." },
    { "3D resolution", NULL, RESOLUTION,
      "Resolution of the 3D world, stretched to the\n"
      "screen. 320x240 is sharp but slow, 160x120 much\n"
      "faster but blocky, 80x60 fastest and very blocky.\n"
      "Text and HUD follow the Sharp HUD setting.",
      "PICTURE" },
    { "Textures", NULL, TEXTURES,
      "Normal: as in the original. Smooth: filtered like\n"
      "the N64, much slower. Flat: walls and floors in\n"
      "one colour each, much faster (trees, coins, text\n"
      "and the sky keep their textures)." },
    { "Sky", &configDrawSky, PLAIN,
      "Off shows a plain black sky. Faster, because the\n"
      "sky covers a big part of the screen." },
    { "Fog", &configEnableFog, PLAIN,
      "Distance fog in the levels that use it. Closer to\n"
      "the original look, but fogged areas draw more\n"
      "slowly." },
    { "Shadows", &configDrawShadows, PLAIN,
      "Shadows under Mario, enemies and items. Off is a\n"
      "little faster, but jumps are harder to judge." },
    { "Sharp text", NULL, SHARP_TEXT,
      "Readable text at low resolutions. HUD: HUD, dialog\n"
      "boxes and pause menu at full 320x240. HUD + menus:\n"
      "also whole menu screens (title, file and star\n"
      "select). Off is a little faster." },
};

static const struct Item cheat_items[] = {
    { "Infinite health", &configCheatHealth, PLAIN,
      "Health refills every frame: enemies, fire, falls\n"
      "and lack of air can't hurt Mario. Bottomless pits\n"
      "still cost a life." },
    { "Infinite lives", &configCheatLives, PLAIN,
      "Lives stay at 99, so no game over." },
    { "Moon jump", &configCheatMoonJump, PLAIN,
      "Hold the A button in the air and Mario keeps\n"
      "rising. Let go to fall." },
    { "Super speed", &configCheatSuperSpeed, PLAIN,
      "Mario covers twice the ground when walking,\n"
      "running or sliding." },
    { "Keep caps", &configCheatKeepCaps, PLAIN,
      "Wing, metal and vanish caps don't run out until\n"
      "you leave the level or lose the cap." },
    { "Skip intro", &configCheatSkipIntro, PLAIN,
      "A new save file starts right outside the castle,\n"
      "without Peach's letter and Lakitu's flight." },
    { "Exit course anytime", &configCheatExitCourse, PLAIN,
      "The pause menu always offers Exit course, even in\n"
      "mid-air or swimming." },
};

#define N_SETTINGS ((int) (sizeof(settings_items) / sizeof(settings_items[0])))
#define N_CHEATS ((int) (sizeof(cheat_items) / sizeof(cheat_items[0])))

static const char *const controls_help =
    "Press left or right, then the new key. Esc cancels.\n"
    "Esc (quit) and ctrl (statistics) can't be used.\n"
    "A key already in use swaps with this one.";

// remembered while the game runs, so the screen reopens where it was left
static int page = PAGE_SETTINGS;
static int sel[PAGE_COUNT];

// everything the screen can change, to undo (Esc during play) and to save only on changes
struct Snapshot {
    unsigned int res, textures, sharp_text;
    unsigned int frameskip;
    bool flags[N_SETTINGS + N_CHEATS];
    int keys[CONTROL_COUNT];
};

static void snapshot_take(struct Snapshot *s) {
    memset(s, 0, sizeof(*s));
    s->res = configResolution;
    s->textures = configTextures;
    s->sharp_text = configSharpText;
    s->frameskip = configFrameskip;
    for (int i = 0; i < N_SETTINGS; i++) {
        s->flags[i] = settings_items[i].flag != NULL && *settings_items[i].flag;
    }
    for (int i = 0; i < N_CHEATS; i++) {
        s->flags[N_SETTINGS + i] = *cheat_items[i].flag;
    }
    memcpy(s->keys, controlKeys, sizeof(s->keys));
}

static void snapshot_restore(const struct Snapshot *s) {
    configResolution = s->res;
    configTextures = s->textures;
    configSharpText = s->sharp_text;
    configFrameskip = s->frameskip;
    for (int i = 0; i < N_SETTINGS; i++) {
        if (settings_items[i].flag != NULL) {
            *settings_items[i].flag = s->flags[i];
        }
    }
    for (int i = 0; i < N_CHEATS; i++) {
        *cheat_items[i].flag = s->flags[N_SETTINGS + i];
    }
    memcpy(controlKeys, s->keys, sizeof(s->keys));
}

static bool snapshot_equal(const struct Snapshot *a, const struct Snapshot *b) {
    return memcmp(a, b, sizeof(*a)) == 0; // memset in snapshot_take zeroes the padding
}

static int page_rows(int p) {
    return p == PAGE_SETTINGS ? N_SETTINGS : p == PAGE_CHEATS ? N_CHEATS : CONTROL_COUNT;
}

static void item_value(const struct Item *it, char *buf, size_t size) {
    switch (it->special) {
        case RESOLUTION:
            snprintf(buf, size, "%s", configfile_resolution_name(configResolution));
            break;
        case TEXTURES: {
            static const char *const names[TEXTURES_COUNT] = { "Flat (fastest)", "Normal",
                                                               "Smooth (slow)" };
            snprintf(buf, size, "%s", names[configTextures % TEXTURES_COUNT]);
            break;
        }
        case SHARP_TEXT: {
            static const char *const names[SHARP_TEXT_COUNT] = { "Off", "HUD", "HUD + menus" };
            snprintf(buf, size, "%s", names[configSharpText % SHARP_TEXT_COUNT]);
            break;
        }
        case FRAMESKIP:
            if (configFrameskip == FRAMESKIP_AUTO) {
                snprintf(buf, size, "Auto (full speed)");
            } else if (configFrameskip == 0) {
                snprintf(buf, size, "Off (slow motion)");
            } else {
                snprintf(buf, size, "At most %u", configFrameskip);
            }
            break;
        default:
            snprintf(buf, size, "%s", *it->flag ? "On" : "Off");
            break;
    }
}

// left (dir -1) or right (+1) on a settings or cheats row. Frameskip order: Auto, 0 .. 30.
static void item_change(const struct Item *it, int dir) {
    switch (it->special) {
        case RESOLUTION: // right: higher resolution (slower, sharper), like Textures
            if (dir < 0 && configResolution + 1 < RES_COUNT) {
                configResolution++;
            } else if (dir > 0 && configResolution > 0) {
                configResolution--;
            }
            break;
        case SHARP_TEXT: // off, hud, hud + menus
            if (dir > 0 && configSharpText + 1 < SHARP_TEXT_COUNT) {
                configSharpText++;
            } else if (dir < 0 && configSharpText > 0) {
                configSharpText--;
            }
            break;
        case TEXTURES: // flat, normal, smooth
            if (dir > 0 && configTextures + 1 < TEXTURES_COUNT) {
                configTextures++;
            } else if (dir < 0 && configTextures > 0) {
                configTextures--;
            }
            break;
        case FRAMESKIP:
            if (configFrameskip == FRAMESKIP_AUTO) {
                if (dir > 0) {
                    configFrameskip = 0;
                }
            } else if (dir < 0) {
                configFrameskip = configFrameskip == 0 ? FRAMESKIP_AUTO : configFrameskip - 1;
            } else if (configFrameskip < FRAMESKIP_MAX) {
                configFrameskip++;
            }
            break;
        default:
            *it->flag = !*it->flag;
            break;
    }
}

static void put_line(const char *s) {
    nio_printf("%-*.*s\n", LINE_W, LINE_W, s);
}

static void put_help(nio_console *console, const char *help) {
    int lines = 0;
    char line[LINE_W + 1];
    nio_color(console, NIO_COLOR_WHITE, NIO_COLOR_GRAY);
    for (const char *p = help; *p != '\0' && lines < HELP_LINES; lines++) {
        const char *nl = strchr(p, '\n');
        const size_t n = nl != NULL ? (size_t) (nl - p) : strlen(p);
        snprintf(line, sizeof(line), "%.*s", (int) n, p);
        put_line(line);
        p += n + (nl != NULL);
    }
    nio_color(console, NIO_COLOR_WHITE, NIO_COLOR_BLACK);
    for (; lines < HELP_LINES; lines++) {
        put_line("");
    }
}

static void draw(nio_console *console, bool paused, const char *status) {
    char line[LINE_W + 1], value[24];

    nio_color(console, NIO_COLOR_WHITE, NIO_COLOR_BLACK);
    nio_clear(console);
    put_line(paused ? "SUPER MARIO 64 - paused" : "SUPER MARIO 64 for TI-Nspire");

    // page bar: the current page highlighted
    int used = 0;
    for (int p = 0; p < PAGE_COUNT; p++) {
        if (p == page) {
            nio_color(console, NIO_COLOR_BLUE, NIO_COLOR_WHITE);
        }
        used += nio_printf(" %s ", page_names[p]);
        nio_color(console, NIO_COLOR_WHITE, NIO_COLOR_BLACK);
        used += nio_printf(" ");
    }
    nio_printf("%*s\n", LINE_W - used, "tab: next page");
    put_line("");

    const int rows = page_rows(page);
    for (int row = 0; row < rows; row++) {
        const char *name;
        if (page == PAGE_CONTROLS) {
            name = controls_name(row);
            snprintf(value, sizeof(value), "%s", controls_key_name(controlKeys[row]));
        } else {
            const struct Item *it = page == PAGE_SETTINGS ? &settings_items[row] : &cheat_items[row];
            if (it->group != NULL) {
                put_line(it->group);
            }
            name = it->name;
            item_value(it, value, sizeof(value));
        }
        snprintf(line, sizeof(line), " %s %-20s %s", row == sel[page] ? ">" : " ", name, value);
        if (row == sel[page]) {
            nio_color(console, NIO_COLOR_BLUE, NIO_COLOR_WHITE);
        }
        put_line(line);
        nio_color(console, NIO_COLOR_WHITE, NIO_COLOR_BLACK);
    }
    put_line("");

    if (page == PAGE_SETTINGS) {
        put_line("Presets: 1 Ultra Performance  2 Performance");
        put_line("         3 Balanced  4 Quality  5 Ultra Quality");
        const enum ConfigPreset preset = configfile_current_preset();
        snprintf(line, sizeof(line), "Now: %s",
                 preset == PRESET_CUSTOM ? "Custom" : configPresetNames[preset]);
        put_line(line);
        put_line("");
        put_help(console, settings_items[sel[page]].help);
    } else if (page == PAGE_CHEATS) {
        put_help(console, cheat_items[sel[page]].help);
    } else {
        put_help(console, controls_help);
    }

    put_line("");
    put_line(page == PAGE_CONTROLS ? "up/down: choose   left/right: set a new key"
                                   : "up/down: choose   left/right: change");
    put_line("0: reset everything to defaults");
    put_line(paused ? "Enter: save and go back   Esc: undo changes"
                    : "Enter: save and play      Esc: quit");
    if (status != NULL) {
        nio_color(console, NIO_COLOR_WHITE, NIO_COLOR_RED);
        put_line(status);
        nio_color(console, NIO_COLOR_WHITE, NIO_COLOR_BLACK);
    }
}

// The menu's own key waits: they also see the touchpad arrows, which any_key_pressed() may not.
static void wait_release(void) {
    while (any_key_pressed() || controls_pressed_key() != KEY_NONE) {
        idle();
    }
}

static void wait_press(void) {
    while (!any_key_pressed() && controls_pressed_key() == KEY_NONE) {
        idle();
    }
}

// Controls page: waits for the new key for the selected button. Returns a message, or NULL.
static const char *capture_key(nio_console *console, bool paused) {
    static char msg[LINE_W + 1];
    const int control = sel[PAGE_CONTROLS];
    snprintf(msg, sizeof(msg), "Press the new key for %s (esc: cancel)", controls_name(control));
    draw(console, paused, msg);
    wait_release();
    wait_press();
    const int key = controls_pressed_key();
    wait_release();
    if (key == KEY_NONE || strcmp(controls_key_name(key), "esc") == 0) {
        return NULL;
    }
    if (controls_key_reserved(key)) {
        snprintf(msg, sizeof(msg), "%s is reserved, pick another key", controls_key_name(key));
        return msg;
    }
    // a key already used by another button swaps with this one
    const int old = controlKeys[control];
    controlKeys[control] = key;
    for (int c = 0; c < CONTROL_COUNT; c++) {
        if (c != control && controlKeys[c] == key) {
            controlKeys[c] = old;
            snprintf(msg, sizeof(msg), "%s now uses %s", controls_name(c), controls_key_name(old));
            return msg;
        }
    }
    return NULL;
}

// Runs the screen. Returns true on Enter (changes kept and saved), false on Esc.
static bool run(bool paused) {
    nio_console console;
    // nio_printf() writes to the default console. This one lives on the stack, so the previous
    // default must be put back before returning, or later printing writes to freed memory.
    nio_console *const prev_default = nio_get_default();
    if (!nio_init(&console, NIO_MAX_COLS, NIO_MAX_ROWS, 0, 0, NIO_COLOR_WHITE, NIO_COLOR_BLACK, true)) {
        return true; // no console: carry on with the current settings
    }
    nio_set_default(&console);

    struct Snapshot before, now;
    snapshot_take(&before);

    bool keep = true, reset_armed = false;
    const char *status = NULL;
    wait_release(); // the key that started the game or opened the screen
    while (true) {
        draw(&console, paused, status);
        status = NULL;
        wait_press();
        const bool was_armed = reset_armed;
        reset_armed = false;

        if (isKeyPressed(KEY_NSPIRE_ENTER) || isKeyPressed(KEY_NSPIRE_RET)) {
            snapshot_take(&now);
            if (!snapshot_equal(&before, &now) && !configfile_save(CONFIG_FILE)) {
                // keep the new settings anyway; they just won't be there next time
                draw(&console, paused, "Could not save the settings. Press a key.");
                wait_release();
                wait_press();
            }
            break;
        }
        if (isKeyPressed(KEY_NSPIRE_ESC)) {
            keep = false;
            break;
        }

        const int rows = page_rows(page);
        if (isKeyPressed(KEY_NSPIRE_TAB)) {
            page = (page + 1) % PAGE_COUNT;
        } else if (isKeyPressed(KEY_NSPIRE_UP)) {
            sel[page] = (sel[page] + rows - 1) % rows;
        } else if (isKeyPressed(KEY_NSPIRE_DOWN)) {
            sel[page] = (sel[page] + 1) % rows;
        } else if (isKeyPressed(KEY_NSPIRE_LEFT) || isKeyPressed(KEY_NSPIRE_RIGHT)) {
            const int dir = isKeyPressed(KEY_NSPIRE_LEFT) ? -1 : 1;
            if (page == PAGE_SETTINGS) {
                item_change(&settings_items[sel[page]], dir);
            } else if (page == PAGE_CHEATS) {
                item_change(&cheat_items[sel[page]], dir);
            } else {
                status = capture_key(&console, paused);
                continue; // capture_key already waited for the keys to be let go
            }
        } else if (isKeyPressed(KEY_NSPIRE_1)) {
            configfile_apply_preset(PRESET_ULTRA_PERFORMANCE);
        } else if (isKeyPressed(KEY_NSPIRE_2)) {
            configfile_apply_preset(PRESET_PERFORMANCE);
        } else if (isKeyPressed(KEY_NSPIRE_3)) {
            configfile_apply_preset(PRESET_BALANCED);
        } else if (isKeyPressed(KEY_NSPIRE_4)) {
            configfile_apply_preset(PRESET_QUALITY);
        } else if (isKeyPressed(KEY_NSPIRE_5)) {
            configfile_apply_preset(PRESET_ULTRA_QUALITY);
        } else if (isKeyPressed(KEY_NSPIRE_0)) {
            if (was_armed) {
                configfile_reset_defaults();
                status = "Reset to defaults (not saved until Enter)";
            } else {
                reset_armed = true;
                status = "Press 0 again to reset settings, cheats and keys";
            }
        }
        wait_release();
    }
    wait_release(); // don't let Enter also press Start in the game
    nio_free(&console);
    nio_set_default(prev_default);
    if (!keep) {
        snapshot_restore(&before);
    }
    return keep;
}

bool settings_menu_run(void) {
    return run(false);
}

void settings_menu_run_paused(void) {
    run(true);
}

#else

bool settings_menu_run(void) {
    return true;
}

void settings_menu_run_paused(void) {
}

#endif
