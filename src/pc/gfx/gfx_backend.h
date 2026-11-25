#ifndef GFX_BACKEND_H
#define GFX_BACKEND_H

#include "gfx_rendering_api.h"

extern struct GfxRenderingAPI gfx_soft_api;
extern uint32_t *gfx_output;

// Bytes of converted texture data held by the renderer, and a way to drop all of it.
uint32_t gfx_soft_texture_bytes(void);
// Between GFX_TAG_SKY_START and _END: flat colours leave the sky textured.
void gfx_soft_set_sky(bool sky);
// Sharp HUD: stretch the frame drawn so far to 320x240 and continue at that size.
void gfx_soft_promote_to_full_res(void);
// With sharp HUD after the switch: the 3D scene at its own size, how much it is stretched (2
// or 4), and which of its rows the HUD drew on (those are in gfx_output at 320x240; the rest
// are only in the scene). NULL when the frame wasn't switched.
const uint32_t *gfx_soft_promoted_scene(int *f, const uint8_t **rows_drawn);
// Stretches every remaining row into gfx_output, so it holds the whole frame (benchmark).
void gfx_soft_promote_finish(void);
// Width of the frame in gfx_output (320, 160 or 80); can differ from the setting mid-frame.
int gfx_soft_output_width(void);
void gfx_soft_reset_textures(void);

#endif
