#include <stdio.h>
#include "weak.h"

//#define SHOW_MOVES

static PerftStats initStats(void);

uint64_t
QuickPerft(Game *game, int depth)
{
#if defined(SHOW_MOVES)
  bool capture;
  Piece piece;
#endif
  Move move;
  Move *curr, *end;
  Move buffer[INIT_MOVE_LEN];

  uint64_t ret = 0;

  end = AllMoves(buffer, game);

  if(depth <= 1) {
#if defined(SHOW_MOVES)
    for(curr = buffer; curr < end; curr++) {
      move = *curr;

      piece = PieceAt(&game->ChessSet, FROM(move));

      if(TYPE(move) == EnPassant) {
        capture = true;
      } else {
        capture = PieceAt(&game->ChessSet, TO(move)) != MissingPiece;
      }

      puts(StringMove(move, piece, capture));
    }
#endif

    return end-buffer;
  }

  for(curr = buffer; curr < end; curr++) {
    move = *curr;

    DoMove(game, move);
    ret += QuickPerft(game, depth - 1);
    Unmove(game);
  }

  return ret;
}

PerftStats
Perft(Game *game, int depth)
{
#if defined(SHOW_MOVES)
  bool capture;
  Piece piece;
#endif
  Move move;
  Move buffer[INIT_MOVE_LEN];
  Move *curr, *end;
  PerftStats ret, stats;

  if(depth <= 0) {
    panic("Invalid depth %d.", depth);
  }

  ret = initStats();

  end = AllMoves(buffer, game);

  for(curr = buffer; curr != end; curr++) {
    move = *curr;

    if(depth == 1) {
#if defined(SHOW_MOVES)
      piece = PieceAt(&game->ChessSet, FROM(move));

      if(TYPE(move) == EnPassant) {
        capture = true;
      } else {
        capture = PieceAt(&game->ChessSet, TO(move)) != MissingPiece;
      }

      puts(StringMove(move, piece, capture));
#endif
      ret.Count++;
      /*
      if(CAPTURE(move)) {
        ret.Captures++;
      }
      */
      switch(TYPE(move)) {
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
        panic("Invalid move type %d.", TYPE(move));
      }

      DoMove(game, move);
      if(game->CheckStats.CheckSources != EmptyBoard) {
        ret.Checks++;

        if(Checkmated(game)) {
          ret.Checkmates++;
        }
      }
      Unmove(game);
    } else {
      DoMove(game, move);
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
