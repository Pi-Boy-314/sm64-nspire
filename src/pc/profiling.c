#include <string.h>

#include "profiling.h"
#include "timer.h"

struct ProfFrame gProfFrame;

static uint32_t sLapMark;
static uint32_t sRasterMark;

void prof_frame_begin(void) {
    memset(&gProfFrame, 0, sizeof(gProfFrame));
    sLapMark = tmr_ticks();
}

void prof_lap(enum ProfBucket bucket) {
    uint32_t now = tmr_ticks();
    gProfFrame.ticks[bucket] += now - sLapMark;
    sLapMark = now;
}

void prof_raster_begin(void) {
    sRasterMark = tmr_ticks();
}

void prof_raster_end(uint32_t tris) {
    gProfFrame.raster_ticks += tmr_ticks() - sRasterMark;
    gProfFrame.tris += tris;
}

uint32_t prof_ticks_to_us(uint64_t ticks) {
    uint32_t tps = tmr_ticks_per_sec();
    return tps ? (uint32_t)(ticks * 1000000u / tps) : 0;
}
