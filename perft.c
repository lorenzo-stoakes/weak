/*
  Weak, a chess perft calculator derived from Stockfish.

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
      if(Checked(game)) {
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
