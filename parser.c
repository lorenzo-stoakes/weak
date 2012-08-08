/*
  Weak, a chess engine derived from Stockfish.

  Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
  Copyright (C) 2008-2012 Marco Costalba, Joona Kiiski, Tord Romstad (Stockfish authors)
  Copyright (C) 2011-2012 Lorenzo Stoakes

  Stockfish is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Stockfish is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <ctype.h>
#include <string.h>
#include "weak.h"

// Parse a FEN string into a game object.
// See http://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation
Game
ParseFen(char *fen)
{
  bool seenKing[2] = { false, false };
  char chr;
  int i, len, pieceCount;
  // Make file/rank integers so we can make them negative to detect errors. Both are unsigned.
  // TODO: fix.
  int file, rank;
  Game ret = NewEmptyGame(false, White);
  Piece piece;
  Position king, pos;
  Side side;

  if(fen == NULL) {
    panic("Null char pointer in ParseFen().");
  }

  len = strlen(fen);

  if(len < 25) {
    panic("Invalid FEN '%s' - too short. Expect at least 25 characters.", fen);
  }

  rank = Rank8;
  file = FileA;
  for(i = 0; i < len; i++) {
    chr = fen[i];

    if(chr >= '1' && chr <= '8') {
      file += (chr - '0');
      continue;
    } else if(chr == '/') {
      rank--;
      file = FileA;
      continue;
    } else if(rank == Rank1 && chr == ' ') {
      break;
    } else {
      side = isupper(chr) ? White : Black;
      switch(chr) {
      case 'P':
      case 'p':
        piece = Pawn;
        break;
      case 'R':
      case 'r':
        piece = Rook;
        break;
      case 'N':
      case 'n':
        piece = Knight;
        break;
      case 'B':
      case 'b':
        piece = Bishop;
        break;
      case 'Q':
      case 'q':
        piece = Queen;
        break;
      case 'K':
      case 'k':
        piece = King;
        seenKing[side] = true;
        break;
      default:
        panic("Unrecognised character '%c' at position %d.", chr, i);
      }
    }

    if(rank < 0) {
      panic("Too many ranks at position %d.", i);
    }
    if(file < 0) {
      panic("Too many pieces in the file at position %d.", i);
    }

    pos = POSITION(rank, file);
    ret.ChessSet.Sets[side].Boards[piece] |= POSBOARD(pos);

    ret.ChessSet.Squares[pos] = piece;

    pieceCount = ret.ChessSet.PieceCounts[side][piece];
    ret.ChessSet.PiecePositionIndexes[pos] = pieceCount;
    ret.ChessSet.PiecePositions[side][piece][pieceCount] = pos;
    ret.ChessSet.PieceCounts[side][piece]++;

    file++;
  }

  for(side = White; side <= Black; side++) {
    if(!seenKing[side]) {
      panic("No %s king.", StringSide(side));
    }
  }

  if(i+2 >= len) {
    panic("FEN string does not contain enough characters to determine position.");
  }
  i++;

  // Who's turn.

  switch(fen[i]) {
  case 'w':
    ret.WhosTurn = White;
    break;
  case 'b':
    ret.WhosTurn = Black;
    break;
  default:
    panic("No turn indicator at position %d.", i);
  }

  i += 2;
  if(fen[i] != '-') {
    for(; i < len && i != ' '; i++) {
      chr = fen[i];

      switch(chr) {
      case 'K':
        ret.CastlingRights[White][KingSide] = true;
        break;
      case 'k':
        ret.CastlingRights[Black][KingSide] = true;
        break;
      case 'Q':
        ret.CastlingRights[White][QueenSide] = true;
        break;
      case 'q':
        ret.CastlingRights[Black][QueenSide] = true;
        break;
      case ' ':
        goto done;
      default:
        panic("Unrecognised character '%c' at position %d.", chr, i);
      }
    }
  }

  if(i == len) {
    panic("FEN '%s' ended without en passant square.", fen);
  }

  if(fen[i] != '-') {
    if(len - i < 2) {
      panic("Not enough space in fen string for non-empty en passant square.");
    }

    chr = fen[i];

    if(chr < 'a' || chr > 'h') {
      panic("Invalid file '%c'.", chr);
    }
    file = chr - 'a';

    if(chr < '1' || chr > '8') {
      panic("Invalid rank '%c'.", chr);
    }
    rank = chr - '1';

    ret.EnPassantSquare = POSITION(file, rank);
  }

  // TODO: Implement parsing of clock times.

 done:
  UpdateOccupancies(&ret.ChessSet);

  king = BitScanForward(ret.ChessSet.Sets[ret.WhosTurn].Boards[King]);
  ret.CheckStats = CalculateCheckStats(&ret);
  ret.CheckStats.CheckSources = AllAttackersTo(&ret.ChessSet, king, ret.ChessSet.Occupancy) &
    ret.ChessSet.Sets[OPPOSITE(ret.WhosTurn)].Occupancy;

  return ret;
}
