// configfile.c - loads and saves the settings file (CONFIG_FILE, plain text)
//
// The file is meant to be edited by hand on a computer as well as from the settings screen on
// the calculator, so it explains every setting. It is only rewritten when it is missing, when it
// comes from an older version of the game (no header line), or when the settings screen saves.
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "configfile.h"
#include "controls.h"

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

// Changing this makes the game rewrite (and so re-explain) files from older versions on first
// start, keeping their values; v2 added resolution 80x60, draw_shadows and flat_colors;
// v3 added sharp_hud and smooth_textures; v4 grouped the settings and merged flat_colors and
// smooth_textures into textures; v5 replaced sharp_hud with sharp_text (adds menu screens).
#define CONFIG_HEADER "# Super Mario 64 for TI-Nspire: settings v5"

/*
 * Settings and their defaults (the Balanced preset; everything else off)
 */
unsigned int configFrameskip     = FRAMESKIP_AUTO; // most frames in a row skipped to keep game speed
bool configShowFps               = false;
unsigned int configResolution    = RES_160x120;
unsigned int configTextures      = TEXTURES_NORMAL;
bool configDrawSky               = true;
bool configEnableFog             = false;
bool configDrawShadows           = true;
unsigned int configSharpText    = SHARP_TEXT_HUD_AND_MENUS;

bool configCheatHealth           = false;
bool configCheatLives            = false;
bool configCheatMoonJump         = false;
bool configCheatSuperSpeed       = false;
bool configCheatKeepCaps         = false;
bool configCheatSkipIntro        = false;
bool configCheatExitCourse       = false;

#ifdef BENCHMARK
unsigned int configBenchDemo     = 0;   // which attract-mode demo to play (wraps if too large)
unsigned int configBenchWarmup   = 30;  // ticks rendered after the demo starts but not measured
unsigned int configBenchTicks    = 600; // max ticks measured (stops earlier if the demo ends)
bool         configBenchDump     = false; // write a few frames as .ppm.tns files for visual diffs
#endif

/*
 * Presets
 */
const char *const configPresetNames[PRESET_COUNT] = {
    "Ultra Performance", "Performance", "Balanced", "Quality", "Ultra Quality",
};

// Fastest to best-looking. All use auto frameskip: full game speed, skipping drawing as needed.
static const struct {
    unsigned int res, frameskip, textures;
    unsigned int sharp_text;
    bool sky, fog, shadows;
} presets[PRESET_COUNT] = {
    [PRESET_ULTRA_PERFORMANCE] = { RES_80x60,   FRAMESKIP_AUTO, TEXTURES_FLAT,
                                   .sky = false, .fog = false, .shadows = false, .sharp_text = SHARP_TEXT_HUD_AND_MENUS },
    [PRESET_PERFORMANCE]       = { RES_160x120, FRAMESKIP_AUTO, TEXTURES_NORMAL,
                                   .sky = false, .fog = false, .shadows = true,  .sharp_text = SHARP_TEXT_HUD_AND_MENUS },
    [PRESET_BALANCED]          = { RES_160x120, FRAMESKIP_AUTO, TEXTURES_NORMAL,
                                   .sky = true,  .fog = false, .shadows = true,  .sharp_text = SHARP_TEXT_HUD_AND_MENUS },
    [PRESET_QUALITY]           = { RES_320x240, FRAMESKIP_AUTO, TEXTURES_NORMAL,
                                   .sky = true,  .fog = true,  .shadows = true,  .sharp_text = SHARP_TEXT_HUD_AND_MENUS },
    [PRESET_ULTRA_QUALITY]     = { RES_320x240, FRAMESKIP_AUTO, TEXTURES_SMOOTH,
                                   .sky = true,  .fog = true,  .shadows = true,  .sharp_text = SHARP_TEXT_HUD_AND_MENUS },
};

static const char *const res_names[RES_COUNT] = { "320x240", "160x120", "80x60" };
static const char *const textures_names[TEXTURES_COUNT] = { "flat", "normal", "smooth" };
static const char *const sharp_text_names[SHARP_TEXT_COUNT] = { "off", "hud", "hud_and_menus" };

const char *configfile_resolution_name(unsigned int res) {
    return res < RES_COUNT ? res_names[res] : "?";
}

void configfile_apply_preset(enum ConfigPreset preset) {
    if (preset >= PRESET_COUNT) {
        return;
    }
    configResolution = presets[preset].res;
    configFrameskip = presets[preset].frameskip;
    configTextures = presets[preset].textures;
    configDrawSky = presets[preset].sky;
    configEnableFog = presets[preset].fog;
    configDrawShadows = presets[preset].shadows;
    configSharpText = presets[preset].sharp_text;
}

enum ConfigPreset configfile_current_preset(void) {
    for (int p = 0; p < PRESET_COUNT; p++) {
        if (configResolution == presets[p].res && configFrameskip == presets[p].frameskip
            && configDrawSky == presets[p].sky && configEnableFog == presets[p].fog
            && configDrawShadows == presets[p].shadows && configTextures == presets[p].textures
            && configSharpText == presets[p].sharp_text) {
            return (enum ConfigPreset) p;
        }
    }
    return PRESET_CUSTOM;
}

void configfile_reset_defaults(void) {
    configfile_apply_preset(PRESET_BALANCED);
    configShowFps = false;
    configCheatHealth = configCheatLives = configCheatMoonJump = false;
    configCheatSuperSpeed = configCheatKeepCaps = configCheatSkipIntro = false;
    configCheatExitCourse = false;
    controls_reset();
}

/*
 * The settings in the file, in the order they are written. help is written above each one,
 * section (if any) as a heading before it.
 */
enum ConfigOptionType {
    CONFIG_TYPE_BOOL,
    CONFIG_TYPE_UINT,
    CONFIG_TYPE_FRAMESKIP, // a number up to FRAMESKIP_MAX, or "auto"
    CONFIG_TYPE_CHOICE,    // one of the words in choices; the value is its index
};

struct ConfigOption {
    const char *name;
    enum ConfigOptionType type;
    union {
        bool *boolValue;
        unsigned int *uintValue;
    };
    unsigned int max; // largest value accepted (numbers), or number of choices
    const char *const *choices; // CONFIG_TYPE_CHOICE
    const char *section;
    const char *help;
};

static const struct ConfigOption options[] = {
    {.name = "frameskip", .type = CONFIG_TYPE_FRAMESKIP, .uintValue = &configFrameskip,
     .max = FRAMESKIP_MAX, .section = "SPEED",
     .help = "Frame skipping. The game runs 30 times a second, but the calculator\n"
             "usually can't draw that often. Skipping the drawing of some frames\n"
             "keeps the game at its real speed.\n"
             "auto (recommended): skip only as many frames as needed to keep full\n"
             "speed, at most 10 in a row.\n"
             "0: never skip. Every frame is drawn, but the game runs in slow motion\n"
             "whenever the calculator can't keep up.\n"
             "1 to 30: skip at most that many frames in a row."},
#ifndef BENCHMARK
    {.name = "show_fps", .type = CONFIG_TYPE_BOOL, .boolValue = &configShowFps,
     .help = "true: show a counter in the bottom-left corner with the frames drawn\n"
             "per second (FPS) and the game speed (100% = full speed, lower = slow\n"
             "motion). Not part of the presets."},
#endif
    {.name = "resolution", .type = CONFIG_TYPE_CHOICE, .uintValue = &configResolution,
     .choices = res_names, .max = RES_COUNT,
     .section = "PICTURE",
     .help = "Resolution of the 3D world; lower ones are stretched to fill the\n"
             "screen. (The HUD and text follow sharp_text below.)\n"
             "320x240: full resolution. Sharp, but much slower.\n"
             "160x120: much faster (roughly 2 to 3 times), but blocky.\n"
             "80x60: fastest. Very blocky."},
    {.name = "textures", .type = CONFIG_TYPE_CHOICE, .uintValue = &configTextures,
     .choices = textures_names, .max = TEXTURES_COUNT,
     .help = "normal: textures as in the original, with sharp square pixels.\n"
             "smooth: filtered textures like on the N64. Much slower.\n"
             "flat: walls, floors and other solid surfaces in one plain colour each\n"
             "(the average colour of their texture). Much faster; looks like a very\n"
             "early 3D game. Trees, fences, coins, text and the sky stay textured."},
    {.name = "draw_sky", .type = CONFIG_TYPE_BOOL, .boolValue = &configDrawSky,
     .help = "true: draw the sky backgrounds.\n"
             "false: plain black sky. Faster, because the sky covers a large part\n"
             "of the screen in many levels."},
    {.name = "enable_fog", .type = CONFIG_TYPE_BOOL, .boolValue = &configEnableFog,
     .help = "true: distance fog in the levels that use it. Closer to the original\n"
             "look, but fogged areas draw more slowly.\n"
             "false: no fog."},
    {.name = "draw_shadows", .type = CONFIG_TYPE_BOOL, .boolValue = &configDrawShadows,
     .help = "true: Mario, enemies and items cast shadows.\n"
             "false: no shadows. A little faster, but jumps are harder to judge."},
    {.name = "sharp_text", .type = CONFIG_TYPE_CHOICE, .uintValue = &configSharpText,
     .choices = sharp_text_names, .max = SHARP_TEXT_COUNT,
     .help = "Keeps text readable when the 3D world uses a lower resolution.\n"
             "hud_and_menus: the HUD (lives, coins, stars), dialog boxes and the\n"
             "pause menu are drawn at the full 320x240, and so are whole menu\n"
             "screens without Mario (title, file select, star select), which also\n"
             "use normal textures. Menus are simple, so they stay smooth.\n"
             "hud: only the HUD, dialog boxes and pause menu.\n"
             "off: everything at the 3D world's resolution. A little faster.\n"
             "No effect at 320x240."},
#ifndef BENCHMARK
    {.name = "cheat_infinite_health", .type = CONFIG_TYPE_BOOL, .boolValue = &configCheatHealth,
     .section = "CHEATS",
     .help = "All cheats are off (false) by default. They don't change your save\n"
             "file, but stars and coins you collect while using them are saved as\n"
             "usual.\n"
             "\n"
             "true: Mario's health refills every frame, so enemies, fire, falls and\n"
             "running out of air can't hurt him. Bottomless pits still cost a life."},
    {.name = "cheat_infinite_lives", .type = CONFIG_TYPE_BOOL, .boolValue = &configCheatLives,
     .help = "true: the life counter stays at 99, so you never get a game over."},
    {.name = "cheat_moon_jump", .type = CONFIG_TYPE_BOOL, .boolValue = &configCheatMoonJump,
     .help = "true: hold the A button in the air and Mario keeps rising.\n"
             "Let go to fall."},
    {.name = "cheat_super_speed", .type = CONFIG_TYPE_BOOL, .boolValue = &configCheatSuperSpeed,
     .help = "true: Mario covers twice the ground when walking, running or sliding."},
    {.name = "cheat_keep_caps", .type = CONFIG_TYPE_BOOL, .boolValue = &configCheatKeepCaps,
     .help = "true: the wing, metal and vanish caps don't run out until you leave\n"
             "the level or lose the cap."},
    {.name = "cheat_skip_intro", .type = CONFIG_TYPE_BOOL, .boolValue = &configCheatSkipIntro,
     .help = "true: a new save file starts right outside the castle, without\n"
             "Peach's letter and Lakitu's flight."},
    {.name = "cheat_exit_course_anytime", .type = CONFIG_TYPE_BOOL,
     .boolValue = &configCheatExitCourse,
     .help = "true: the pause menu always offers \"Exit course\", even in mid-air,\n"
             "swimming or anywhere the game normally doesn't allow it."},
#endif
#ifdef BENCHMARK
    {.name = "bench_demo", .type = CONFIG_TYPE_UINT, .uintValue = &configBenchDemo, .max = 255,
     .section = "BENCHMARK",
     .help = "Benchmark build only: which attract-mode demo to play (0 to 3)."},
    {.name = "bench_warmup", .type = CONFIG_TYPE_UINT, .uintValue = &configBenchWarmup,
     .max = 100000,
     .help = "Benchmark build only: frames played before measuring starts."},
    {.name = "bench_ticks", .type = CONFIG_TYPE_UINT, .uintValue = &configBenchTicks,
     .max = 100000,
     .help = "Benchmark build only: most frames measured (stops early when the\n"
             "demo ends)."},
    {.name = "bench_dump_frames", .type = CONFIG_TYPE_BOOL, .boolValue = &configBenchDump,
     .help = "Benchmark build only: true saves frames 60, 240 and 420 as pictures\n"
             "(.ppm.tns) for comparing versions."},
#endif
};

static const char *const intro =
    "#\n"
    "# You can change these on the calculator: start the game, and the first\n"
    "# screen shows the settings (arrow keys to change, tab for the next page,\n"
    "# Enter to save and play). During play, the settings key (tab) opens the\n"
    "# same screen.\n"
    "#\n"
    "# Or edit this file on a computer: copy it from the calculator with the\n"
    "# TI-Nspire software (or TI-Nspire Computer Link), open it in any plain text\n"
    "# editor (Notepad, TextEdit, ...), save it, and send it back into the same\n"
    "# folder as the game. Keep the name sm64_config.txt.tns.\n"
    "#\n"
    "# Each setting is one line: its name, a space, then its value. Lines that\n"
    "# start with # are notes for you and are ignored. A missing or misspelt\n"
    "# setting keeps its default value.\n"
    "#\n";

// "true"/"false" and a few friendlier spellings
static bool parse_bool(const char *s, bool *out) {
    static const char *const yes[] = { "true", "on", "yes", "1" };
    static const char *const no[] = { "false", "off", "no", "0" };
    for (unsigned int i = 0; i < ARRAY_LEN(yes); i++) {
        if (strcmp(s, yes[i]) == 0) {
            *out = true;
            return true;
        }
        if (strcmp(s, no[i]) == 0) {
            *out = false;
            return true;
        }
    }
    return false;
}

static bool parse_uint(const char *s, unsigned int max, unsigned int *out) {
    if (!isdigit((unsigned char) *s)) {
        return false;
    }
    char *end;
    const unsigned long v = strtoul(s, &end, 10);
    if (*end != '\0') {
        return false;
    }
    *out = v > max ? max : (unsigned int) v;
    return true;
}

// Splits off the next word of *s (lower-cased), or returns NULL at the end of the line.
static char *next_word(char **s) {
    char *p = *s;
    while (isspace((unsigned char) *p)) {
        p++;
    }
    if (*p == '\0') {
        return NULL;
    }
    char *word = p;
    while (*p != '\0' && !isspace((unsigned char) *p)) {
        *p = tolower((unsigned char) *p);
        p++;
    }
    if (*p != '\0') {
        *p++ = '\0';
    }
    *s = p;
    return word;
}

// One line of the file. Comments and unknown settings (such as the numeric key_* lines and
// texture_filtering that older versions wrote) are ignored.
static void parse_line(char *line, bool *has_header) {
    char *p = line;
    while (isspace((unsigned char) *p)) {
        p++;
    }
    if (strncmp(p, CONFIG_HEADER, strlen(CONFIG_HEADER)) == 0) {
        *has_header = true;
    }
    char *hash = strchr(p, '#');
    if (hash != NULL) {
        *hash = '\0';
    }
    const char *name = next_word(&p);
    const char *value = next_word(&p);
    if (name == NULL || value == NULL) {
        return;
    }
    // settings from older versions, now part of textures
    if (strcmp(name, "flat_colors") == 0 || strcmp(name, "smooth_textures") == 0) {
        bool on;
        if (parse_bool(value, &on) && on) {
            configTextures = name[0] == 'f' ? TEXTURES_FLAT : TEXTURES_SMOOTH;
        }
        return;
    }
    if (strcmp(name, "sharp_hud") == 0) { // v3: true/false, now sharp_text
        bool on;
        if (parse_bool(value, &on)) {
            configSharpText = on ? SHARP_TEXT_HUD_AND_MENUS : SHARP_TEXT_OFF;
        }
        return;
    }
    if (strcmp(name, "enable_120p_mode") == 0) { // older files: true = 160x120, false = 320x240
        bool low;
        if (parse_bool(value, &low)) {
            configResolution = low ? RES_160x120 : RES_320x240;
        }
        return;
    }
    for (unsigned int i = 0; i < ARRAY_LEN(options); i++) {
        const struct ConfigOption *o = &options[i];
        if (strcmp(name, o->name) == 0) {
            if (o->type == CONFIG_TYPE_BOOL) {
                parse_bool(value, o->boolValue);
            } else if (o->type == CONFIG_TYPE_FRAMESKIP && strcmp(value, "auto") == 0) {
                *o->uintValue = FRAMESKIP_AUTO;
            } else if (o->type == CONFIG_TYPE_CHOICE) {
                for (unsigned int k = 0; k < o->max; k++) {
                    if (strcmp(value, o->choices[k]) == 0) {
                        *o->uintValue = k;
                    }
                }
            } else {
                parse_uint(value, o->max, o->uintValue);
            }
            return;
        }
    }
#ifndef BENCHMARK
    for (int c = 0; c < CONTROL_COUNT; c++) {
        if (strcmp(name, controls_config_name(c)) == 0) {
            const int key = controls_key_by_name(value);
            if (key != KEY_NONE && !controls_key_reserved(key)) {
                controlKeys[c] = key;
            }
            return;
        }
    }
#endif
}

void configfile_load(const char *filename) {
    controls_reset(); // keys missing from the file keep their defaults

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        configfile_save(filename); // first start: write the defaults with their explanations
        return;
    }

    bool has_header = false;
    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        const size_t len = strlen(line);
        if (len == sizeof(line) - 1 && line[len - 1] != '\n') {
            // longer than the buffer: parse the start, skip the rest
            int c;
            while ((c = fgetc(file)) != EOF && c != '\n') {
            }
        }
        parse_line(line, &has_header);
    }
    fclose(file);

#ifndef BENCHMARK
    // the settings key must be a key of its own, or pressing that button would open the screen
    for (int c = 0; c < CONTROL_COUNT; c++) {
        if (c != CONTROL_SETTINGS && controlKeys[c] == controlKeys[CONTROL_SETTINGS]) {
            controlKeys[CONTROL_SETTINGS] = controls_key_by_name("tab");
            for (int d = 0; d < CONTROL_COUNT; d++) {
                if (d != CONTROL_SETTINGS && controlKeys[d] == controlKeys[CONTROL_SETTINGS]) {
                    controlKeys[CONTROL_SETTINGS] = KEY_NONE; // tab is taken too: no settings key
                }
            }
            break;
        }
    }
#endif

    if (!has_header) {
        configfile_save(filename); // file from an older version: same values, now explained
    }
}

// writes text as comment lines ("# " in front, just "#" for empty lines)
static void write_comment(FILE *file, const char *text) {
    while (*text != '\0') {
        const char *nl = strchr(text, '\n');
        const size_t n = nl != NULL ? (size_t) (nl - text) : strlen(text);
        fprintf(file, n > 0 ? "# %.*s\n" : "#\n", (int) n, text);
        text += n + (nl != NULL);
    }
}

static void write_section(FILE *file, const char *title) {
    fprintf(file, "\n\n# ===== %s =====\n", title);
}

static void frameskip_text(unsigned int v, char *buf, size_t size) {
    if (v == FRAMESKIP_AUTO) {
        snprintf(buf, size, "auto");
    } else {
        snprintf(buf, size, "%u", v);
    }
}

static void write_presets(FILE *file) {
    // made from the presets above so it can't go out of date
    fputs("# Ready-made presets, fastest first (on the settings screen: keys 1 to 5):\n#\n", file);
    for (int p = 0; p < PRESET_COUNT; p++) {
        char fs[16];
        frameskip_text(presets[p].frameskip, fs, sizeof(fs));
        fprintf(file, "#   %d %s\n", p + 1, configPresetNames[p]);
        fprintf(file, "#       frameskip %s  resolution %s  textures %s  draw_sky %s\n", fs,
                res_names[presets[p].res], textures_names[presets[p].textures],
                presets[p].sky ? "true" : "false");
        fprintf(file, "#       enable_fog %s  draw_shadows %s  sharp_text %s\n",
                presets[p].fog ? "true" : "false", presets[p].shadows ? "true" : "false",
                sharp_text_names[presets[p].sharp_text]);
    }
    fputs("#\n# Balanced is the default. Ultra Performance gets the most speed whatever\n"
          "# it looks like; Ultra Quality the best picture whatever the speed.\n"
          "# Any mix of values works too.\n", file);
}
#ifndef BENCHMARK
static void write_controls(FILE *file) {
    write_section(file, "CONTROLS");
    fputs("# The calculator key for each N64 button. esc (quit) and ctrl (speed\n"
          "# statistics) are fixed and can't be used. If two buttons share a key,\n"
          "# that key presses both, but key_settings must have a key of its own.\n"
          "# The arrow keys include the touchpad diagonals.\n"
          "# Key names:\n", file);
    char line[80];
    int n = snprintf(line, sizeof(line), "#  ");
    for (int k = 0; k < controls_key_count(); k++) {
        if (controls_key_reserved(k)) {
            continue;
        }
        const char *name = controls_key_name(k);
        if (n + 1 + (int) strlen(name) > 76) {
            fprintf(file, "%s\n", line);
            n = snprintf(line, sizeof(line), "#  ");
        }
        n += snprintf(line + n, sizeof(line) - n, " %s", name);
    }
    fprintf(file, "%s\n\n", line);
    for (int c = 0; c < CONTROL_COUNT; c++) {
        fprintf(file, "%-16s %s\n", controls_config_name(c), controls_key_name(controlKeys[c]));
    }
}
#endif

bool configfile_save(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        return false;
    }

    fputs(CONFIG_HEADER "\n", file);
    fputs(intro, file);
    write_presets(file);

    for (unsigned int i = 0; i < ARRAY_LEN(options); i++) {
        const struct ConfigOption *o = &options[i];
        if (o->section != NULL) {
            write_section(file, o->section);
        }
        fputc('\n', file);
        write_comment(file, o->help);
        char value[16];
        switch (o->type) {
            case CONFIG_TYPE_BOOL:
                snprintf(value, sizeof(value), "%s", *o->boolValue ? "true" : "false");
                break;
            case CONFIG_TYPE_FRAMESKIP:
                frameskip_text(*o->uintValue, value, sizeof(value));
                break;
            case CONFIG_TYPE_CHOICE:
                snprintf(value, sizeof(value), "%s", o->choices[*o->uintValue % o->max]);
                break;
            default:
                snprintf(value, sizeof(value), "%u", *o->uintValue);
                break;
        }
        fprintf(file, "%s %s\n", o->name, value);
    }
#ifndef BENCHMARK
    write_controls(file);
#endif

    const bool ok = !ferror(file);
    return fclose(file) == 0 && ok;
}
