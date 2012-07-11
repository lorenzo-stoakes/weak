#include <stdio.h>
#include <strings.h>
#include <time.h>
#include "bench.h"

static void
init()
{
  // Perft benchmark handled specially.
  BenchFunctions[0] = NULL;
  BenchNames[0] = strdup("Perft Benchmark");
}

int
main()
{
  clock_t ticks;
  double elapsed;
  int i;
  int64_t nodes;
  long iters, j;

  init();
  InitEngine();

  // Want results to appear as soon as they are ready.
  SetUnbufferedOutput();

  // Handle perft benchmark specially.
  BenchPerft();

  for(i = 1; i < BENCH_COUNT; i++) {
    elapsed = 0;
    iters = 1;
    while(elapsed < MIN_ELAPSED) {
      ticks = clock();
      for(j = 0; j < iters; j++) {
        nodes = BenchFunctions[i]();
      }
      ticks = clock() - ticks;
      // In ms.
      elapsed = 1000*((double)ticks)/CLOCKS_PER_SEC;
      iters *= 2;
    }
    // Remove final *2.
    iters /= 2;

    OutputBenchResults(BenchNames[i], elapsed, iters, nodes);
  }

  return 0;
}
