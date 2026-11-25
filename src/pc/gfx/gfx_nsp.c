#include <libndls.h>
#include <SDL/SDL.h>

#include "gfx_window_manager_api.h"
#include "gfx_backend.h"

extern bool gfx_menu_screen; // gfx_frontend.c: whole menu screen at full resolution
#include "macros.h"
#include "nspireio.h"

#include "pc/configfile.h"
#include "pc/timer.h"
#include "pc/profiling.h"
#include "pc/benchmark.h"
#include "pc/fps_overlay.h"
#include "pc/controls.h"
#include "pc/settings_menu.h"

#define HALF_WIDTH SCREEN_WIDTH / 2
#define HALF_HEIGHT SCREEN_HEIGHT / 2

static uint32_t frames_now = 0;
static uint32_t frames_prev = 0;

static bool skip_frame = false;

void nsp_init(UNUSED const char *game_name, UNUSED bool start_in_fullscreen) {
    //set_cpu_speed(CPU_SPEED_150MHZ);
    lcd_init(SCR_320x240_565);
}

void nsp_main_loop(void (*run_one_game_iter)(void)) {
    nio_console console_storage; // nio_init() writes the console handle here
    nio_console *console = &console_storage;
    if (!nio_init(console, NIO_MAX_COLS, NIO_MAX_ROWS, 0, 0, NIO_COLOR_WHITE, NIO_COLOR_BLACK, true))
        abort();
    nio_set_default(console);
    
    tmr_init();

#ifdef BENCHMARK
    nio_puts("Running benchmark, ESC aborts...\n");
    nio_free(console);
    tmr_reset();
    tmr_start();
    bench_run(run_one_game_iter, &skip_frame);
    return;
#endif
    
    // the controls and settings were shown by settings_menu_run() before the game started
    nio_free(console);
    
    tmr_reset();
    tmr_start();
    while (true) {
        frames_now = tmr_ms() / 33; // one frame every 33 milliseconds -> 30 fps
        uint32_t new_frames = frames_now - frames_prev;

        if (new_frames) {
            //printf("ideal: %lu\ncurrent: %lu\n", frames_now, frames_prev); // PRINTING TOO MUCH ON HARDWARE CAUSES EXTREME TEARING

            // catch up by skipping up to max_skip frames; auto: as many as needed, within reason
            const uint32_t max_skip = configFrameskip == FRAMESKIP_AUTO ? FRAMESKIP_AUTO_MAX : configFrameskip;
            int to_skip = (new_frames > max_skip) ? max_skip : (new_frames - 1);

            //printf("Rendering: %lu\nSkipping: %lu\n", new_frames, to_skip);
            for (int i = 0; i < to_skip + 1; ++i) { // render only one frame out of this chunk
                //printf("skipping: %ld\n", to_skip);
                skip_frame = i < to_skip;
                run_one_game_iter();
                fps_overlay_tick();

                if (isKeyPressed(KEY_NSPIRE_ESC)) {
                    tmr_stop();
                    if (settings_menu_confirm_quit()) {
                        return;
                    }
                    tmr_start();
                }
            }
            //printf("tFRAME: %lu\n", tmr_ms() - t0);


            // settings screen during play; the timer stops so the game doesn't try to catch up
            if (controls_held(CONTROL_SETTINGS)) {
                tmr_stop();
                settings_menu_run_paused();
                tmr_start();
            }

            // speed info screen; the timer stops so the game doesn't try to catch up
            if (isKeyPressed(KEY_NSPIRE_CTRL)) {
                tmr_stop();
                settings_menu_speed_info(to_skip);
                tmr_start();
            }
        } // mostly borrowed from dos implementation

        frames_prev = frames_now;
    }
}

void nsp_get_dimensions(uint32_t *width, uint32_t *height) {
    const unsigned int res = gfx_menu_screen ? RES_320x240 : configResolution;
    *width = SCREEN_WIDTH >> res;
    *height = SCREEN_HEIGHT >> res;
}

bool nsp_start_frame(void) {
    return !skip_frame; // (current_frame % 4) == 0; //
}

static inline uint16_t c4444_to_c565(uint32_t c) { // aaaaaaaa bbbbbbbb gggggggg rrrrrrrr -> rrrrr gggggg bbbbb
    return ((c & 0b11111000) << 8) | ((c & 0b1111110000000000) >> 5) | ((c >> 19) & 0b11111);
}

void nsp_swap_buffers_begin(void) {
    
}

static inline uint32_t c565_pair(uint32_t c) { // one pixel, twice
    const uint32_t c16 = c4444_to_c565(c);
    return c16 | (c16 << 16);
}

// One row of a lower-resolution picture (w pixels), each pixel drawn f x f (f = 2 or 4).
// dst is in words (two pixels), SCREEN_WIDTH / 2 words per screen row.
static inline void present_low_row(uint32_t *dst, const uint32_t *src, int w, int f) {
    if (f == 2) {
        for (int x = 0; x < w; x++) {
            const uint32_t pair = c565_pair(src[x]);
            dst[x] = pair;
            dst[x + SCREEN_WIDTH / 2] = pair;
        }
    } else {
        for (int x = 0; x < w; x++) {
            const uint32_t pair = c565_pair(src[x]);
            uint32_t *p = dst + 2 * x;
            p[0] = p[1] = pair;
            p[SCREEN_WIDTH / 2] = p[SCREEN_WIDTH / 2 + 1] = pair;
            p[SCREEN_WIDTH] = p[SCREEN_WIDTH + 1] = pair;
            p[SCREEN_WIDTH * 3 / 2] = p[SCREEN_WIDTH * 3 / 2 + 1] = pair;
        }
    }
}

// rows full-resolution rows
static inline void present_full_rows(uint32_t *dst, const uint32_t *src, int rows) {
    for (int i = 0; i < rows * SCREEN_WIDTH / 2; i++, src += 2) {
        dst[i] = c4444_to_c565(src[0]) | ((uint32_t) c4444_to_c565(src[1]) << 16);
    }
}

void nsp_swap_buffers_end(void) {
    // Two RGB565 pixels per word: the ARM is little-endian, so the low half is the left pixel.
    // Writing whole words halves the number of stores compared to 16-bit writes.
    static uint32_t buffer[SCREEN_WIDTH * SCREEN_HEIGHT / 2];

    int f;
    const uint8_t *hud_rows;
    const uint32_t *scene = gfx_soft_promoted_scene(&f, &hud_rows);
    if (scene != NULL) {
        // sharp HUD: rows the HUD drew on are at full resolution, the rest only in the scene
        const int w = SCREEN_WIDTH / f;
        for (int ly = 0; ly < SCREEN_HEIGHT / f; ly++) {
            uint32_t *dst = buffer + ly * f * (SCREEN_WIDTH / 2);
            if (hud_rows[ly]) {
                present_full_rows(dst, gfx_output + ly * f * SCREEN_WIDTH, f);
            } else {
                present_low_row(dst, scene + ly * w, w, f);
            }
        }
    } else {
        const int w = gfx_soft_output_width();
        f = SCREEN_WIDTH / w;
        if (f <= 1) {
            present_full_rows(buffer, gfx_output, SCREEN_HEIGHT);
        } else {
            for (int ly = 0; ly < SCREEN_HEIGHT / f; ly++) {
                present_low_row(buffer + ly * f * (SCREEN_WIDTH / 2), gfx_output + ly * w, w, f);
            }
        }
    }

#ifndef BENCHMARK
    fps_overlay_draw((uint16_t *) buffer); // little-endian: pixel order matches the word pairs
#endif
    lcd_blit(buffer, SCR_320x240_565);
}

// unimplemented windowing features
void nsp_set_keyboard_callbacks(
    UNUSED bool (*on_key_down)(int scancode), 
    UNUSED bool (*on_key_up)(int scancode),
    UNUSED void (*on_all_keys_up)(void)) {
}
void nsp_set_fullscreen_changed_callback(UNUSED void (*on_fullscreen_changed)(bool is_now_fullscreen)) {
}
void nsp_set_fullscreen(UNUSED bool enable) {
}
void nsp_handle_events(void) {
}
double nsp_get_time(void) {
    return 0.0;
}

void nsp_shutdown(void) {
    lcd_init(SCR_TYPE_INVALID);
    //tmr_shutdown(); // BANDAID FIX: attempting to restore old timer soft locks calc. not required but could be problematic? 
}

struct GfxWindowManagerAPI gfx_nsp_api = { nsp_init,
                                           nsp_set_keyboard_callbacks,
                                           nsp_set_fullscreen_changed_callback,
                                           nsp_set_fullscreen,
                                           nsp_main_loop,
                                           nsp_get_dimensions,
                                           nsp_handle_events,
                                           nsp_start_frame,
                                           nsp_swap_buffers_begin,
                                           nsp_swap_buffers_end,
                                           nsp_get_time,
                                           nsp_shutdown};

// #endif