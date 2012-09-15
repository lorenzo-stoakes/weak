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

uint64_t
HashGame(Game *game)
{
  BitBoard bitBoard;
  Piece piece;
  Position pos;
  Side side;

  CastleSide castleSide;
  uint64_t ret = 0;

  for(side = White; side <= Black; side++) {
    for(castleSide = KingSide; castleSide <= QueenSide; castleSide++) {
      if(game->CastlingRights[side][castleSide]) {
        ret ^= ZobristCastlingHash[side][castleSide];
      }
    }
  }

  if(game->EnPassantSquare != EmptyPosition) {
    ret ^= ZobristEnPassantFileHash[FILE(game->EnPassantSquare)];
  }

  for(side = White; side <= Black; side++) {
    for(piece = Pawn; piece <= King; piece++) {
      bitBoard = game->ChessSet.Sets[side].Boards[piece];

      while(bitBoard) {
        pos = PopForward(&bitBoard);

        ret ^= ZobristPositionHash[side][piece][pos];
      }
    }
  }

  ret ^= (side == Black) * ZobristBlackHash;

  return ret;
}

void
InitZobrist()
{
  int i;
  File file;
  Piece piece;
  Position pos;
  Side side;

  for(side = White; side <= Black; side++) {
    for(i = 0; i < 2; i++) {
      ZobristCastlingHash[side][i] = randk();
    }
  }

  for(file = FileA; file <= FileH; file++) {
    ZobristEnPassantFileHash[file] = randk();
  }

  for(side = White; side <= Black; side++) {
    for(piece = Pawn; piece <= King; piece++) {
      for(pos = A1; pos <= H8; pos++) {
        ZobristPositionHash[side][piece][pos] = randk();
      }
    }
  }

  ZobristBlackHash = randk();
}
