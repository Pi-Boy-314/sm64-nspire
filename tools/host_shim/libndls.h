// Host-build stand-in for Ndless libndls.h (see Makefile HOST=1). No input, no LCD.
#ifndef HOST_SHIM_LIBNDLS_H
#define HOST_SHIM_LIBNDLS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

typedef int t_key;
#define KEY_NSPIRE_ESC 1
#define KEY_NSPIRE_CTRL 2
#define KEY_NSPIRE_ENTER 3
#define KEY_NSPIRE_MENU 4
#define KEY_NSPIRE_DEL 5
#define KEY_NSPIRE_DOC 6
#define KEY_NSPIRE_LEFT 7
#define KEY_NSPIRE_RIGHT 8
#define KEY_NSPIRE_UP 9
#define KEY_NSPIRE_DOWN 10
#define KEY_NSPIRE_LEFTUP 11
#define KEY_NSPIRE_UPRIGHT 12
#define KEY_NSPIRE_RIGHTDOWN 13
#define KEY_NSPIRE_DOWNLEFT 14
#define KEY_NSPIRE_8 15
#define KEY_NSPIRE_2 16
#define KEY_NSPIRE_6 17
#define KEY_NSPIRE_4 18

typedef enum {
    SCR_TYPE_INVALID = -1,
    SCR_320x240_565 = 0
} scr_type_t;

#define CPU_SPEED_150MHZ 150

static inline bool isKeyPressed(t_key k) { (void) k; return false; }
static inline void msleep(unsigned ms) { usleep(ms * 1000u); }
static inline void wait_key_pressed(void) {}
static inline bool lcd_init(scr_type_t t) { (void) t; return true; }
static inline void lcd_blit(void *buf, scr_type_t t) { (void) buf; (void) t; }
static inline void enable_relative_paths(char **argv) { (void) argv; }
static inline void set_cpu_speed(int s) { (void) s; }

#endif
