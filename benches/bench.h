/*
  Weak, a chess engine derived from Stockfish.

  Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
  Copyright (C) 2008-2012 Marco Costalba, Joona Kiiski, Tord Romstad (Stockfish authors)
  Copyright (C) 2011-2012 Lorenzo Stoakes

  Stockfish is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Stockfish is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
