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

//#define SHOW_LINES
//#define DISABLE_TRANS

#include <stdio.h>
#include <unistd.h> // For sleep().


#include "weak.h"

static int quiesce(Game*, int, int, uint64_t*);
static int negaMax(Game*, int, int, int, uint64_t*, int);

#if defined(SHOW_LINES)
static Move lines[200][10];
#endif

// TODO: Remove all these statics :'-(.
static Move bestMove;
static uint64_t *iterCount;
static bool stop;

static void*
doIterSearch(void *gameVoid)
{
  int depth = 1;
  int val;
  uint64_t currCount;
  Move move;

  Game *game = (Game*)gameVoid;

  while(!stop) {
    move = Search(game, &currCount, &val, depth);
    *iterCount += currCount;

    if(!stop) {
      bestMove = move;
      fprintf(stderr, "Depth %d, bestmove=%s,val=%d.\n", depth, StringMove(bestMove), val);
    }

    depth++;
  }

  return NULL;
}

Move
IterSearch(Game *game, uint64_t *count, uint16_t maxSeconds)
{
// TODO: Hackish.

  iterCount = count;

  stop = false;

  if(CreateThread(&doIterSearch, game)) {
    panic("Error creating thread.");
  }

  sleep(maxSeconds);

  stop = true;

  // Allow the search to stop.
  sleep(1);

  return bestMove;
}

Move
Search(Game *game, uint64_t *count, int *value, int depth)
{
  int max, val;
  int i;
#if defined(SHOW_LINES)
  int selectedLine = 0;
#endif
  Move move = INVALID_MOVE;
  Move moves[INIT_MOVE_LEN];
  Move best;
  Move *start = moves;
  Move *curr, *end;

  end = AllMoves(moves, game);

  *count = end-start;

  // Iterate through all moves looking for the best, whose definition
  // varies based on who's turn it is.

  max = -INT_MAX;
  best = INVALID_MOVE;

  for(i = 0, curr = start; curr != end; i++, curr++) {
    move = *curr;

    DoMove(game, move);

#if defined(SHOW_LINES)
    lines[i][depth-1] = move;
#endif

    val = -negaMax(game, SMALL, BIG, depth-1, count, i);

    if(val > max) {
      max = val;
      best = move;
#if defined(SHOW_LINES)
      selectedLine = i;
#endif
    }

    Unmove(game);
  }

  if(best == INVALID_MOVE) {
    panic("No move selected!");
  }

#if defined(SHOW_LINES)
  if(!stop) {
    for(i = depth-1; i >= 0; i--) {
      printf("%s ", StringMove(lines[selectedLine][i]));
    }

    printf("\n");
  }
#endif

  *value = max;

  return best;
}

static int
negaMax(Game *game, int alpha, int beta, int depth, uint64_t *count, int lineInd)
{
  int val;
  Move move;
  Move moves[INIT_MOVE_LEN];
  Move *start = moves;
  Move *curr, *end;
#ifndef DISABLE_TRANS
  TransEntry *entry;
#endif

  if(stop) {
    return beta;
  }

  if(depth == 0) {
    return quiesce(game, alpha, beta, count);
  }

  end = AllMoves(moves, game);

  // If no moves available, all we can do is eval. Stalemate or checkmate.
  if(end == moves) {
    return Eval(game);
  }

  *count += end - start;

  for(curr = start; curr != end; curr++) {
    move = *curr;

    DoMove(game, move);

#ifndef DISABLE_TRANS
    if((entry = LookupPosition(game->Hash)) && entry->Depth >= depth) {
      UpdateGeneration(entry);
      entry->QuickMove = (QuickMove)move;
      val = entry->Value;
    } else {
#endif
      val = -negaMax(game, -beta, -alpha, depth-1, count, lineInd);
#ifndef DISABLE_TRANS
      if(!stop) {
        SavePosition(game->Hash, val, (QuickMove)move, depth);
      }
    }
#endif

    Unmove(game);

    if(val >= alpha) {
#if defined(SHOW_LINES)
    lines[lineInd][depth-1] = move;
#endif

      alpha = val;
    }

    // Fail high.
    if(val >= beta) {
      return val;
    }
  }

  return alpha;
}

// Quiescent search. See http://chessprogramming.wikispaces.com/Quiescence+Search
static int
quiesce(Game *game, int alpha, int beta, uint64_t *count)
{
  int val;
  int standPat = Eval(game);
  Move buffer[INIT_MOVE_LEN];
  Move move;
  Move *end;
  Move *curr = buffer;
#ifndef DISABLE_TRANS
  TransEntry *entry;
#endif

  if(stop) {
    return beta;
  }

  // Fail high.
  if(standPat >= beta) {
    return beta;
  }

  if(alpha < standPat) {
    alpha = standPat;
  }

  end = AllCaptures(curr, game);

  *count += end-curr;

  for(; curr != end; curr++) {
    move = *curr;

    DoMove(game, move);

#ifndef DISABLE_TRANS
    if((entry = LookupPosition(game->Hash))) {
      UpdateGeneration(entry);
      entry->QuickMove = (QuickMove)move;
      val = entry->Value;
    } else {
#endif
      val = -quiesce(game, -beta, -alpha, count);
#ifndef DISABLE_TRANS
      if(!stop) {
        SavePosition(game->Hash, val, (QuickMove)move, 0);
      }
    }
#endif

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
