#include "weak.h"

static PerftStats initStats(void);

uint64_t
QuickPerft(Game *game, int depth)
{
  int i;
  uint64_t ret = 0;
  MoveSlice allMoves;
  Move move;
  Move buffer[INIT_MOVE_LEN];

  allMoves = NewMoveSlice(buffer, INIT_MOVE_LEN);
  AllMoves(&allMoves, game);

  if(depth <= 1) {
    return allMoves.Len;
  }

  for(i = 0; i < allMoves.Len; i++) {
    if(depth == 1) {
    } else {
      move = allMoves.Vals[i];
      DoMove(game, &move);
      ret += QuickPerft(game, depth-1);
      Unmove(game);
    }
  }

  return ret;
}

PerftStats
Perft(Game *game, int depth)
{
  int i;
  Move move;
  MoveSlice allMoves;
  Move buffer[INIT_MOVE_LEN];
  PerftStats ret, stats;

  if(depth <= 0) {
    panic("Invalid depth %d.", depth);
  }

  ret = initStats();

  allMoves = NewMoveSlice(buffer, INIT_MOVE_LEN);
  AllMoves(&allMoves, game);

  for(i = 0; i < allMoves.Len; i++) {
    move = allMoves.Vals[i];

    if(depth == 1) {
      ret.Count++;
      if(move.Capture) {
        ret.Captures++;
      }
      switch(move.Type) {
      case CastleQueenSide:
      case CastleKingSide:
        ret.Castles++;
        break;
      case EnPassant:
        ret.EnPassants++;
        break;
      case PromoteKnight:
      case PromoteBishop:
      case PromoteRook:
      case PromoteQueen:
        ret.Promotions++;
        break;
      case Normal:
        break;
      default:
        panic("Invalid move type %d.", move.Type);
      }

      DoMove(game, &move);
      if(Checked(&game->ChessSet, game->WhosTurn)) {
        ret.Checks++;
        if(Checkmated(game)) {
          ret.Checkmates++;
        }
      }
      Unmove(game);

    } else {
      DoMove(game, &move);
      stats = Perft(game, depth - 1);
      Unmove(game);
      ret.Count += stats.Count;
      ret.Captures += stats.Captures;
      ret.EnPassants += stats.EnPassants;
      ret.Castles += stats.Castles;
      ret.Promotions += stats.Promotions;
      ret.Checks += stats.Checks;
      ret.Checkmates += stats.Checkmates;
    }
  }

  return ret;
}

static PerftStats
initStats()
{
  PerftStats ret;

  ret.Count = 0;
  ret.Captures = 0;
  ret.EnPassants = 0;
  ret.Castles = 0;
  ret.Promotions = 0;
  ret.Checks = 0;
  ret.Checkmates = 0;

  return ret;
}
