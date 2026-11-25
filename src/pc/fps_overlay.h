#ifndef FPS_OVERLAY_H
#define FPS_OVERLAY_H

#include <stdint.h>

// FPS counter in the bottom-left corner (config option show_fps).

// Call once per game tick, drawn or skipped.
void fps_overlay_tick(void);
// Call once per drawn frame with the 320x240 RGB565 picture about to be shown.
void fps_overlay_draw(uint16_t *screen);

#endif
