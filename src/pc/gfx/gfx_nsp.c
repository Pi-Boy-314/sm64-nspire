#include <libndls.h>
#include <SDL/SDL.h>

#include "gfx_window_manager_api.h"
#include "gfx_backend.h"
#include "macros.h"
#include "nspireio.h"

#include "pc/configfile.h"
#include "pc/timer.h"
#include "pc/profiling.h"
#include "pc/benchmark.h"

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

            int to_skip = (new_frames > configFrameskip) ? configFrameskip : (new_frames - 1); // catch up by skipping up to configFrameskip frames

            uint32_t t0 = tmr_ms();

            //printf("Rendering: %lu\nSkipping: %lu\n", new_frames, to_skip);
            for (int i = 0; i < to_skip + 1; ++i) { // render only one frame out of this chunk
                //printf("skipping: %ld\n", to_skip);
                skip_frame = i < to_skip;
                run_one_game_iter();

                if (isKeyPressed(KEY_NSPIRE_ESC)) {
                    return;
                }
            }
            //printf("tFRAME: %lu\n", tmr_ms() - t0);


            // Status screen / debug
            if (isKeyPressed(KEY_NSPIRE_CTRL)) {
                tmr_stop();
                if (!nio_init(console, NIO_MAX_COLS, NIO_MAX_ROWS, 0, 0, NIO_COLOR_WHITE, NIO_COLOR_BLACK, true))
                    abort();

                uint32_t tDelta = tmr_ms() - t0;
                float fps = 1000.f / tDelta;
                uint32_t raster_us = prof_ticks_to_us(gProfFrame.raster_ticks);
                uint32_t gfx_us = prof_ticks_to_us(gProfFrame.ticks[PROF_GFX]);

                nio_printf("Total elapsed (ms): %lu\n"
                    "Total frame time (ms): %lu\n"
                    "FPS, physical: %f\n"
                    "FPS, virtual: %f\n"
                    "^ This includes frames skipped\n"
                    "Frames skipped: %d\n"
                    "\nLast drawn tick (ms):\n"
                    "  logic %lu, scene %lu\n"
                    "  gfx front %lu, raster %lu\n"
                    "  present %lu\n"
                    "  tris %lu\n"
                    "Skipped ticks cost only logic + scene.\n",
                    tmr_ms(), tDelta, fps, fps * (to_skip + 1), to_skip,
                    prof_ticks_to_us(gProfFrame.ticks[PROF_LOGIC]) / 1000,
                    prof_ticks_to_us(gProfFrame.ticks[PROF_SCENE]) / 1000,
                    (gfx_us - raster_us) / 1000, raster_us / 1000,
                    prof_ticks_to_us(gProfFrame.ticks[PROF_PRESENT]) / 1000,
                    (unsigned long) gProfFrame.tris);

                wait_key_pressed();
                nio_free(console);
                tmr_start();
            }
             
        } // mostly borrowed from dos implementation

        frames_prev = frames_now;
    }
}

void nsp_get_dimensions(uint32_t *width, uint32_t *height) {
    if (config120pMode) { // quarter the pixel resolution
        *width = HALF_WIDTH;
        *height = HALF_HEIGHT;
    } else {
        *width = SCREEN_WIDTH;
        *height = SCREEN_HEIGHT;
    }
}

bool nsp_start_frame(void) {
    return !skip_frame; // (current_frame % 4) == 0; //
}

static inline uint16_t c4444_to_c565(uint32_t c) { // aaaaaaaa bbbbbbbb gggggggg rrrrrrrr -> rrrrr gggggg bbbbb
    return ((c & 0b11111000) << 8) | ((c & 0b1111110000000000) >> 5) | ((c >> 19) & 0b11111);
}

void nsp_swap_buffers_begin(void) {
    
}

void nsp_swap_buffers_end(void) {
    // Two RGB565 pixels per word: the ARM is little-endian, so the low half is the left pixel.
    // Writing whole words halves the number of stores compared to 16-bit writes.
    static uint32_t buffer[SCREEN_WIDTH * SCREEN_HEIGHT / 2];
    const uint32_t *src = gfx_output;

    // populate buffer according to configuration
    if (config120pMode) {
        // spread 160 * 120 img to fill whole screen: each pixel becomes a 2*2 square
        for (int y = 0; y < HALF_HEIGHT; y++) {
            uint32_t *row0 = buffer + y * SCREEN_WIDTH; // 2 screen rows = SCREEN_WIDTH words
            uint32_t *row1 = row0 + SCREEN_WIDTH / 2;
            for (int x = 0; x < HALF_WIDTH; x++) {
                uint32_t c16 = c4444_to_c565(*src++);
                uint32_t pair = c16 | (c16 << 16);
                row0[x] = pair;
                row1[x] = pair;
            }
        }
    } else {
        for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT / 2; i++, src += 2) {
            buffer[i] = c4444_to_c565(src[0]) | ((uint32_t) c4444_to_c565(src[1]) << 16);
        }
    }

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