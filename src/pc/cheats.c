// cheats.c - optional cheats, switched on in the config file or on the settings screen
#include <PR/ultratypes.h>

#include "sm64.h"
#include "types.h"
#include "cheats.h"
#include "configfile.h"

#define FULL_HEALTH 0x880 // 8 segments of the power meter
#define MOON_JUMP_SPEED 25.0f // upward speed while A is held in the air
#define KEEP_CAP_TIMER 300 // frames (10 s); caps run out when the timer reaches 0

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
    // Only while the cap is on Mario's head and not in a cutscene (the ending takes the wing cap
    // away by setting a short timer). Grabbing a star sets the timer to 1 so the cap ends that
    // frame: this must keep running before mario_process_interactions, as it does now.
    if (configCheatKeepCaps && (m->flags & (MARIO_WING_CAP | MARIO_METAL_CAP | MARIO_VANISH_CAP))
        && (m->flags & MARIO_CAP_ON_HEAD) && (m->action & ACT_GROUP_MASK) != ACT_GROUP_CUTSCENE
        && m->capTimer > 0 && m->capTimer < KEEP_CAP_TIMER) {
        m->capTimer = KEEP_CAP_TIMER; // never reaches the last 2 seconds, where the cap runs out
    }
    if (configCheatMoonJump && (m->action & ACT_FLAG_AIR) && m->controller != NULL
        && (m->controller->buttonDown & A_BUTTON)) {
        m->vel[1] = MOON_JUMP_SPEED;
    }
#endif
}
