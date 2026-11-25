#ifdef BENCHMARK

// Benchmark mode: see benchmark.h.
//
// The game is deterministic given identical inputs, and the attract-mode demos
// are fixed input recordings, so every run plays exactly the same frames. That
// makes the timings comparable between builds, and the frame hash tells you
// whether a change altered what ends up on screen.

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <libndls.h>

#include "nspireio.h"
#include "sm64.h"
#include "game/game_init.h"
#include "gfx/gfx_backend.h"

#include "benchmark.h"
#include "configfile.h"
#include "profiling.h"
#include "timer.h"

#ifndef GIT_HASH
#define GIT_HASH "unknown"
#endif

#define BENCH_RESULTS_FILE "sm64_bench.txt.tns"
#define BENCH_MAX_BOOT_TICKS 3000 // the demo normally starts within a few hundred ticks
#define TICK_US 33333              // one game tick at the intended 30 Hz

static FILE *sOut;

// Print to the screen and to the results file.
static void out(const char *fmt, ...) {
    char buf[160];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    nio_printf("%s", buf);
    if (sOut != NULL) {
        fputs(buf, sOut);
    }
}

// Microseconds -> "12.34" (milliseconds, 2 decimals). Returns dst.
static const char *ms(char *dst, uint32_t us) {
    sprintf(dst, "%lu.%02lu", (unsigned long) (us / 1000), (unsigned long) ((us % 1000) / 10));
    return dst;
}

static bool demo_running(void) {
    // timer == 0 marks the end of the input list; the game then warps back to the title
    return gCurrDemoInput != NULL && gCurrDemoInput->timer != 0;
}

// FNV-1a over the frame as it is shown on the LCD (RGB565), so changes that
// keep the visible image identical (e.g. a different internal pixel format)
// keep the same hash.
static uint32_t hash_frame(uint32_t h, uint32_t num_pixels) {
    for (uint32_t i = 0; i < num_pixels; i++) {
        uint32_t c = gfx_output[i];
        uint32_t c565 = ((c & 0xF8) << 8) | ((c & 0xFC00) >> 5) | ((c >> 19) & 0x1F);
        h = (h ^ c565) * 16777619u;
    }
    return h;
}

static void dump_frame(uint32_t tick, uint32_t w, uint32_t h) {
    char name[40];
    static uint8_t row[SCREEN_WIDTH * 3];

    sprintf(name, "sm64_bench_%03lu.ppm.tns", (unsigned long) tick);
    FILE *f = fopen(name, "wb");
    if (f == NULL) {
        return;
    }
    fprintf(f, "P6\n%lu %lu\n255\n", (unsigned long) w, (unsigned long) h);
    for (uint32_t y = 0; y < h; y++) {
        for (uint32_t x = 0; x < w; x++) {
            uint32_t c = gfx_output[y * w + x]; // aaaaaaaa bbbbbbbb gggggggg rrrrrrrr
            row[x * 3 + 0] = c & 0xFF;
            row[x * 3 + 1] = (c >> 8) & 0xFF;
            row[x * 3 + 2] = (c >> 16) & 0xFF;
        }
        fwrite(row, 3, w, f);
    }
    fclose(f);
}

static void show_message_and_wait(const char *msg) {
    nio_console console;
    if (!nio_init(&console, NIO_MAX_COLS, NIO_MAX_ROWS, 0, 0, NIO_COLOR_WHITE, NIO_COLOR_BLACK, true)) {
        return;
    }
    nio_set_default(&console);
    nio_printf("%s\nPress any key to exit.\n", msg);
    wait_key_pressed();
    nio_free(&console);
}

void bench_run(void (*run_one_game_iter)(void), bool *skip_frame) {
    const uint32_t w = config120pMode ? SCREEN_WIDTH / 2 : SCREEN_WIDTH;
    const uint32_t h = config120pMode ? SCREEN_HEIGHT / 2 : SCREEN_HEIGHT;

    // 1. Boot to the attract demo without drawing (logic only, as fast as possible).
    gDemoInputListID = configBenchDemo;
    uint32_t boot_ticks = 0;
    *skip_frame = true;
    while (gCurrDemoInput == NULL) {
        run_one_game_iter();
        if (isKeyPressed(KEY_NSPIRE_ESC)) {
            show_message_and_wait("Benchmark aborted.");
            return;
        }
        if (++boot_ticks > BENCH_MAX_BOOT_TICKS) {
            show_message_and_wait("Benchmark failed: the demo never started.");
            return;
        }
    }
    // gDemoInputListID was advanced past the demo that just started
    uint32_t demo = (gDemoInputListID == 0 ? gDemo.animDmaTable->count : gDemoInputListID) - 1;

    // 2. Warm up: draw, don't measure (skips the level load and fade-in).
    *skip_frame = false;
    for (uint32_t i = 0; i < configBenchWarmup && demo_running(); i++) {
        run_one_game_iter();
    }

    // 3. Measure. Every tick is drawn, so logic and drawing costs are both in every sample.
    uint64_t total[PROF_NUM_BUCKETS] = { 0 };
    uint64_t raster = 0, tris = 0;
    uint32_t worst = 0, worst_tick = 0, n = 0;
    uint32_t hash = 2166136261u;

    while (n < configBenchTicks && demo_running()) {
        run_one_game_iter();

        uint32_t frame = 0;
        for (int b = 0; b < PROF_NUM_BUCKETS; b++) {
            total[b] += gProfFrame.ticks[b];
            frame += gProfFrame.ticks[b];
        }
        raster += gProfFrame.raster_ticks;
        tris += gProfFrame.tris;
        if (frame > worst) {
            worst = frame;
            worst_tick = n;
        }

        // Outside the timed region: prof_frame_begin() restarts the clock next tick.
        hash = hash_frame(hash, w * h);
        if (configBenchDump && (n == 60 || n == 240 || n == 420)) {
            dump_frame(n, w, h);
        }

        n++;
        if (isKeyPressed(KEY_NSPIRE_ESC)) {
            show_message_and_wait("Benchmark aborted.");
            return;
        }
    }
    if (n == 0) {
        show_message_and_wait("Benchmark failed: demo ended during warmup.\nLower bench_warmup.");
        return;
    }

    // 4. Report.
    uint32_t avg[PROF_NUM_BUCKETS];
    for (int b = 0; b < PROF_NUM_BUCKETS; b++) {
        avg[b] = prof_ticks_to_us(total[b]) / n;
    }
    uint32_t avg_raster = prof_ticks_to_us(raster) / n;
    uint32_t avg_front = avg[PROF_GFX] > avg_raster ? avg[PROF_GFX] - avg_raster : 0;
    uint32_t per_tick = avg[PROF_LOGIC] + avg[PROF_SCENE];        // paid by every tick
    uint32_t per_draw = avg[PROF_GFX] + avg[PROF_PRESENT];        // paid only by drawn ticks
    char s1[16], s2[16];

    nio_console console;
    if (!nio_init(&console, NIO_MAX_COLS, NIO_MAX_ROWS, 0, 0, NIO_COLOR_WHITE, NIO_COLOR_BLACK, true)) {
        abort();
    }
    nio_set_default(&console);
    sOut = fopen(BENCH_RESULTS_FILE, "w");

    out("SM64 Nspire benchmark, build %s\n", GIT_HASH);
    out("demo %lu, boot %lu, warmup %u, measured %lu ticks\n",
        (unsigned long) demo, (unsigned long) boot_ticks, configBenchWarmup, (unsigned long) n);
    out("%lux%lu, sky %d, fog %d, timer %lu Hz\n",
        (unsigned long) w, (unsigned long) h, configDrawSky, configEnableFog,
        (unsigned long) tmr_ticks_per_sec());
    out("-- avg ms per tick (every tick drawn) --\n");
    out("logic (objects, Mario)   %8s\n", ms(s1, avg[PROF_LOGIC]));
    out("scene (DL build)         %8s\n", ms(s1, avg[PROF_SCENE]));
    out("gfx front (DL, vertices) %8s\n", ms(s1, avg_front));
    out("gfx raster (triangles)   %8s\n", ms(s1, avg_raster));
    out("present (convert, blit)  %8s\n", ms(s1, avg[PROF_PRESENT]));
    out("total                    %8s\n", ms(s1, per_tick + per_draw));
    out("tris/frame %lu, worst tick %s ms (#%lu)\n",
        (unsigned long) (tris / n), ms(s1, prof_ticks_to_us(worst)), (unsigned long) worst_tick);
    out("-- est. per drawn frame (game speed) --\n");
    for (uint32_t k = 0; k <= 4; k += 4) {
        uint32_t frame_us = (k + 1) * per_tick + per_draw;
        uint32_t speed_pct = (uint32_t) ((uint64_t) (k + 1) * TICK_US * 100 / frame_us);
        out("frameskip %lu: %s ms, %s fps (%lu%%)\n", (unsigned long) k, ms(s1, frame_us),
            ms(s2, 1000000000u / frame_us), (unsigned long) speed_pct);
    }
    out("frame hash %08lx\n", (unsigned long) hash);

    if (sOut != NULL) {
        fclose(sOut);
        sOut = NULL;
        nio_printf("Saved to %s\n", BENCH_RESULTS_FILE);
    } else {
        nio_printf("Could not write %s\n", BENCH_RESULTS_FILE);
    }
    nio_printf("Press any key to exit.\n");
    wait_key_pressed();
    nio_free(&console);
}

#endif
