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

#include "fps_overlay.h"
#include "profiling.h"
#include "timer.h"
#include "gfx/gfx_backend.h"

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
      "Keeps the game at full speed by not drawing some\n"
      "frames. Auto skips only as many as needed (best).\n"
      "Off draws every frame, so busy scenes go slow.\n"
      "At most 1-30: skip up to that many in a row.",
      "SPEED" },
    { "FPS counter", &configShowFps, PLAIN,
      "Shows frames drawn per second and game speed in\n"
      "the bottom-left corner. 100% is full speed, lower\n"
      "is slow motion. Presets don't change it." },
    { "3D resolution", NULL, RESOLUTION,
      "Resolution of the 3D world (lower ones are\n"
      "stretched). 320x240 is sharp but slow, 160x120\n"
      "much faster but blocky, 80x60 fastest and very\n"
      "blocky. The HUD follows the Sharp HUD setting.",
      "PICTURE" },
    { "Textures", NULL, TEXTURES,
      "Flat: solid surfaces in one colour each, much\n"
      "faster (trees, coins, text and the sky keep their\n"
      "textures). Normal: as in the original. Smooth:\n"
      "blurred slightly like the N64, much slower." },
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
    { "Sharp HUD", NULL, SHARP_TEXT,
      "Keeps the HUD (lives, coins, stars), dialogs and\n"
      "pause menu sharp at low 3D resolution. On + menus:\n"
      "also the title, file and star select screens.\n"
      "Off is a little faster." },
};

static const struct Item cheat_items[] = {
    { "Infinite health", &configCheatHealth, PLAIN,
      "Health refills every frame: enemies, fire, falls\n"
      "and lack of air can't hurt Mario. Bottomless pits\n"
      "still cost a life." },
    { "Infinite lives", &configCheatLives, PLAIN,
      "Lives stay at 99, so no game over." },
    { "Moon jump", &configCheatMoonJump, PLAIN,
      "Hold the A button (menu key by default) in the\n"
      "air and Mario keeps rising. Let go to fall." },
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

// first help line on the Controls page: what the selected button does in the game
static const char *const control_use[CONTROL_COUNT] = {
    [CONTROL_A] = "A button: jump, swim, read signs.",
    [CONTROL_B] = "B button: punch, kick, dive, pick things up.",
    [CONTROL_Z] = "Z trigger: crouch; in the air, ground pound.",
    [CONTROL_R] = "R trigger: switch camera (Mario or Lakitu).",
    [CONTROL_START] = "Start: pause the game.",
    [CONTROL_C_UP] = "C up: look around (first person).",
    [CONTROL_C_DOWN] = "C down: move the camera back.",
    [CONTROL_C_LEFT] = "C left: turn the camera.",
    [CONTROL_C_RIGHT] = "C right: turn the camera.",
    [CONTROL_STICK_UP] = "Stick: walk and run.",
    [CONTROL_STICK_DOWN] = "Stick: walk and run.",
    [CONTROL_STICK_LEFT] = "Stick: walk and run.",
    [CONTROL_STICK_RIGHT] = "Stick: walk and run.",
    [CONTROL_SETTINGS] = "Settings: opens this screen during play.",
};

static const char *const controls_help =
    "Press left or right, then the new key. Esc cancels.\n"
    "A key already in use swaps with this one.\n"
    "esc (quit) and ctrl (speed info) can't be used.";

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
            static const char *const names[SHARP_TEXT_COUNT] = { "Off", "On", "On + menus" };
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

// Screen rows, the same on every page: header, list, help box, status, footer.
#define HELP_ROW 19   // first help line (a rule above and below)
static int cur_row;   // rows printed so far

static void put_line(const char *s) {
    nio_printf("%-*.*s\n", LINE_W, LINE_W, s);
    cur_row++;
}

static void pad_to(int row) {
    while (cur_row < row - 1) {
        put_line("");
    }
}

static void put_heading(nio_console *console, const char *s) {
    nio_color(console, NIO_COLOR_WHITE, NIO_COLOR_BLUE);
    put_line(s);
    nio_color(console, NIO_COLOR_WHITE, NIO_COLOR_BLACK);
}

static void put_rule(nio_console *console) {
    char rule[LINE_W + 1];
    memset(rule, '-', LINE_W);
    rule[LINE_W] = '\0';
    nio_color(console, NIO_COLOR_WHITE, NIO_COLOR_GRAY);
    put_line(rule);
    nio_color(console, NIO_COLOR_WHITE, NIO_COLOR_BLACK);
}

// first: an optional line shown before help (Controls page: what the button does)
static void put_help(nio_console *console, const char *first, const char *help) {
    int lines = 0;
    char line[LINE_W + 1];
    nio_color(console, NIO_COLOR_WHITE, NIO_COLOR_LIGHTBLACK);
    if (first != NULL) {
        put_line(first);
        lines++;
    }
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

// one row of the preset bar: presets first..last, the active one highlighted
static void put_presets(nio_console *console, int first, int last, const char *tail) {
    const enum ConfigPreset now = configfile_current_preset();
    int used = nio_printf(" ");
    for (int p = first; p <= last; p++) {
        if (p == (int) now) {
            nio_color(console, NIO_COLOR_BLUE, NIO_COLOR_WHITE);
        }
        used += nio_printf("%d %s", p + 1, configPresetNames[p]);
        nio_color(console, NIO_COLOR_WHITE, NIO_COLOR_BLACK);
        used += nio_printf("  ");
    }
    nio_color(console, NIO_COLOR_WHITE, NIO_COLOR_LIGHTBLACK);
    nio_printf("%-*s\n", LINE_W - used, tail);
    nio_color(console, NIO_COLOR_WHITE, NIO_COLOR_BLACK);
    cur_row++;
}

static void draw(nio_console *console, bool paused, const char *status) {
    char line[LINE_W + 1], value[24];

    nio_color(console, NIO_COLOR_WHITE, NIO_COLOR_BLACK);
    nio_clear(console);
    cur_row = 0;
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
    cur_row++;
    put_line("");

    if (page == PAGE_SETTINGS) {
        put_heading(console, "PRESETS (keys 1 to 5)");
        put_presets(console, PRESET_ULTRA_PERFORMANCE, PRESET_BALANCED, "");
        put_presets(console, PRESET_QUALITY, PRESET_ULTRA_QUALITY,
                    configfile_current_preset() == PRESET_CUSTOM ? "Now: your own mix" : "");
    } else if (page == PAGE_CHEATS) {
        int on = 0;
        for (int i = 0; i < N_CHEATS; i++) {
            on += *cheat_items[i].flag;
        }
        snprintf(line, sizeof(line), "CHEATS (%d on)", on);
        put_heading(console, line);
    }

    const int rows = page_rows(page);
    for (int row = 0; row < rows; row++) {
        const char *name;
        if (page == PAGE_CONTROLS) {
            name = controls_name(row);
            snprintf(value, sizeof(value), "%s", controls_key_name(controlKeys[row]));
        } else {
            const struct Item *it = page == PAGE_SETTINGS ? &settings_items[row] : &cheat_items[row];
            if (it->group != NULL) {
                put_heading(console, it->group);
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

    pad_to(HELP_ROW - 1);
    put_rule(console);
    if (page == PAGE_SETTINGS) {
        put_help(console, NULL, settings_items[sel[page]].help);
    } else if (page == PAGE_CHEATS) {
        put_help(console, NULL, cheat_items[sel[page]].help);
    } else {
        put_help(console, control_use[sel[page]], controls_help);
    }
    put_rule(console);

    // status row: always there, so the footer never moves
    if (status != NULL) {
        nio_color(console, NIO_COLOR_RED, NIO_COLOR_WHITE);
        put_line(status);
        nio_color(console, NIO_COLOR_WHITE, NIO_COLOR_BLACK);
    } else {
        put_line("");
    }
    put_line(page == PAGE_SETTINGS  ? "up/down: choose   left/right: change   1-5: preset"
             : page == PAGE_CHEATS ? "up/down: choose   left/right: turn on or off"
                                   : "up/down: choose   left/right: set a new key");
    put_line("0 twice: reset settings, cheats and keys");
    put_line(paused ? "Enter: save and go back       Esc: undo and go back"
                    : "Enter: save and play                       Esc: quit");
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

// ---- speed info screen (ctrl during play) ----

// "12.3" from microseconds
static void put_ms(char *buf, size_t size, uint32_t us) {
    snprintf(buf, size, "%lu.%lu", (unsigned long) (us / 1000), (unsigned long) (us % 1000 / 100));
}

static void put_stage(const char *name, uint32_t us, uint32_t total_us) {
    char line[LINE_W + 1], ms[16], bar[11];
    const uint32_t pct = total_us > 0 ? (uint32_t) ((uint64_t) us * 100 / total_us) : 0;
    const int n = (int) ((pct + 5) / 10);
    memset(bar, '#', n);
    bar[n] = '\0';
    put_ms(ms, sizeof(ms), us);
    snprintf(line, sizeof(line), "  %-18s %6s ms %3lu%%  %s", name, ms, (unsigned long) pct, bar);
    put_line(line);
}

void settings_menu_speed_info(int skipped) {
    static const char *const textures[TEXTURES_COUNT] = { "flat", "normal", "smooth" };
    static const char *const sharp[SHARP_TEXT_COUNT] = { "off", "on", "on + menus" };
    nio_console console;
    nio_console *const prev_default = nio_get_default();
    if (!nio_init(&console, NIO_MAX_COLS, NIO_MAX_ROWS, 0, 0, NIO_COLOR_WHITE, NIO_COLOR_BLACK, true)) {
        return;
    }
    nio_set_default(&console);
    nio_clear(&console);
    cur_row = 0;
    char line[LINE_W + 1], a[24];

    put_line("SUPER MARIO 64 - speed info");
    put_line("");

    // SPEED: the last full second, from the FPS counter's measurement
    put_heading(&console, "SPEED (last second)");
    uint32_t frames, ticks, ms;
    if (fps_overlay_last(&frames, &ticks, &ms)) {
        const uint32_t fps10 = (frames * 10000u + ms / 2) / ms;
        const uint32_t speed = (ticks * 100000u / 30u + ms / 2) / ms;
        snprintf(line, sizeof(line), "  Frames drawn      %lu.%lu per second", (unsigned long) (fps10 / 10),
                 (unsigned long) (fps10 % 10));
        put_line(line);
        snprintf(line, sizeof(line), "  Game speed        %lu%% %s", (unsigned long) speed,
                 speed >= 95 ? "(full speed)" : "(slow motion)");
        put_line(line);
        if (ticks > frames) {
            snprintf(line, sizeof(line), "  Frames skipped    %lu of every %lu", (unsigned long) (ticks - frames),
                     (unsigned long) ticks);
        } else {
            snprintf(line, sizeof(line), "  Frames skipped    none");
        }
        put_line(line);
    } else {
        put_line("  Not measured yet: play for a second first.");
        put_line("");
        put_line("");
    }
    put_line("");

    // TIME PER DRAWN FRAME: the last drawn frame, split into stages
    const uint32_t logic = prof_ticks_to_us(gProfFrame.ticks[PROF_LOGIC]);
    const uint32_t scene = prof_ticks_to_us(gProfFrame.ticks[PROF_SCENE]);
    const uint32_t gfx = prof_ticks_to_us(gProfFrame.ticks[PROF_GFX]);
    const uint32_t raster = prof_ticks_to_us(gProfFrame.raster_ticks);
    const uint32_t present = prof_ticks_to_us(gProfFrame.ticks[PROF_PRESENT]);
    const uint32_t total = logic + scene + gfx + present;
    put_ms(a, sizeof(a), total);
    snprintf(line, sizeof(line), "LAST DRAWN FRAME              %8s ms total", a);
    put_heading(&console, line);
    put_stage("Game logic", logic, total);
    put_stage("Scene setup", scene, total);
    put_stage("3D setup", gfx > raster ? gfx - raster : 0, total);
    put_stage("Drawing triangles", raster, total);
    put_stage("Copy to screen", present, total);
    snprintf(line, sizeof(line), "  Triangles         %lu", (unsigned long) gProfFrame.tris);
    put_line(line);
    snprintf(line, sizeof(line), "  Skipped frames: %d. They cost only logic + scene.", skipped);
    put_line(line);
    put_line("");

    // SETTINGS
    const enum ConfigPreset preset = configfile_current_preset();
    snprintf(line, sizeof(line), "SETTINGS                 preset: %s",
             preset == PRESET_CUSTOM ? "your own mix" : configPresetNames[preset]);
    put_heading(&console, line);
    snprintf(line, sizeof(line), "  %s, %s textures, sky %s, fog %s,",
             configfile_resolution_name(configResolution), textures[configTextures % TEXTURES_COUNT],
             configDrawSky ? "on" : "off", configEnableFog ? "on" : "off");
    put_line(line);
    if (configFrameskip == FRAMESKIP_AUTO) {
        snprintf(a, sizeof(a), "Auto");
    } else if (configFrameskip == 0) {
        snprintf(a, sizeof(a), "off");
    } else {
        snprintf(a, sizeof(a), "at most %u", configFrameskip);
    }
    snprintf(line, sizeof(line), "  shadows %s, Sharp HUD %s", configDrawShadows ? "on" : "off",
             sharp[configSharpText % SHARP_TEXT_COUNT]);
    put_line(line);
    snprintf(line, sizeof(line), "  Frame skip        %s", a);
    put_line(line);
    snprintf(line, sizeof(line), "  Texture memory    %lu KB of 1536 KB",
             (unsigned long) (gfx_soft_texture_bytes() / 1024));
    put_line(line);
    put_line("");

    // SESSION
    put_heading(&console, "SESSION");
    const uint32_t secs = tmr_ms() / 1000;
    snprintf(line, sizeof(line), "  Play time         %lu:%02lu:%02lu", (unsigned long) (secs / 3600),
             (unsigned long) (secs / 60 % 60), (unsigned long) (secs % 60));
    put_line(line);
    snprintf(line, sizeof(line), "  Calculator        TI-Nspire %s", is_cx2 ? "CX II" : "CX");
    put_line(line);
    // cheats that are on, by name, cut to the line
    int on = 0;
    snprintf(line, sizeof(line), "  Cheats            ");
    for (int i = 0; i < N_CHEATS; i++) {
        if (*cheat_items[i].flag) {
            const size_t len = strlen(line);
            snprintf(line + len, sizeof(line) - len, "%s%s", on > 0 ? ", " : "", cheat_items[i].name);
            on++;
        }
    }
    if (on == 0) {
        snprintf(line + strlen(line), sizeof(line) - strlen(line), "none");
    }
    put_line(line);
    put_line("");
    put_line("Press any key to go back to the game.");

    wait_key_pressed(); // waits for ctrl to be let go first
    wait_no_key_pressed(); // and this key, so the game doesn't get it
    nio_free(&console);
    nio_set_default(prev_default);
}

bool settings_menu_confirm_quit(void) {
    nio_console console;
    nio_console *const prev_default = nio_get_default();
    if (!nio_init(&console, NIO_MAX_COLS, NIO_MAX_ROWS, 0, 0, NIO_COLOR_WHITE, NIO_COLOR_BLACK, true)) {
        return true;
    }
    nio_set_default(&console);
    nio_clear(&console);
    cur_row = 0;
    put_line("SUPER MARIO 64 - paused");
    put_line("");
    put_heading(&console, "Quit the game?");
    put_line("");
    put_line("Anything you haven't saved in the game is lost.");
    put_line("");
    put_line("Enter: quit          any other key: keep playing");

    wait_key_pressed(); // waits for esc to be let go first
    const bool quit = isKeyPressed(KEY_NSPIRE_ENTER) || isKeyPressed(KEY_NSPIRE_RET);
    wait_no_key_pressed();
    nio_free(&console);
    nio_set_default(prev_default);
    return quit;
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
                draw(&console, paused, "Not saved; changes last until you quit. Press a key.");
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

void settings_menu_speed_info(int skipped) {
    (void) skipped;
}

bool settings_menu_confirm_quit(void) {
    return true;
}

#endif
