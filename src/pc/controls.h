#ifndef CONTROLS_H
#define CONTROLS_H

#include <stdbool.h>

// Configurable calculator keys for the N64 buttons (config file key_* lines and the Controls
// page of the settings screen). Esc (quit) and ctrl (speed statistics) are fixed.

enum Control {
    CONTROL_A,
    CONTROL_B,
    CONTROL_Z,
    CONTROL_R,
    CONTROL_START,
    CONTROL_C_UP,
    CONTROL_C_DOWN,
    CONTROL_C_LEFT,
    CONTROL_C_RIGHT,
    CONTROL_STICK_UP,
    CONTROL_STICK_DOWN,
    CONTROL_STICK_LEFT,
    CONTROL_STICK_RIGHT,
    CONTROL_SETTINGS, // opens the settings screen during play
    CONTROL_COUNT,
};

#define KEY_NONE (-1)

extern int controlKeys[CONTROL_COUNT]; // index into the key list, or KEY_NONE

const char *controls_name(int control);        // "A button"
const char *controls_config_name(int control); // "key_a"
void controls_reset(void);                     // default keys

int controls_key_count(void);
const char *controls_key_name(int key);        // "menu"
int controls_key_by_name(const char *name);    // KEY_NONE if unknown
bool controls_key_reserved(int key);           // esc and ctrl

// calculator only (other builds: never held / no key)
bool controls_held(int control);
int controls_pressed_key(void);                // first key in the list that is down, or KEY_NONE

#endif
