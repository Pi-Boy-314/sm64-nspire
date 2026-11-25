// Host-build stand-in for Ndless nspireio.h: console output goes to stdout.
#ifndef HOST_SHIM_NSPIREIO_H
#define HOST_SHIM_NSPIREIO_H

#include <stdbool.h>
#include <stdio.h>

typedef struct { int unused; } nio_console;

#define NIO_MAX_ROWS 30
#define NIO_MAX_COLS 53
#define NIO_COLOR_WHITE 0
#define NIO_COLOR_BLACK 1

#define nio_printf printf
#define nio_puts(s) fputs((s), stdout)

static inline bool nio_init(nio_console *c, int cols, int rows, int x, int y, int fg, int bg, bool drawing) {
    (void) c; (void) cols; (void) rows; (void) x; (void) y; (void) fg; (void) bg; (void) drawing;
    return true;
}
static inline void nio_set_default(nio_console *c) { (void) c; }
static inline void nio_free(nio_console *c) { (void) c; }

#endif
