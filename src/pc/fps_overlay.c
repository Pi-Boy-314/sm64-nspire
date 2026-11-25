// fps_overlay.c - FPS counter in the bottom-left corner of the screen
//
// Shows frames drawn per second and the game speed (game ticks per second as a share of the
// N64's 30), averaged over about a second. Drawn into the final 320x240 picture, so it stays
// sharp in 120p mode too.
#include <stdbool.h>
#include <stdio.h>

#include "fps_overlay.h"
#include "configfile.h"
#include "timer.h"

#define SCREEN_W 320
#define SCREEN_H 240
#define GLYPH_W 5
#define GLYPH_H 7
#define ADVANCE (GLYPH_W + 1)
#define PAD 2
#define TEXT_MAX 24
#define COLOR_TEXT 0xFFFF // white, RGB565
#define COLOR_BOX 0x0000  // black

static uint32_t window_start_ms;
static uint32_t window_frames, window_ticks;
static uint32_t last_frames, last_ticks, last_ms; // the last full window, for the speed info screen
static char text[TEXT_MAX] = "";

// 5x7 glyphs, one byte per row, bit 4 = leftmost pixel
static const uint8_t *glyph(char c) {
    static const uint8_t digits[10][GLYPH_H] = {
        { 0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E }, // 0
        { 0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E }, // 1
        { 0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F }, // 2
        { 0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E }, // 3
        { 0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02 }, // 4
        { 0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E }, // 5
        { 0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E }, // 6
        { 0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08 }, // 7
        { 0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E }, // 8
        { 0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C }, // 9
    };
    static const uint8_t dot[GLYPH_H] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C };
    static const uint8_t pct[GLYPH_H] = { 0x18, 0x19, 0x02, 0x04, 0x08, 0x13, 0x03 };
    static const uint8_t F[GLYPH_H] = { 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10 };
    static const uint8_t P[GLYPH_H] = { 0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10 };
    static const uint8_t S[GLYPH_H] = { 0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E };
    static const uint8_t dash[GLYPH_H] = { 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00 };
    static const uint8_t blank[GLYPH_H] = { 0 };
    switch (c) {
        case '.': return dot;
        case '%': return pct;
        case '-': return dash;
        case 'F': return F;
        case 'P': return P;
        case 'S': return S;
        default: return c >= '0' && c <= '9' ? digits[c - '0'] : blank;
    }
}

void fps_overlay_tick(void) {
    window_ticks++;
}

bool fps_overlay_last(uint32_t *frames, uint32_t *ticks, uint32_t *ms) {
    *frames = last_frames;
    *ticks = last_ticks;
    *ms = last_ms;
    return last_ms > 0;
}

void fps_overlay_draw(uint16_t *screen) {
    // measured all the time, so the speed info screen has numbers even with the counter off
    window_frames++;
    const uint32_t now = tmr_ms();
    const uint32_t elapsed = now - window_start_ms;
    if (elapsed >= 1000) {
        last_frames = window_frames;
        last_ticks = window_ticks;
        last_ms = elapsed;
        window_start_ms = now;
        window_frames = window_ticks = 0;
        // frames drawn per second, to one decimal; game speed in percent of 30 ticks/s.
        // One frame in a window is enough: below 1 fps the window just runs longer.
        const uint32_t fps10 = (last_frames * 10000u + elapsed / 2) / elapsed;
        const uint32_t speed = (last_ticks * 100000u / 30u + elapsed / 2) / elapsed;
        snprintf(text, sizeof(text), "%lu.%lu FPS %lu%%", (unsigned long) (fps10 / 10),
                 (unsigned long) (fps10 % 10), (unsigned long) speed);
    }
    if (!configShowFps) {
        return;
    }
    if (text[0] == '\0') {
        snprintf(text, sizeof(text), "- FPS"); // first second: nothing measured yet
    }

    int len = 0;
    while (text[len] != '\0') {
        len++;
    }
    // black box, then the text, in the bottom-left corner
    const int box_w = PAD * 2 + len * ADVANCE - 1;
    const int box_h = PAD * 2 + GLYPH_H;
    const int y0 = SCREEN_H - box_h;
    for (int y = y0; y < SCREEN_H; y++) {
        for (int x = 0; x < box_w && x < SCREEN_W; x++) {
            screen[y * SCREEN_W + x] = COLOR_BOX;
        }
    }
    for (int i = 0; i < len; i++) {
        const uint8_t *g = glyph(text[i]);
        const int gx = PAD + i * ADVANCE;
        for (int row = 0; row < GLYPH_H; row++) {
            for (int col = 0; col < GLYPH_W; col++) {
                if ((g[row] >> (GLYPH_W - 1 - col)) & 1) {
                    screen[(y0 + PAD + row) * SCREEN_W + gx + col] = COLOR_TEXT;
                }
            }
        }
    }
}
