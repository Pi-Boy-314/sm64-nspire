#include <libndls.h>

#include "macros.h"

#include "lib/src/libultra_internal.h"
#include "lib/src/osContInternal.h"

#include "controls.h"

// Keys are set in the config file / on the Controls page of the settings screen (controls.c).

s32 osContInit(UNUSED OSMesgQueue *mq, u8 *controllerBits, UNUSED OSContStatus *status) {
    *controllerBits = 1;
    return 0;
}

s32 osContStartReadData(UNUSED OSMesgQueue *mesg) {
    return 0;
}

void osContGetReadData(OSContPad *pad) {
    static const struct {
        int control;
        u16 button;
    } buttons[] = {
        { CONTROL_START, START_BUTTON },  { CONTROL_A, A_BUTTON },
        { CONTROL_B, B_BUTTON },          { CONTROL_Z, Z_TRIG },
        { CONTROL_R, R_TRIG },            { CONTROL_C_UP, U_CBUTTONS },
        { CONTROL_C_DOWN, D_CBUTTONS },   { CONTROL_C_LEFT, L_CBUTTONS },
        { CONTROL_C_RIGHT, R_CBUTTONS },
    };

    pad->button = 0;
    pad->stick_x = 0;
    pad->stick_y = 0;
    pad->errnum = 0;

    for (unsigned int i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
        if (controls_held(buttons[i].control)) {
            pad->button |= buttons[i].button;
        }
    }

    if (controls_held(CONTROL_STICK_LEFT))
        pad->stick_x = -128;
    if (controls_held(CONTROL_STICK_RIGHT))
        pad->stick_x = 127;
    if (controls_held(CONTROL_STICK_UP))
        pad->stick_y = 127;
    if (controls_held(CONTROL_STICK_DOWN))
        pad->stick_y = -128;
}
