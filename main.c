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
#include <string.h>
#include <time.h>
#include "weak.h"

int
main(int argc, char **argv)
{
  Game game;
  uint64_t perftVal;
  int depth;

  SetUnbufferedOutput();

  if(argc == 2 && strcmp(argv[1], "--version") == 0) {
      printf("Weak %s.\n", version);
      return EXIT_SUCCESS;
  }

  if(argc < 3) {
    fprintf(stderr, "Usage: %s [fen] [depth]\n", argv[0]);
    return EXIT_FAILURE;
  }

  if(!(depth = atoi(argv[2]))) {
    fprintf(stderr, "Invalid depth '%s'.\n", argv[2]);
  }

  // Initialise prng.
  randk_seed();
  randk_warmup(KISS_WARMUP_ROUNDS);

  InitEngine();

  game = ParseFen(argv[1]);

  perftVal = QuickPerft(&game, depth);

  printf("%lu\n", perftVal);

  return EXIT_SUCCESS;
}
