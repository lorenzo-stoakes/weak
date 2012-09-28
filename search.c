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

// Examine node by node.
#define EXPLAIN
//#define EXPLAIN_OUTPUT
#include <stdio.h>
#include <unistd.h> // For sleep().

#include "weak.h"

static int quiesce(Game*, int, int, int, uint64_t*
#if defined(EXPLAIN)
                   , SearchNode*
#endif
                   );
static int negaMax(Game*, int, int, int, uint64_t*, int
#if defined(EXPLAIN)
                   , SearchNode*
#endif
                   );

#if defined(EXPLAIN)
static void correctDepth(int, SearchNode*);
static void doExplain(int, SearchNode*);

static SearchNode*
newNode(int alpha, int beta, int depth, Side side, SearchNode *parent)
{
  SearchNode *node = allocate(sizeof(SearchNode), 1);

  node->Alpha      = alpha;
  node->Beta       = beta;
  node->Depth      = depth;
  node->BetaPruned = false;
  node->Quiesce    = false;
  node->Stopped    = false;
  node->TransHit   = false;
  node->Children   = NULL;
  node->CurrChild  = NULL;
  node->Moves      = NULL;
  node->CurrMove   = NULL;
  node->Side       = side;

  if(parent != NULL) {
    *parent->CurrChild++ = node;
  }

  return node;
}

static void
allocateChildren(SearchNode *node, size_t size)
{
  // +1 so we can null-terminate.
  node->Children  = allocateZero(sizeof(SearchNode*), size+1);
  node->CurrChild = node->Children;

  node->Moves    = allocateZero(sizeof(Move), size+1);
  node->CurrMove = node->Moves;
}

#endif

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

#if defined(EXPLAIN)
  SearchNode *node = newNode(SMALL, BIG, depth, game->WhosTurn, NULL);
#endif
  end = AllMoves(moves, game);

#if defined(EXPLAIN)
  allocateChildren(node, end-start);
#endif

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
    val = -negaMax(game, SMALL, BIG, depth-1, count, i
#if defined(EXPLAIN)
                   , node
#endif
                   );

#if defined(EXPLAIN)
    *node->CurrMove++ = move;
#endif

    if(val > max) {
      max = val;
      best = move;
#if defined(SHOW_LINES)
      selectedLine = i;
#endif
    }

    Unmove(game);
  }

#if defined(EXPLAIN)
  node->Value = max;

  // Correct depth values so ascending, not descending.
  correctDepth(depth, node);

  doExplain(depth, node);
#endif

  if(stop) {
    return INVALID_MOVE;
  }

  if(move == INVALID_MOVE) {
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
negaMax(Game *game, int alpha, int beta, int depth, uint64_t *count, int lineInd
#if defined(EXPLAIN)
        ,SearchNode *parentNode
#endif
        )
{
  int val;
  Move move;
  Move moves[INIT_MOVE_LEN];
  Move *start = moves;
  Move *curr, *end;
#ifndef DISABLE_TRANS
  TransEntry *entry;
#endif

#if defined(EXPLAIN)
  // Set initial values here in case we are stopped.
  SearchNode *node = newNode(alpha, beta, depth, game->WhosTurn, parentNode);
#endif

  if(stop) {
#if defined(EXPLAIN)
    node->Stopped = true;
    node->Value = alpha;
#endif

    return alpha;
  }

  if(depth == 0) {
#if defined(EXPLAIN)
    // One child for quiescing!
    allocateChildren(node, 1);
    // Mark with invalid move.
    *node->CurrMove = INVALID_MOVE;
#endif

    val = quiesce(game, alpha, beta, 0, count
#if defined(EXPLAIN)
                  , node
#endif
                  );

#if defined(EXPLAIN)
    node->Value = val;
#endif

    return val;
  }

  end = AllMoves(moves, game);

  // If no moves available, all we can do is eval. Stalemate or checkmate.
  if(end == moves) {
    val = Eval(game);

#if defined(EXPLAIN)
    node->Value = val;
#endif


    return val;
  }

  *count += end - start;

#if defined(EXPLAIN)
  allocateChildren(node, end-start);
#endif

  for(curr = start; curr != end; curr++) {
    move = *curr;

    DoMove(game, move);

#if defined(EXPLAIN)
    *node->CurrMove++ = move;
#endif

#ifndef DISABLE_TRANS
    if((entry = LookupPosition(game->Hash)) && entry->Depth >= depth) {
      UpdateGeneration(entry);
      entry->QuickMove = (QuickMove)move;
      val = entry->Value;

#if defined(EXPLAIN)
      *node->CurrChild = newNode(-beta, -alpha, depth-1, game->WhosTurn, node);
      node->CurrChild--;
      (*node->CurrChild)->TransHit = true;
      node->CurrChild++;
#endif

    } else {
#endif

      val = -negaMax(game, -beta, -alpha, depth-1, count, lineInd
#if defined(EXPLAIN)
                   , node
#endif
                    );

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
#if defined(EXPLAIN)
      node->CurrChild--;
      (*node->CurrChild)->BetaPruned = true;
      node->CurrChild++;

      node->Value = beta;
#endif

      return beta;
    }
  }

#if defined(EXPLAIN)
  node->Value = alpha;
#endif

  return alpha;
}

// Quiescent search. See http://chessprogramming.wikispaces.com/Quiescence+Search
static int
quiesce(Game *game, int alpha, int beta, int depth, uint64_t *count
#if defined(EXPLAIN)
        , SearchNode *parentNode
#endif
        )
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

#if defined(EXPLAIN)
  // Set initial values here in case we are stopped.
  SearchNode *node = newNode(alpha, beta, depth, game->WhosTurn, parentNode);
  node->Quiesce = true;
#endif

  if(stop) {
#if defined(EXPLAIN)
    node->Stopped = true;
    node->Value = beta;
#endif

    return beta;
  }

  // Fail high.
  if(standPat >= beta) {
#if defined(EXPLAIN)
    node->BetaPruned = true;
    node->Value      = beta;
#endif

    return beta;
  }

  if(alpha < standPat) {
    alpha = standPat;
  }

  end = AllCaptures(curr, game);

#if defined(EXPLAIN)
  allocateChildren(node, end-curr);
#endif

  *count += end-curr;

  for(; curr != end; curr++) {
    move = *curr;

    DoMove(game, move);

#if defined(EXPLAIN)
    *node->CurrMove++ = move;
#endif

#ifndef DISABLE_TRANS
    if((entry = LookupPosition(game->Hash))) {
      UpdateGeneration(entry);
      entry->QuickMove = (QuickMove)move;
      val = entry->Value;

#if defined(EXPLAIN)
      *node->CurrChild = newNode(-beta, -alpha, depth-1, game->WhosTurn, node);
      node->CurrChild--;
      (*node->CurrChild)->TransHit = true;
      node->CurrChild++;
#endif
    } else {
#endif
      val = -quiesce(game, -beta, -alpha, depth-1, count
#if defined(EXPLAIN)
                   , node
#endif
                    );
#ifndef DISABLE_TRANS
      if(!stop) {
        SavePosition(game->Hash, val, (QuickMove)move, 0);
      }
    }
#endif

    Unmove(game);

    // Fail high.
    if(val >= beta) {
#if defined(EXPLAIN)
      node->CurrChild--;
      (*node->CurrChild)->BetaPruned = true;
      node->CurrChild++;

      node->Value = beta;
#endif

      return beta;
    }

    if(val > alpha) {
      val = alpha;
    }
  }

#if defined(EXPLAIN)
  node->Value = alpha;
#endif

  return alpha;
}

#if defined(EXPLAIN)

static void
correctDepth(int depth, SearchNode *node)
{
  SearchNode *child;
  SearchNode **childPtr;

  if(node == NULL) {
    return;
  }

  childPtr = node->Children;

  node->Depth = depth - node->Depth;

  while(childPtr && (child = *childPtr++)) {
    correctDepth(depth, child);
  }
}

static Move prevMoves[100];

#if defined(EXPLAIN_OUTPUT)
static char*
stringNode(SearchNode *node)
{
  int i;
  StringBuilder builder = NewStringBuilder();

  for(i = 0; i < node->Depth; i++) {
    AppendString(&builder, "%s ", StringMove(prevMoves[i]));
  }

  AppendString(&builder, "(%d) [", node->Value);

  if(node->Alpha == SMALL) {
    AppendString(&builder, "- ");
  } else {
    AppendString(&builder, "%d ", node->Alpha);
  }

  if(node->Beta == BIG) {
    AppendString(&builder, "+");
  } else {
    AppendString(&builder, "%d", node->Beta);
  }

  if(node->Quiesce || node->BetaPruned || node->Stopped || node->TransHit) {
    AppendString(&builder, " ");
  }

  if(node->Quiesce) {
    AppendString(&builder, "Q");
  }
  if(node->BetaPruned) {
    AppendString(&builder, "b");
  }
  if(node->Stopped) {
    AppendString(&builder, "x");
  }
  if(node->TransHit) {
    AppendString(&builder, "t");
  }

  AppendString(&builder, "] ");

  return BuildString(&builder, true);
}
#endif

static uint64_t betaPruneCount, nodeCount, qBetaPruneCount, quiesceCount, stopCount,
  transHitCount;

static void
walk(SearchNode *node)
{
  SearchNode *child;
  SearchNode **childPtr = node->Children;
  Move *movePtr = node->Moves;
  Move move;

  if(!node->Quiesce) {
    nodeCount++;
  }

  if(node->BetaPruned) {
    if(node->Quiesce) {
      qBetaPruneCount++;
    } else {
      betaPruneCount++;
    }
  }
  if(node->Quiesce) {
    quiesceCount++;
  }
  if(node->Stopped) {
    stopCount++;
  }
  if(node->TransHit) {
    transHitCount++;
  }

#if defined(EXPLAIN_OUTPUT)
  printf("EXPLAIN: %s\n", stringNode(node));
#endif

  while(childPtr && (child = *childPtr++)) {
    if(movePtr) {
      move = *movePtr++;
    } else {
      move = INVALID_MOVE;
    }
    prevMoves[child->Depth-1] = move;

    walk(child);
  }
}

static void
doExplain(int depth, SearchNode *root)
{
  printf("EXPLAIN: Search results for depth %d:-\n", depth);

  walk(root);

  printf("EXPLAIN: %llu nodes, %llu beta prunes, %llu quiesces (%llu beta pruned), "
         "%llu stops, %llu trans hits.\n",
         nodeCount, betaPruneCount, quiesceCount, qBetaPruneCount, stopCount, transHitCount);

  puts("EXPLAIN: Done.");
}

#endif
