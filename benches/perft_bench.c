#include <stdio.h>
#include <time.h>
#include "../weak.h"
#include "bench.h"

#if defined(QUICK_BENCH)
#define MAX_DEPTH 6
#else
#define MAX_DEPTH 7
#endif

static char *fens[PERFT_COUNT] = { FEN1, FEN2, FEN3, FEN4, FEN4_REVERSED };
static Game games[PERFT_COUNT];
static int depthCounts[PERFT_COUNT] = { 6, 6, 7, 6, 6 };

void
BenchPerft()
{
  char tmp[200];
  clock_t ticks;
  double elapsed, totalElapsed = 0;
  int i, j;
  int64_t nodes, totalNodes = 0;
  long iters, k;

  // Initialise games.
  for(i = 0; i < PERFT_COUNT; i++) {
    games[i] = ParseFen(fens[i]);
  }

  for(i = 0; i < PERFT_COUNT; i++) {
    // Depth 1-3 tests are too short to be meaningful. Ignore.
    for(j = 4; j <= depthCounts[i] && j <= MAX_DEPTH; j++) {
      elapsed = 0;
      iters = 1;

      while(elapsed < MIN_ELAPSED) {
        ticks = clock();
        for(k = 0; k < iters; k++) {
          nodes = (int64_t)QuickPerft(&games[i], j);
        }
        ticks = clock() - ticks;
        // In ms.
        elapsed = 1000*((double)ticks)/CLOCKS_PER_SEC;
        iters *= 2;
      }
      // Remove final *2.
      iters /= 2;

      totalNodes += nodes;
      totalElapsed += elapsed/iters;

      sprintf(tmp, "Perft Position %d Depth %d", i+1, j);
      OutputBenchResults(tmp, elapsed, iters, nodes);
    }
  }

  printf("Median Perft Performance: %f Mn/s\n", 1E-3*totalNodes/totalElapsed);
}
