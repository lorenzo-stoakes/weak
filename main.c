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

#include <stdio.h>
#include <time.h>
#include "weak.h"

//#define SHOW_PERFT_STATS

#define FEN FEN1
#define DEPTH 6

int
main()
{
  clock_t ticks;
  double elapsed;
  Game game;

#if defined(SHOW_PERFT_STATS)
  PerftStats ret;
#else
  uint64_t ret;
#endif

  SetUnbufferedOutput();

  puts("WeakC v0.0.dev.\n");

  printf("Initialising... ");
  InitEngine();
  puts("done.\n");

  game = ParseFen(FEN);

  puts(StringChessSet(&game.ChessSet));

  // Warm up.
  if(DEPTH > 2) {
    QuickPerft(&game, DEPTH-2);
    QuickPerft(&game, DEPTH-1);    
    QuickPerft(&game, DEPTH);    
  }

  ticks = clock();

#if defined(SHOW_PERFT_STATS)
  ret = Perft(&game, DEPTH);
#else
  ret = QuickPerft(&game, DEPTH);
#endif
    
  ticks = clock() - ticks;

  // In ms.
  elapsed = 1000*((double)ticks)/CLOCKS_PER_SEC;

#if defined(SHOW_PERFT_STATS)
  puts(StringPerft(&ret));
  printf("%fms elapsed.\n", elapsed);
#else
  printf("%llu - %f elapsed, %f Mnps.\n", ret, elapsed, ret/elapsed/1000);
#endif

  return 0;
}
