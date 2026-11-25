#ifndef SETTINGS_MENU_H
#define SETTINGS_MENU_H

#include <stdbool.h>

// Start screen on the calculator: controls, settings, presets and cheats. Saves the config file
// if anything changed. Returns false if the player chose to quit. Other builds: returns true.
bool settings_menu_run(void);
// Same screen during play (settings key). Enter keeps the changes (and saves them), Esc undoes
// them.
void settings_menu_run_paused(void);
// Speed info screen (ctrl during play): frame rate, time per stage, settings, play time.
// skipped: frames skipped just before the last drawn one. Waits for a key.
void settings_menu_speed_info(int skipped);
// Asks whether to quit (esc during play). Returns true on Enter. Other builds: true.
bool settings_menu_confirm_quit(void);

#endif
