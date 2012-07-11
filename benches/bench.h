#ifndef BENCH_HEADER
#define BENCH_HEADER

#include "../weak.h"

#define BENCH_COUNT 1
// Minimum elapsed time to take a measurement from, in ms.
#define MIN_ELAPSED 1000

// perft_bench.c
void BenchPerft(void);

// util.c
void OutputBenchResults(char*, double, long, int64_t);

int64_t (*BenchFunctions[BENCH_COUNT])(void);
char *BenchNames[BENCH_COUNT];

#endif
