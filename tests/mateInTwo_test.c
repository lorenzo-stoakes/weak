/*
  Weak, a chess engine derived from Stockfish.

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

#include "test.h"

#define COUNT 10

// Taken from 'Chess' by Laszlo Polgar.

static char* fens[COUNT] = {
  "1Q6/8/8/8/8/k2K4/8/8 w - -",
  "8/8/8/8/8/k2K4/7Q/8 w - -",
  "8/8/k1K3Q1/8/8/8/8/8 w - -",
  "8/8/8/8/8/6KQ/8/4n1k1 w - -",
  "8/8/2p5/2Q5/7k/5K2/8/8 w - -",
  "4K2k/8/5N2/4Q3/8/8/8/8 w - -",
  "5K1k/8/8/6N1/8/3p4/8/1B6 w - -",
  "8/8/8/6N1/8/4K3/7k/5Q2 w - -",
  "8/8/8/8/8/K3Q3/B1k5/8 w - -",
  "8/5Q2/8/8/5p2/5K2/8/5k2 w - -"
};

static char* mates[COUNT] = {
  "d3c3", "d3c4", "g6b1", "h3h2", "c5f5", "f6h5", "g5f7", "f1h3", "a2b3", "f7a2"
};

// Test that our search finds example mates in one.
char*
TestMatesInTwo()
{
  char *fen;
  Game game;
  int i;
  Move actual, expected;
  uint64_t dummy = 0;

  StringBuilder builder = NewStringBuilder();

  for(i = 0; i < COUNT; i++) {
    fen = fens[i];
    expected = ParseMove(mates[i]);

    game = ParseFen(fen);
    actual = Search(&game, &dummy, 3);

    if(actual != expected) {
      AppendString(&builder, "Search failed mate-in-two for:-\n\n"
                   "%s\n"
                   "Expected move %s, engine selected %s.\n\n",
                   StringChessSet(&game.ChessSet), StringMove(expected), StringMove(actual));
    }
  }

  return builder.Length == 0 ? NULL : BuildString(&builder, true);
}
