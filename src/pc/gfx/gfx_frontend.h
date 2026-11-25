#ifndef GFX_FRONTEND_H
#define GFX_FRONTEND_H

#include <stdbool.h>
#include <stdint.h>

// gDPNoOpTag value the game puts in the display list between the 3D scene and the HUD, text
// and menus; with sharp_hud on, the renderer switches to full resolution there.
#define GFX_TAG_HUD_START 0x53485544 // "SHUD"
// gDPNoOpTag values around the sky background
#define GFX_TAG_SKY_START 0x534B5953 // "SKYS"
#define GFX_TAG_SKY_END   0x534B5945 // "SKYE"

struct GfxRenderingAPI;
struct GfxWindowManagerAPI;

struct GfxDimensions {
    uint32_t width, height;
    float aspect_ratio;
};

extern struct GfxDimensions gfx_current_dimensions;
// Set each frame: a menu screen (no Mario: title, file select, star select, ...) that
// sharp_text hud_and_menus draws whole at 320x240 with normal textures.
extern bool gfx_menu_screen;

#ifdef __cplusplus
extern "C" {
#endif

void gfx_init(struct GfxWindowManagerAPI *wapi, struct GfxRenderingAPI *rapi, const char *game_name, bool start_in_fullscreen);
void gfx_shutdown(void);
struct GfxRenderingAPI *gfx_get_current_rendering_api(void);
void gfx_start_frame(void);
void gfx_run(Gfx *commands);
void gfx_end_frame(void);

#ifdef __cplusplus
}
#endif

#endif
