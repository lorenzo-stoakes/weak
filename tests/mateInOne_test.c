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
  "3q1rk1/5pbp/5Qp1/8/8/2B5/5PPP/6K1 w - -",
  "2r2rk1/2q2p1p/6pQ/4P1N1/8/8/1PP5/2KR4 w - -",
  "r2q1rk1/pp1p1p1p/5PpQ/8/4N3/8/PP3PPP/R5K1 w - -",
  "6r1/7k/2p1pPp1/3p4/8/1R6/5PPP/5K2 w - -",
  "1r4k1/1q3p2/5Bp1/8/8/8/PP6/1K5R w - -",
  "r4rk1/5p1p/8/8/8/8/1BP5/2KR4 w - -",
  "4r2k/4r1p1/6p1/8/2B5/8/1PP5/2KR4 w - -",
  "8/2r1N1pk/8/8/8/2q2p2/2P5/2KR4 w - -",
  "r7/4KNkp/8/8/B7/8/8/1R6 w - -",
  "2kr4/3n4/2p5/8/5B2/8/6PP/5B1K w - -"
};

static char* mates[COUNT] = {
  "f6g7", "h6h7", "h6g7", "b3h3", "h1h7", "d1g1", "d1h1", "d1h1", "b1g1", "f1a6"
};

// Test that our search finds example mates in one.
char*
TestMatesInOne()
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
    actual = Search(&game, &dummy, 1);

    if(actual != expected) {
      AppendString(&builder, "Search failed mate-in-one for:-\n\n"
                   "%s\n"
                   "Expected move %s, engine selected %s.\n\n",
                   StringChessSet(&game.ChessSet), mates[i], StringMove(actual));
    }
  }

  return builder.Length == 0 ? NULL : BuildString(&builder, true);
}
