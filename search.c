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

#define DEPTH 6

#define SMALL -1E10
#define BIG    1E10

// Perform hacky mini-max. Note the scoring is from white's perspective.
static double
miniMax(Game *game, double alpha, double beta, int depth, uint64_t *count)
{
  double max, val;
  Move moves[INIT_MOVE_LEN];
  Move *start = moves;
  Move *curr, *end;
  Side side = game->WhosTurn;

  if(depth == DEPTH) {
    return Eval(game, White);
  }

  end = AllMoves(moves, game);

  *count += end-start;

  // Iterate through all moves looking for the best, whose definition
  // varies based on who's turn it is.

  max = side == White ? SMALL : BIG;

  for(curr = start; curr != end; curr++) {
    DoMove(game, *curr);
    val = miniMax(game, alpha, beta, depth+1, count);
    Unmove(game);

    if(side == White) {
      if(val > beta) {
        return val;
      }

      if(val > max) {
        max = val;
        alpha = val;
      }
    } else {
      if(val < alpha) {
        return val;
      }

      if(val < max) {
        max = val;
        beta = val;
      }
    }
  }

  return max;
}

// Perform hacky mini-max. Note the scoring is from white's perspective.
Move
Search(Game *game, uint64_t *count)
{
  double max, val;
  Move moves[INIT_MOVE_LEN];
  Move best;
  Move *start = moves;
  Move *curr, *end;
  Side side = game->WhosTurn;

  // Don't perform alpha/beta pruning for now.

  end = AllMoves(moves, game);

  *count = end-start;

  // Iterate through all moves looking for the best, whose definition
  // varies based on who's turn it is.

  max = side == White ? SMALL : BIG;
  best = INVALID_MOVE;

  for(curr = start; curr != end; curr++) {
    DoMove(game, *curr);

    val = miniMax(game, SMALL, BIG, 1, count);

    if((side == White && val > max) || (side == Black && val < max)) {
      max = val;
      best = *curr;
    }

    Unmove(game);
  }

  if(best == INVALID_MOVE) {
    panic("No move selected!");
  }

  return best;
}
