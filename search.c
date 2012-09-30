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

// Use breadth-first search.
#define BREADTH_EXPLAIN

// Output EXPLAIN: xxx for every node.
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
static void        correctDepth(int, SearchNode*);
static void        doExplain(int, SearchNode*);
static SearchNode* newNode(int, int, int, Side, SearchNode*, MemorySlice*, int);
static void        allocateChildren(SearchNode*, size_t);

#endif

#if defined(SHOW_LINES)
#define MAX_LINES 200
#define MAX_DEPTH 50

static Move lines[MAX_LINES][MAX_DEPTH];
#endif

// TODO: Remove all these statics :'-(.
static Move     bestMove;
static uint64_t *iterCount;
static bool     stop;
static int      currDepth;
static int      historyOffset;

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

  // Allow the search to stop. TODO: Remove this hacktasticness.
  sleep(1);

  return bestMove;
}

Move
Search(Game *game, uint64_t *count, int *value, int depth)
{
  int max, val;
  int i;
  Move move = INVALID_MOVE;
  Move moves[INIT_MOVE_LEN];
  Move best;
  Move *start = moves;
  Move *curr, *end;

#if defined(EXPLAIN)
  SearchNode *node = newNode(SMALL, BIG, depth, game->WhosTurn, NULL, NULL, 0);
#endif

#if defined(SHOW_LINES)
  int j;

  int selectedLine = 0;

  // Reset.
  for(i = 0; i < MAX_LINES; i++) {
    for(j = 0; j < MAX_DEPTH; j++) {
      lines[i][j] = INVALID_MOVE;
    }
  }
#endif

  currDepth     = depth;
  historyOffset = game->Memories.Curr - game->Memories.Vals;

  end = AllMoves(moves, game);

#if defined(EXPLAIN)
  allocateChildren(node, end-start);
#endif

  *count = end-start;

  // Iterate through all moves looking for the best scoring one.

  max  = -INT_MAX;
  best = INVALID_MOVE;

  for(i = 0, curr = start; curr != end; i++, curr++) {
    move = *curr;

    DoMove(game, move);

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

#if defined(EXPLAIN)
      node->CurrChild--;
      (*node->CurrChild)->AlphaBeat = true;
      node->CurrChild++;
#endif

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
    printf("Line %d selected.\n", selectedLine);

    for(i = 0; i < end - start; i++) {
      printf("%d: ", i);

      for(j = 0; j < depth; j++) {
        printf("%s ", StringMove(lines[i][j]));
      }

      printf("\n");
    }
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
#if defined(SHOW_LINES)
  int i, j;
  int64_t offset;
  MemorySlice *history = &game->Memories;
  Memory *currMem;
#endif

#ifndef DISABLE_TRANS
  TransEntry *entry;
#endif

#if defined(EXPLAIN)
  // Set initial values here in case we are stopped.
  SearchNode *node = newNode(alpha, beta, depth, game->WhosTurn, parentNode, &game->Memories,
                             historyOffset);
#endif

  if(stop) {
#if defined(EXPLAIN)
    node->Leaf    = true;
    node->Stopped = true;
    node->Value   = alpha;
#endif

    return alpha;
  }

  if(depth == 0) {
#if defined(EXPLAIN)
    node->Leaf = true;

    // One child for quiescing!
    allocateChildren(node, 1);
    // Mark with invalid move.
    *node->CurrMove = INVALID_MOVE;
#endif

    val = quiesce(game, alpha, beta, -1, count
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
    node->Leaf  = true;
    node->Value = val;
#endif

#if defined(SHOW_LINES)
    if(val > alpha && val <= beta) {
      offset = history->Curr - history->Vals;
      offset -= currDepth - depth;

      for(currMem = history->Vals + offset, i = 0;
          currMem != history->Curr && i < currDepth - depth; currMem++, i++) {
        lines[lineInd][i] = currMem->Move;
      }

      for(i = currDepth - depth; i < currDepth; i++) {
        lines[lineInd][i] = INVALID_MOVE;
      }
    }
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
      *node->CurrChild = newNode(-beta, -alpha, depth-1, game->WhosTurn, node,
                                 &game->Memories, historyOffset);
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

#if defined(SHOW_LINES)
    if(depth == 1 && val > alpha && val < beta) {
      offset = (history->Curr - history->Vals) - 1;
      offset -= currDepth - depth;

      j = 0;
      for(currMem = history->Vals + offset; currMem != history->Curr; currMem++) {
        lines[lineInd][j] = currMem->Move;
        j++;
      }
    }
#endif

    Unmove(game);

    if(val > alpha) {
      alpha = val;

#if defined(EXPLAIN)
      node->CurrChild--;
      (*node->CurrChild)->AlphaBeat = true;
      node->CurrChild++;
#endif
    }

    // Fail high.
    if(val >= beta) {
#if defined(EXPLAIN)
      node->CurrChild--;
      (*node->CurrChild)->BetaPruned = true;
      (*node->CurrChild)->Leaf       = true;
      node->CurrChild++;

      node->Value = beta;
#endif

      return beta;
    }
  }

#if defined(EXPLAIN)
  node->Value = alpha;
  node->Alpha = alpha;
  node->Beta  = beta;
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
  SearchNode *node = newNode(alpha, beta, depth, game->WhosTurn, parentNode,
                             &game->Memories, historyOffset);
  node->Quiesce = true;
#endif

  if(stop) {
#if defined(EXPLAIN)
    node->Leaf    = true;
    node->Stopped = true;
    node->Value   = beta;
#endif

    return beta;
  }

  // Fail high.
  if(standPat >= beta) {
#if defined(EXPLAIN)
    node->Leaf       = true;
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
      *node->CurrChild = newNode(-beta, -alpha, depth-1, game->WhosTurn, node,
                                 &game->Memories, historyOffset);
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
      (*node->CurrChild)->Leaf       = true;
      node->CurrChild++;

      node->Value = beta;
#endif

      return beta;
    }

    if(val > alpha) {
      alpha = val;

#if defined(EXPLAIN)
      node->CurrChild--;
      (*node->CurrChild)->AlphaBeat = true;
      node->CurrChild++;
#endif
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

#if defined(EXPLAIN_OUTPUT)
static char*
stringNode(SearchNode *node)
{
  int i;
  StringBuilder builder = NewStringBuilder();

  Move *moves = UnpackMoveHistory(&node->MoveHistory, true);
  for(i = 0; i < node->Depth; i++) {
    AppendString(&builder, "%s ", StringMove(moves[i]));
  }
  release(moves);

  AppendString(&builder, "(%d%c) [", node->Value, node->Side == White ? 'w' : 'b');

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

  if(node->Quiesce || node->AlphaBeat || node->BetaPruned || node->Stopped ||
     node->TransHit || node->Leaf) {
    AppendString(&builder, " ");
  }

  if(node->Quiesce) {
    AppendString(&builder, "Q");
  }
  if(node->AlphaBeat) {
    AppendString(&builder, "a");
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
  if(node->Leaf) {
    AppendString(&builder, "L");    
  }

  AppendString(&builder, "]");

  return BuildString(&builder, true);
}
#endif

static uint64_t betaPruneCount, nodeCount, qBetaPruneCount, quiesceCount, stopCount,
  transHitCount;

static void
updateCounts(SearchNode *node)
{
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
}

#if defined(BREADTH_EXPLAIN)
static List *nodeList;

static void
walk(SearchNode *node)
{
  SearchNode *child;
  SearchNode **childPtr = node->Children;

  updateCounts(node);

#if defined(EXPLAIN_OUTPUT)
  printf("EXPLAIN: %s\n", stringNode(node));
#endif

  while(childPtr && (child = *childPtr++)) {
    PushFront(nodeList, child);
  }

  while(nodeList->Count > 0) {
    child = PopBack(nodeList);

    walk(child);
  }
}
#else
static void
walk(SearchNode *node)
{
  SearchNode *child;
  SearchNode **childPtr = node->Children;

  updateCounts(node);

#if defined(EXPLAIN_OUTPUT)
  printf("EXPLAIN: %s\n", stringNode(node));
#endif

  while(childPtr && (child = *childPtr++)) {
    walk(child);
  }
}
#endif

static void
doExplain(int depth, SearchNode *root)
{
  printf("EXPLAIN: Search results for depth %d:-\n", depth);

#if defined(BREADTH_EXPLAIN)
  nodeList = NewList();
#endif

  walk(root);

  printf("EXPLAIN: %llu nodes, %llu beta prunes, %llu quiesces (%llu beta pruned), "
         "%llu stops, %llu trans hits.\n",
         nodeCount, betaPruneCount, quiesceCount, qBetaPruneCount, stopCount, transHitCount);

  puts("EXPLAIN: Done.");
}

static SearchNode*
newNode(int alpha, int beta, int depth, Side side, SearchNode *parent, MemorySlice *history,
        int offset)
{
  SearchNode *node = allocateZero(sizeof(SearchNode), 1);

  node->Alpha      = alpha;
  node->Beta       = beta;
  node->Depth      = depth;
  node->Side       = side;

  if(history != NULL) {
    node->MoveHistory = PackMoveHistory(history, offset);
  }

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
