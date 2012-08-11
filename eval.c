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

#include "weak.h"

static double weights[] = { 0, 1, 3.5, 3.5, 5, 9 };

// Value of game position for white.
double
Eval(Game *game, Side side)
{
  double ret = 0;
  Piece piece;

  // Very basic evaluation for now.

  if(Checkmated(game)) {
    ret = SMALL * (1 - 2*game->WhosTurn);
  } else {
    for(piece = Pawn; piece <= Queen; piece++) {
      ret += weights[piece]*PopCount(game->ChessSet.Sets[White].Boards[piece]);
      ret -= weights[piece]*PopCount(game->ChessSet.Sets[Black].Boards[piece]);
    }
  }

  return ret * (1 - 2*side);
}
