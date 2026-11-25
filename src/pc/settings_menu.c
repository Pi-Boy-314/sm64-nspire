// settings_menu.c - start screen on the calculator: controls, settings, presets and cheats.
// Runs before the renderer starts, so a resolution change applies straight away.
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "settings_menu.h"
#include "configfile.h"

#if defined(TARGET_NSP) && !defined(BENCHMARK)

#include <libndls.h>
#include <nspireio/nspireio.h>

#define LINE_W 52 // console is 53 columns; a full 53 would wrap

enum Row {
    ROW_RESOLUTION,
    ROW_FRAMESKIP,
    ROW_SKY,
    ROW_FOG,
    ROW_CHEAT_HEALTH,
    ROW_CHEAT_LIVES,
    ROW_CHEAT_MOON_JUMP,
    ROW_COUNT,
};

static const char *const row_names[ROW_COUNT] = {
    "Resolution", "Frameskip", "Sky", "Fog", "Infinite health", "Infinite lives", "Moon jump",
};

// shown under the menu for the selected row, up to 4 lines
static const char *const row_help[ROW_COUNT] = {
    "Low draws 160x120 and stretches it to the screen:\n"
    "much faster, but blocky and small text is hard to\n"
    "read. High is the full 320x240: sharp but slow.\n"
    "Applies as soon as you start.",
    "Most frames in a row the game may skip drawing to\n"
    "keep its real speed. 0 draws every frame, but the\n"
    "game runs in slow motion when it can't keep up.\n"
    "Higher: closer to real speed, but jumpier.",
    "Off shows a plain black sky. Faster, because the\n"
    "sky covers a big part of the screen.",
    "Distance fog in the levels that use it (Bowser\n"
    "levels, for example). Closer to the original look\n"
    "but fogged areas draw more slowly.",
    "Cheat. Health refills every frame: enemies, fire,\n"
    "falls and lack of air can't hurt Mario.\n"
    "Bottomless pits still cost a life.",
    "Cheat. Lives stay at 99, so no game over.",
    "Cheat. Hold A (menu key) in the air and Mario\n"
    "keeps rising. Let go to fall.",
};

static const char *on_off(bool b) {
    return b ? "On" : "Off";
}

static void row_value(int row, char *buf, size_t size) {
    switch (row) {
        case ROW_RESOLUTION:
            snprintf(buf, size, "%s", config120pMode ? "Low (160x120)" : "High (320x240)");
            break;
        case ROW_FRAMESKIP:
            snprintf(buf, size, "%u", configFrameskip);
            break;
        case ROW_SKY:
            snprintf(buf, size, "%s", on_off(configDrawSky));
            break;
        case ROW_FOG:
            snprintf(buf, size, "%s", on_off(configEnableFog));
            break;
        case ROW_CHEAT_HEALTH:
            snprintf(buf, size, "%s", on_off(configCheatHealth));
            break;
        case ROW_CHEAT_LIVES:
            snprintf(buf, size, "%s", on_off(configCheatLives));
            break;
        case ROW_CHEAT_MOON_JUMP:
            snprintf(buf, size, "%s", on_off(configCheatMoonJump));
            break;
    }
}

// left (dir -1) or right (+1) on a row
static void row_change(int row, int dir) {
    switch (row) {
        case ROW_RESOLUTION:
            config120pMode = !config120pMode;
            break;
        case ROW_FRAMESKIP:
            if (dir < 0 && configFrameskip > 0) {
                configFrameskip--;
            } else if (dir > 0 && configFrameskip < FRAMESKIP_MAX) {
                configFrameskip++;
            }
            break;
        case ROW_SKY:
            configDrawSky = !configDrawSky;
            break;
        case ROW_FOG:
            configEnableFog = !configEnableFog;
            break;
        case ROW_CHEAT_HEALTH:
            configCheatHealth = !configCheatHealth;
            break;
        case ROW_CHEAT_LIVES:
            configCheatLives = !configCheatLives;
            break;
        case ROW_CHEAT_MOON_JUMP:
            configCheatMoonJump = !configCheatMoonJump;
            break;
    }
}

static void put_line(const char *s) {
    nio_printf("%-*.*s\n", LINE_W, LINE_W, s);
}

static void draw(nio_console *console, int sel, const char *status) {
    char line[LINE_W + 1], value[24];

    nio_color(console, NIO_COLOR_WHITE, NIO_COLOR_BLACK);
    nio_clear(console);
    put_line("SUPER MARIO 64 for TI-Nspire");
    put_line("");
    put_line("Controls");
    put_line(" Stick: touchpad  A: menu  B: del  Z: doc");
    put_line(" Start: enter  C buttons: 8 up 2 down 4 left 6 right");
    put_line(" In game: Esc quits, Ctrl shows speed statistics");
    put_line("");
    put_line("Settings     up/down: choose   left/right: change");
    for (int row = 0; row < ROW_COUNT; row++) {
        if (row == ROW_CHEAT_HEALTH) {
            put_line("");
            put_line("Cheats");
        }
        row_value(row, value, sizeof(value));
        snprintf(line, sizeof(line), " %s %-16s %s", row == sel ? ">" : " ", row_names[row], value);
        if (row == sel) {
            nio_color(console, NIO_COLOR_BLUE, NIO_COLOR_WHITE);
        }
        put_line(line);
        nio_color(console, NIO_COLOR_WHITE, NIO_COLOR_BLACK);
    }
    put_line("");
    put_line("Presets: 1 Performance  2 Balanced  3 Quality");
    const enum ConfigPreset preset = configfile_current_preset();
    snprintf(line, sizeof(line), "Now: %s", preset == PRESET_CUSTOM ? "Custom" : configPresetNames[preset]);
    put_line(line);
    put_line("");

    // help for the selected row, one console line per text line
    int lines = 0;
    for (const char *p = row_help[sel]; *p != '\0' && lines < 4; lines++) {
        const char *nl = strchr(p, '\n');
        const size_t n = nl != NULL ? (size_t) (nl - p) : strlen(p);
        snprintf(line, sizeof(line), "%.*s", (int) n, p);
        nio_color(console, NIO_COLOR_WHITE, NIO_COLOR_GRAY);
        put_line(line);
        p += n + (nl != NULL);
    }
    nio_color(console, NIO_COLOR_WHITE, NIO_COLOR_BLACK);
    for (; lines < 4; lines++) {
        put_line("");
    }
    put_line("");
    put_line("Enter: save and play        Esc: quit");
    if (status != NULL) {
        nio_color(console, NIO_COLOR_WHITE, NIO_COLOR_RED);
        put_line(status);
        nio_color(console, NIO_COLOR_WHITE, NIO_COLOR_BLACK);
    }
}

bool settings_menu_run(void) {
    nio_console console;
    if (!nio_init(&console, NIO_MAX_COLS, NIO_MAX_ROWS, 0, 0, NIO_COLOR_WHITE, NIO_COLOR_BLACK, true)) {
        return true; // no console: just play with the settings from the file
    }
    nio_set_default(&console);

    // remember the settings as loaded, to save only when something changed
    const bool old_low_res = config120pMode, old_sky = configDrawSky, old_fog = configEnableFog;
    const bool old_health = configCheatHealth, old_lives = configCheatLives;
    const bool old_moon = configCheatMoonJump;
    const unsigned int old_frameskip = configFrameskip;

    int sel = 0;
    bool play = true;
    const char *status = NULL;
    wait_no_key_pressed(); // the key that started the game
    while (true) {
        draw(&console, sel, status);
        status = NULL;
        wait_key_pressed();
        if (isKeyPressed(KEY_NSPIRE_ENTER) || isKeyPressed(KEY_NSPIRE_RET)) {
            const bool changed = config120pMode != old_low_res || configDrawSky != old_sky
                || configEnableFog != old_fog || configFrameskip != old_frameskip
                || configCheatHealth != old_health || configCheatLives != old_lives
                || configCheatMoonJump != old_moon;
            if (changed && !configfile_save(CONFIG_FILE)) {
                // still play with the new settings; they just won't be there next time
                draw(&console, sel, "Could not save the settings. Press a key to play.");
                wait_no_key_pressed();
                wait_key_pressed();
            }
            break;
        }
        if (isKeyPressed(KEY_NSPIRE_ESC)) {
            play = false;
            break;
        }
        if (isKeyPressed(KEY_NSPIRE_UP)) {
            sel = (sel + ROW_COUNT - 1) % ROW_COUNT;
        } else if (isKeyPressed(KEY_NSPIRE_DOWN)) {
            sel = (sel + 1) % ROW_COUNT;
        } else if (isKeyPressed(KEY_NSPIRE_LEFT)) {
            row_change(sel, -1);
        } else if (isKeyPressed(KEY_NSPIRE_RIGHT)) {
            row_change(sel, 1);
        } else if (isKeyPressed(KEY_NSPIRE_1)) {
            configfile_apply_preset(PRESET_PERFORMANCE);
        } else if (isKeyPressed(KEY_NSPIRE_2)) {
            configfile_apply_preset(PRESET_BALANCED);
        } else if (isKeyPressed(KEY_NSPIRE_3)) {
            configfile_apply_preset(PRESET_QUALITY);
        }
        wait_no_key_pressed();
    }
    wait_no_key_pressed(); // don't let Enter also press Start in the game
    nio_free(&console);
    return play;
}

#else

bool settings_menu_run(void) {
    return true;
}

#endif
