// cheats.c - optional cheats, switched on in the config file or on the settings screen
#include <PR/ultratypes.h>

#include "sm64.h"
#include "types.h"
#include "cheats.h"
#include "configfile.h"

#define FULL_HEALTH 0x880 // 8 segments of the power meter
#define MOON_JUMP_SPEED 25.0f // upward speed while A is held in the air

void cheats_update(struct MarioState *m) {
#ifdef BENCHMARK
    (void) m;
#else
    if (configCheatHealth && m->health > 0xFF) { // > 0xFF: don't undo a death already under way
        m->health = FULL_HEALTH;
        m->hurtCounter = 0;
    }
    if (configCheatLives && m->numLives < 99) {
        m->numLives = 99;
    }
    if (configCheatMoonJump && (m->action & ACT_FLAG_AIR) && m->controller != NULL
        && (m->controller->buttonDown & A_BUTTON)) {
        m->vel[1] = MOON_JUMP_SPEED;
    }
#endif
}
