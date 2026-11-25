// controls.c - configurable keys for the N64 buttons
#include <string.h>

#include "controls.h"

#ifndef BENCHMARK
#include <libndls.h>
#endif

int controlKeys[CONTROL_COUNT];

static const struct {
    const char *name, *config_name, *default_key;
} controls[CONTROL_COUNT] = {
    [CONTROL_A]           = { "A button",    "key_a",           "menu" },
    [CONTROL_B]           = { "B button",    "key_b",           "del" },
    [CONTROL_Z]           = { "Z trigger",   "key_z",           "doc" },
    [CONTROL_R]           = { "R trigger",   "key_r",           "shift" },
    [CONTROL_START]       = { "Start",       "key_start",       "enter" },
    [CONTROL_C_UP]        = { "C up",        "key_c_up",        "8" },
    [CONTROL_C_DOWN]      = { "C down",      "key_c_down",      "2" },
    [CONTROL_C_LEFT]      = { "C left",      "key_c_left",      "4" },
    [CONTROL_C_RIGHT]     = { "C right",     "key_c_right",     "6" },
    [CONTROL_STICK_UP]    = { "Stick up",    "key_stick_up",    "up" },
    [CONTROL_STICK_DOWN]  = { "Stick down",  "key_stick_down",  "down" },
    [CONTROL_STICK_LEFT]  = { "Stick left",  "key_stick_left",  "left" },
    [CONTROL_STICK_RIGHT] = { "Stick right", "key_stick_right", "right" },
    [CONTROL_SETTINGS]    = { "Settings",    "key_settings",    "tab" },
};

#ifdef BENCHMARK
// benchmark builds (calculator, host, QEMU) play demo inputs; no keys
#define KEYS(...) 0
#else
#define KEYS(...) { __VA_ARGS__ }
#endif

// Key names as used in the config file. Only keys that exist on both keypads (touchpad CX and
// clickpad CX II): theta and scratchpad are missing on one of them. The arrow entries include the touchpad diagonals, so
// the stick works in 8 directions as before.
static const struct {
    const char *name;
#ifndef BENCHMARK
    const t_key *keys[3];
#else
    int unused;
#endif
} keys[] = {
    { "esc", KEYS(&KEY_NSPIRE_ESC) },     { "ctrl", KEYS(&KEY_NSPIRE_CTRL) },
    { "up", KEYS(&KEY_NSPIRE_UP, &KEY_NSPIRE_LEFTUP, &KEY_NSPIRE_UPRIGHT) },
    { "down", KEYS(&KEY_NSPIRE_DOWN, &KEY_NSPIRE_RIGHTDOWN, &KEY_NSPIRE_DOWNLEFT) },
    { "left", KEYS(&KEY_NSPIRE_LEFT, &KEY_NSPIRE_LEFTUP, &KEY_NSPIRE_DOWNLEFT) },
    { "right", KEYS(&KEY_NSPIRE_RIGHT, &KEY_NSPIRE_UPRIGHT, &KEY_NSPIRE_RIGHTDOWN) },
    { "click", KEYS(&KEY_NSPIRE_CLICK) }, { "enter", KEYS(&KEY_NSPIRE_ENTER) },
    { "ret", KEYS(&KEY_NSPIRE_RET) },     { "menu", KEYS(&KEY_NSPIRE_MENU) },
    { "del", KEYS(&KEY_NSPIRE_DEL) },     { "doc", KEYS(&KEY_NSPIRE_DOC) },
    { "tab", KEYS(&KEY_NSPIRE_TAB) },     { "shift", KEYS(&KEY_NSPIRE_SHIFT) },
    { "var", KEYS(&KEY_NSPIRE_VAR) },     { "cat", KEYS(&KEY_NSPIRE_CAT) },
    { "0", KEYS(&KEY_NSPIRE_0) }, { "1", KEYS(&KEY_NSPIRE_1) }, { "2", KEYS(&KEY_NSPIRE_2) },
    { "3", KEYS(&KEY_NSPIRE_3) }, { "4", KEYS(&KEY_NSPIRE_4) }, { "5", KEYS(&KEY_NSPIRE_5) },
    { "6", KEYS(&KEY_NSPIRE_6) }, { "7", KEYS(&KEY_NSPIRE_7) }, { "8", KEYS(&KEY_NSPIRE_8) },
    { "9", KEYS(&KEY_NSPIRE_9) },
    { "a", KEYS(&KEY_NSPIRE_A) }, { "b", KEYS(&KEY_NSPIRE_B) }, { "c", KEYS(&KEY_NSPIRE_C) },
    { "d", KEYS(&KEY_NSPIRE_D) }, { "e", KEYS(&KEY_NSPIRE_E) }, { "f", KEYS(&KEY_NSPIRE_F) },
    { "g", KEYS(&KEY_NSPIRE_G) }, { "h", KEYS(&KEY_NSPIRE_H) }, { "i", KEYS(&KEY_NSPIRE_I) },
    { "j", KEYS(&KEY_NSPIRE_J) }, { "k", KEYS(&KEY_NSPIRE_K) }, { "l", KEYS(&KEY_NSPIRE_L) },
    { "m", KEYS(&KEY_NSPIRE_M) }, { "n", KEYS(&KEY_NSPIRE_N) }, { "o", KEYS(&KEY_NSPIRE_O) },
    { "p", KEYS(&KEY_NSPIRE_P) }, { "q", KEYS(&KEY_NSPIRE_Q) }, { "r", KEYS(&KEY_NSPIRE_R) },
    { "s", KEYS(&KEY_NSPIRE_S) }, { "t", KEYS(&KEY_NSPIRE_T) }, { "u", KEYS(&KEY_NSPIRE_U) },
    { "v", KEYS(&KEY_NSPIRE_V) }, { "w", KEYS(&KEY_NSPIRE_W) }, { "x", KEYS(&KEY_NSPIRE_X) },
    { "y", KEYS(&KEY_NSPIRE_Y) }, { "z", KEYS(&KEY_NSPIRE_Z) },
    { "space", KEYS(&KEY_NSPIRE_SPACE) },   { "period", KEYS(&KEY_NSPIRE_PERIOD) },
    { "comma", KEYS(&KEY_NSPIRE_COMMA) },   { "negative", KEYS(&KEY_NSPIRE_NEGATIVE) },
    { "plus", KEYS(&KEY_NSPIRE_PLUS) },     { "minus", KEYS(&KEY_NSPIRE_MINUS) },
    { "multiply", KEYS(&KEY_NSPIRE_MULTIPLY) }, { "divide", KEYS(&KEY_NSPIRE_DIVIDE) },
    { "equals", KEYS(&KEY_NSPIRE_EQU) },    { "power", KEYS(&KEY_NSPIRE_EXP) },
    { "square", KEYS(&KEY_NSPIRE_SQU) },    { "tenx", KEYS(&KEY_NSPIRE_TENX) },
    { "ee", KEYS(&KEY_NSPIRE_EE) },         { "pi", KEYS(&KEY_NSPIRE_PI) },
    { "trig", KEYS(&KEY_NSPIRE_TRIG) },     { "sin", KEYS(&KEY_NSPIRE_SIN) },
    { "cos", KEYS(&KEY_NSPIRE_COS) },       { "tan", KEYS(&KEY_NSPIRE_TAN) },
    { "lparen", KEYS(&KEY_NSPIRE_LP) },     { "rparen", KEYS(&KEY_NSPIRE_RP) },
    { "frac", KEYS(&KEY_NSPIRE_FRAC) },     { "flag", KEYS(&KEY_NSPIRE_FLAG) },
    { "question", KEYS(&KEY_NSPIRE_QUESEXCL) },
    { "colon", KEYS(&KEY_NSPIRE_COLON) },   { "quote", KEYS(&KEY_NSPIRE_QUOTE) },
    { "apostrophe", KEYS(&KEY_NSPIRE_APOSTROPHE) }, { "bar", KEYS(&KEY_NSPIRE_BAR) },
    { "greater", KEYS(&KEY_NSPIRE_GTHAN) }, { "less", KEYS(&KEY_NSPIRE_LTHAN) },
};

#define KEY_COUNT ((int) (sizeof(keys) / sizeof(keys[0])))

const char *controls_name(int control) {
    return control >= 0 && control < CONTROL_COUNT ? controls[control].name : "";
}

const char *controls_config_name(int control) {
    return control >= 0 && control < CONTROL_COUNT ? controls[control].config_name : "";
}

int controls_key_count(void) {
    return KEY_COUNT;
}

const char *controls_key_name(int key) {
    return key >= 0 && key < KEY_COUNT ? keys[key].name : "none";
}

int controls_key_by_name(const char *name) {
    for (int k = 0; k < KEY_COUNT; k++) {
        if (strcmp(keys[k].name, name) == 0) {
            return k;
        }
    }
    return KEY_NONE;
}

bool controls_key_reserved(int key) {
    return key >= 0 && key < KEY_COUNT
        && (strcmp(keys[key].name, "esc") == 0 || strcmp(keys[key].name, "ctrl") == 0);
}

void controls_reset(void) {
    for (int c = 0; c < CONTROL_COUNT; c++) {
        controlKeys[c] = controls_key_by_name(controls[c].default_key);
    }
}

#ifndef BENCHMARK
static bool key_down(int key) {
    if (key < 0 || key >= KEY_COUNT) {
        return false;
    }
    for (int i = 0; i < 3 && keys[key].keys[i] != NULL; i++) {
        if ((isKeyPressed)(keys[key].keys[i])) {
            return true;
        }
    }
    return false;
}

bool controls_held(int control) {
    return control >= 0 && control < CONTROL_COUNT && key_down(controlKeys[control]);
}

int controls_pressed_key(void) {
    for (int k = 0; k < KEY_COUNT; k++) {
        if (key_down(k)) {
            return k;
        }
    }
    return KEY_NONE;
}
#else
bool controls_held(int control) {
    (void) control;
    return false;
}

int controls_pressed_key(void) {
    return KEY_NONE;
}
#endif
