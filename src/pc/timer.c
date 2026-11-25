#ifdef HOST_BUILD
// Host build: ticks are microseconds from CLOCK_MONOTONIC.
#include <stdio.h>
#include <time.h>
#include "timer.h"

static uint64_t sBase, sStopAt;
static int sRunning;
static uint64_t now_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t) ts.tv_sec * 1000000u + ts.tv_nsec / 1000u;
}
static uint64_t elapsed_us(void) { return (sRunning ? now_us() : sStopAt) - sBase; }

void tmr_init(void) { tmr_reset(); tmr_start(); }
void tmr_stop(void) { if (sRunning) { sStopAt = now_us(); sRunning = 0; } }
void tmr_start(void) { if (!sRunning) { sBase += now_us() - sStopAt; sRunning = 1; } }
void tmr_reset(void) { sBase = sStopAt = now_us(); }
uint32_t tmr_ms(void) { return (uint32_t) (elapsed_us() / 1000u); }
void tmr_shutdown(void) { tmr_reset(); }
uint32_t tmr_ticks(void) { return (uint32_t) elapsed_us(); }
uint32_t tmr_ticks_per_sec(void) { return 1000000u; }
uint32_t _tmr_val(void) { return (uint32_t) now_us(); }
#else
#include <stdlib.h>
#include <libndls.h>

#define TIMER_BASE ((unsigned*)0x900C0000)
#define CALIB_TIME 250

static volatile unsigned *tmr_load = TIMER_BASE;
static volatile unsigned *tmr_val = TIMER_BASE + 1;
static volatile unsigned *tmr_control = TIMER_BASE + 2;

static unsigned old_control;
static unsigned old_load;

static unsigned new_control = 0b10000010; // enable, free-running, no-interrupt, (n/a), no-prescale (00), 32-bit, wrap
static unsigned start_val;

static uint32_t tmr_freq_ms; // frequency in ms, initialized by tmr_init
static uint32_t tmr_freq_sec; // ticks per second, initialized by tmr_init (more precise than tmr_freq_ms)
static uint32_t time_ms = 0;
static uint32_t old_ms = 0;

uint32_t tmr_ms(void) {
    uint32_t new_ms = (start_val - (*tmr_val)) / tmr_freq_ms;
    uint32_t delta_ms = new_ms - old_ms;

    time_ms += delta_ms;

    old_ms = new_ms;
    return time_ms;
}

void tmr_start(void) {
    *tmr_control = *tmr_control | 0b10000000;
}

void tmr_stop(void) {
    *tmr_control = *tmr_control & 0b01111111;
}

void tmr_reset(void) {
    tmr_stop();
    time_ms = 0;
    old_ms = 0;
    start_val = *tmr_val;
}

void tmr_init(void) {
    old_control = *tmr_control;
    *tmr_control = *tmr_control & 0b01111111; // disable timer before modifying it
    old_load = *tmr_load;

    *tmr_control = new_control & 0b01111111; // new controls but disabled
    start_val = *tmr_val;
    *tmr_load = start_val;

    tmr_reset(); // just in case

    uint32_t v0 = *tmr_val; // perform calibration
    tmr_start(); 
    msleep(CALIB_TIME);
    tmr_stop();
    tmr_freq_sec = (v0 - *tmr_val) * (1000 / CALIB_TIME);
    tmr_freq_ms = (v0 - *tmr_val) / CALIB_TIME;

    printf("v0: %lu\nv1: %lu\nfreq: %lu\nms: %lu\n", v0, *tmr_val, tmr_freq_ms, tmr_ms());

    tmr_reset();
    tmr_start();
}

void tmr_shutdown(void) {
    tmr_reset();
    *tmr_load = old_load;
    *tmr_control = old_control;
}

uint32_t _tmr_val(void) {
    return *tmr_val;
}

// Raw elapsed timer ticks since the last tmr_reset(). Wraps at 2^32, so only use
// differences between two readings (unsigned subtraction handles the wrap).
uint32_t tmr_ticks(void) {
    return start_val - *tmr_val;
}

uint32_t tmr_ticks_per_sec(void) {
    return tmr_freq_sec;
}

#endif
