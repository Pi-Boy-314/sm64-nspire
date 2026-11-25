#ifndef CHEATS_H
#define CHEATS_H

struct MarioState;

// Applies the cheats switched on in the config file. Called once per frame from
// execute_mario_action, after Mario's inputs are read. Does nothing in benchmark builds, so the
// demos play the same as without cheats.
void cheats_update(struct MarioState *m);

#endif
