#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include <stdbool.h>

#define CONFIG_FILE "sm64_config.txt.tns"

extern bool         configDrawSky;
extern bool         configFiltering; // not in the config file: the renderer has no linear filtering
extern bool         configEnableFog;
extern bool         config120pMode;
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

// Ready-made combinations of the settings above (resolution, frameskip, sky, fog).
enum ConfigPreset {
    PRESET_PERFORMANCE,
    PRESET_BALANCED, // the defaults
    PRESET_QUALITY,
    PRESET_COUNT,
    PRESET_CUSTOM = PRESET_COUNT, // the current settings match no preset
};

extern const char *const configPresetNames[PRESET_COUNT];

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
