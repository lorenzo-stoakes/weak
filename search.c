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

#define PRINT_PV

#if defined(PRINT_PV)
#include <stdio.h>
#endif

#include <time.h>
#include "weak.h"

static double quiesce(Game*, double, double, uint64_t*);
static double negaMax(Game*, double, double, int, uint64_t*);

#if defined(PRINT_PV)
static Move pv[218][30];
static int pvBestVar;
static int pvCurrVar;
static int pvIndex;
static int pvLength;
#endif

Move
IterSearch(Game *game, uint64_t *count, uint32_t maxSecs)
{
  clock_t ticks;
  uint32_t elapsed = 0;
  int depth;
  Move best = INVALID_MOVE;
  uint64_t currCount = 0;

  ticks = clock();
  // We run through this loop at least once.
  for(depth = 1; 30*elapsed < maxSecs; depth++) {
#if defined(PRINT_PV)
    pvCurrVar = 0;
    pvBestVar = -1;
    pvLength = depth;
#endif
    best = Search(game, &currCount, depth);
    *count += currCount;

    elapsed = (clock() - ticks)/CLOCKS_PER_SEC;
  }

#if defined(PRINT_PV)
  // Output PV.
  for(depth = 0; depth < pvLength; depth++) {
    printf("%s ", StringMove(pv[pvBestVar][depth]));
  }
  printf("\n");
#endif

  return best;
}

Move
Search(Game *game, uint64_t *count, int depth)
{
  double max, val;
  Move moves[INIT_MOVE_LEN];
  Move best;
  Move *start = moves;
  Move *curr, *end;
  Side side = game->WhosTurn;

  end = AllMoves(moves, game);

  *count = end-start;

  // Iterate through all moves looking for the best, whose definition
  // varies based on who's turn it is.

  max = side == White ? SMALL : BIG;
  best = INVALID_MOVE;

  for(curr = start; curr != end; curr++) {
    DoMove(game, *curr);

#if defined(PRINT_PV)
    pvIndex = 1;
    pv[pvCurrVar][0] = *curr;
#endif

    val = negaMax(game, SMALL, BIG, depth-1, count);

    if((side == White && val > max) || (side == Black && val < max)) {
      max = val;
      best = *curr;
#if defined(PRINT_PV)
      pvBestVar = pvCurrVar;
#endif
    }

#if defined(PRINT_PV)    
      pvCurrVar++;    
#endif

    Unmove(game);
  }

  if(best == INVALID_MOVE) {
    panic("No move selected!");
  }

  return best;
}

static double
negaMax(Game *game, double alpha, double beta, int depth, uint64_t *count)
{
  double val;
  Move moves[INIT_MOVE_LEN];
  Move *start = moves;
  Move *curr, *end;

  if(depth == 0) {
    return quiesce(game, alpha, beta, count);
  }

  end = AllMoves(moves, game);

  *count += end - start;

  // Iterate through all moves looking for the best, whose definition
  // varies based on who's turn it is.

  for(curr = start; curr != end; curr++) {
    DoMove(game, *curr);
#if defined(PRINT_PV)
    pvIndex++;
#endif
    val = -negaMax(game, -beta, -alpha, depth-1, count);
#if defined(PRINT_PV)
    pvIndex--;
#endif
    Unmove(game);

    // Fail high.
    if(val >= beta) {
      return val;
    }

    if(val >= alpha) {
#if defined(PRINT_PV)
      pv[pvCurrVar][pvIndex] = *curr;
#endif

      alpha = val;
    }
  }

  return alpha;
}

// Quiescent search. See http://chessprogramming.wikispaces.com/Quiescence+Search
static double
quiesce(Game *game, double alpha, double beta, uint64_t *count)
{
  double val;
  double standPat = Eval(game);
  Move buffer[INIT_MOVE_LEN];
  Move move;
  Move *end;
  Move *curr = buffer;

  // Fail high.
  if(standPat >= beta) {
    return beta;
  }

  if(alpha < standPat) {
    alpha = standPat;
  }

  if(Checked(game)) {
    return alpha;
  } else {
    end = AllCaptures(curr, game);
  }

  *count += end-curr;

  for(; curr != end; curr++) {
    move = *curr;

    DoMove(game, move);
    val = -quiesce(game, -beta, -alpha, count);
    Unmove(game);

    // Fail high.
    if(val >= beta) {
      return beta;
    }

    if(val > alpha) {
      val = alpha;
    }
  }

  return alpha;
}
