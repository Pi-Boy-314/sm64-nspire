#ifndef SETTINGS_MENU_H
#define SETTINGS_MENU_H

#include <stdbool.h>

// Start screen on the calculator: controls, settings, presets and cheats. Saves the config file
// if anything changed. Returns false if the player chose to quit. Other builds: returns true.
bool settings_menu_run(void);
// Same screen during play (settings key). Enter keeps the changes (and saves them), Esc undoes
// them. Esc during play still quits the game.
void settings_menu_run_paused(void);

#endif
