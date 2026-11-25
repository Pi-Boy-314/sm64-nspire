#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stdbool.h>

// Self-running benchmark (build with `make BENCH=1`).
// Boots straight into an attract-mode demo, renders every tick (frameskip is
// ignored), and writes a per-stage timing breakdown to sm64_bench.txt.tns.
void bench_run(void (*run_one_game_iter)(void), bool *skip_frame);

#endif
