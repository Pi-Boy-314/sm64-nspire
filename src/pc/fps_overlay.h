#ifndef FPS_OVERLAY_H
#define FPS_OVERLAY_H

#include <stdbool.h>
#include <stdint.h>

// FPS counter in the bottom-left corner (config option show_fps).

// Call once per game tick, drawn or skipped.
void fps_overlay_tick(void);
// Call once per drawn frame with the 320x240 RGB565 picture about to be shown.
void fps_overlay_draw(uint16_t *screen);
// The last full measurement window (about a second, measured even with the counter off):
// frames drawn, game ticks and milliseconds. false until the first second has passed.
bool fps_overlay_last(uint32_t *frames, uint32_t *ticks, uint32_t *ms);

#endif
