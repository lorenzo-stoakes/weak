/*
  Weak, a chess perft calculator derived from Stockfish.

  Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
  Copyright (C) 2008-2012 Marco Costalba, Joona Kiiski, Tord Romstad (Stockfish authors)
  Copyright (C) 2011-2012 Lorenzo Stoakes

  Weak is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Weak is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
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
