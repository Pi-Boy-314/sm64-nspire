#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include <stdbool.h>

#define CONFIG_FILE "sm64_config.txt.tns"

extern bool         configDrawSky;
extern bool         configEnableFog;
// Drawing resolution: the screen's 320x240 divided by 1 << configResolution.
enum ConfigResolution { RES_320x240, RES_160x120, RES_80x60, RES_COUNT };
extern unsigned int configResolution;
// Textures: flat (solid surfaces in their texture's average colour), normal, or smooth
// (bilinear filtering where the game asks for it). Fastest to slowest.
enum ConfigTextures { TEXTURES_FLAT, TEXTURES_NORMAL, TEXTURES_SMOOTH, TEXTURES_COUNT };
extern unsigned int configTextures;
#define configFlatColors (configTextures == TEXTURES_FLAT)
#define configFiltering (configTextures == TEXTURES_SMOOTH)
extern bool         configDrawShadows;
// Readable text when the 3D world is below 320x240: off; hud (HUD, dialog boxes and pause menu
// at 320x240); hud_and_menus (also whole menu screens without Mario, with normal textures).
enum ConfigSharpText { SHARP_TEXT_OFF, SHARP_TEXT_HUD, SHARP_TEXT_HUD_AND_MENUS, SHARP_TEXT_COUNT };
extern unsigned int configSharpText;
#define configSharpHud (configSharpText != SHARP_TEXT_OFF)
extern unsigned int configFrameskip;
extern bool         configShowFps;
// cheats (off by default; never written to the save file, and ignored by benchmark builds)
extern bool         configCheatHealth;
extern bool         configCheatLives;
extern bool         configCheatMoonJump;
extern bool         configCheatSuperSpeed;
extern bool         configCheatKeepCaps;
extern bool         configCheatSkipIntro;
extern bool         configCheatExitCourse;
#ifdef BENCHMARK
extern unsigned int configBenchDemo;
extern unsigned int configBenchWarmup;
extern unsigned int configBenchTicks;
extern bool         configBenchDump;
#endif

#define FRAMESKIP_MAX 30
#define FRAMESKIP_AUTO 0xFFFFu   // configFrameskip value for "auto"
#define FRAMESKIP_AUTO_MAX 10    // most frames auto skips in a row

// Ready-made combinations of the settings above (resolution, frameskip, sky, fog, shadows,
// flat colours).
enum ConfigPreset { // fastest first; keys 1 to 5 on the settings screen
    PRESET_ULTRA_PERFORMANCE, // fastest possible, looks be damned
    PRESET_PERFORMANCE,
    PRESET_BALANCED, // the defaults
    PRESET_QUALITY,
    PRESET_ULTRA_QUALITY, // best picture, speed be damned
    PRESET_COUNT,
    PRESET_CUSTOM = PRESET_COUNT, // the current settings match no preset
};

extern const char *const configPresetNames[PRESET_COUNT];

const char *configfile_resolution_name(unsigned int res); // "320x240", ...
void configfile_apply_preset(enum ConfigPreset preset);
enum ConfigPreset configfile_current_preset(void);
// Every setting, cheat and key back to its default.
void configfile_reset_defaults(void);

// Loads the settings. Writes a fresh, commented file when it is missing or from an older
// version (keeping the values that were read); otherwise the file is left as the user wrote it.
void configfile_load(const char *filename);
// Writes all settings with an explanation of each. Returns false if the file can't be written.
bool configfile_save(const char *filename);

#endif
