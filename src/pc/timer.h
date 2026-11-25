#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

void tmr_init(void); // init, calibrate, + start timer
void tmr_stop(void); // stop (disable) timer
void tmr_start(void); // start (enable) timer
void tmr_reset(void); // reset elapsed ms to 0
uint32_t tmr_ms(void); // elapsed ms
void tmr_shutdown(void); // resets timer and restores old timer controls
uint32_t tmr_ticks(void); // raw elapsed ticks (wraps; use differences only)
uint32_t tmr_ticks_per_sec(void); // calibrated tick rate

#endif