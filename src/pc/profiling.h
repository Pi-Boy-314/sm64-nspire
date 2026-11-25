#ifndef PROFILING_H
#define PROFILING_H

#include <stdint.h>

// Per-frame time breakdown, in raw timer ticks (see tmr_ticks_per_sec()).
//
// One game tick (produce_one_frame) is split into consecutive "laps": each
// prof_lap(b) charges the time since the previous lap to bucket b.
//   PROF_LOGIC   level script, objects, Mario, collision, camera (+ misc glue)
//   PROF_SCENE   render_game(): geo graph traversal, matrices, display list build
//   PROF_GFX     gfx_run(): display list interpretation, vertex transform, rasterization
//   PROF_PRESENT gfx_end_frame(): colour conversion, 120p upscale, lcd_blit
// Skipped frames (frameskip) still pay LOGIC + SCENE; only GFX and PRESENT are skipped.
enum ProfBucket {
    PROF_LOGIC,
    PROF_SCENE,
    PROF_GFX,
    PROF_PRESENT,
    PROF_NUM_BUCKETS
};

struct ProfFrame {
    uint32_t ticks[PROF_NUM_BUCKETS];
    uint32_t raster_ticks; // portion of PROF_GFX spent in the backend triangle rasterizer
    uint32_t tris;
};

extern struct ProfFrame gProfFrame; // the frame in progress / most recently finished frame

void prof_frame_begin(void);
void prof_lap(enum ProfBucket bucket);
void prof_raster_begin(void);
void prof_raster_end(uint32_t tris);

uint32_t prof_ticks_to_us(uint64_t ticks);

#endif
