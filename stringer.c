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

#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include "weak.h"

char
CharPiece(Piece piece)
{
  switch(piece) {
  case Pawn:
    return 'P';
  case Rook:
    return 'R';
  case Knight:
    return 'N';
  case Bishop:
    return 'B';
  case Queen:
    return 'Q';
  case King:
    return 'K';
  default:
    return '?';
  }
}

char*
StringBitBoard(BitBoard bitBoard)
{
  const int LINE_LENGTH = 36;
  const int LINE_COUNT = 18;

  // TODO: Reduce duplication from StringChessSet.

  // Include space for newlines.
  char ret[LINE_LENGTH * LINE_COUNT + 1 + 1000];
  int file, offset, rank;
  Position pos;
  Side lineSide = Black;

  char *topLine    = "    A   B   C   D   E   F   G   H  \n";
  char *dottedLine = "  ---------------------------------\n";
  char *blackLine  = "  |   |...|   |...|   |...|   |...|\n";
  char *whiteLine  = "  |...|   |...|   |...|   |...|   |\n";

  // Add top lines.
  strncpy(ret, topLine, LINE_LENGTH);
  strncpy(ret+LINE_LENGTH, dottedLine, LINE_LENGTH);

  for(rank = Rank8; rank >= Rank1; rank--) {
    offset = 2*LINE_LENGTH + 2*LINE_LENGTH*(Rank8-rank);

    // Add line.
    strncpy(ret + offset, lineSide == White ? whiteLine : blackLine, LINE_LENGTH);

    // Add number.
    ret[offset] = '1'+rank;

    // Add pieces.

    // For the sake of debugging, avoid PieceAt() in case it is buggy.

    offset += 4;
    for(file = FileA; file <= FileH; file++) {
      pos = POSITION(rank, file);

      if((bitBoard&POSBOARD(pos)) == POSBOARD(pos)) {
        ret[offset] = 'X';
      }

      offset += 4;
    }

    // Add dotted line.
    strncpy(ret + offset, dottedLine, LINE_LENGTH);

    lineSide = OPPOSITE(lineSide);
  }

  ret[LINE_LENGTH * LINE_COUNT] = '\0';

  return strdup(ret);
}

// ASCII-art representation of chessboard.
char*
StringChessSet(ChessSet *chessSet)
{
  const int LINE_LENGTH = 36;
  const int LINE_COUNT = 18;

  // Include space for newlines.
  char ret[LINE_LENGTH * LINE_COUNT + 1 + 1000];
  char pieceChr;
  int file, offset, rank;
  Piece piece;
  Position pos;
  Side side;
  Side lineSide = Black;

  char *topLine    = "    A   B   C   D   E   F   G   H  \n";
  char *dottedLine = "  ---------------------------------\n";
  char *blackLine  = "  |   |...|   |...|   |...|   |...|\n";
  char *whiteLine  = "  |...|   |...|   |...|   |...|   |\n";

  // Add top lines.
  strncpy(ret, topLine, LINE_LENGTH);
  strncpy(ret+LINE_LENGTH, dottedLine, LINE_LENGTH);

  // Outputs chess set as ascii-art.
  // pieces are upper-case if white, lower-case if black.
  // P=pawn, R=rook, N=knight, B=bishop, Q=queen, K=king, .=empty square.

  // e.g., the initial position is output as follows:-

  //     A   B   C   D   E   F   G   H
  //   ---------------------------------
  // 8 | r |.n.| b |.q.| k |.b.| n |.r.|
  //   ---------------------------------
  // 7 |.p.| p |.p.| p |.p.| p |.p.| p |
  //   ---------------------------------
  // 6 |   |...|   |...|   |...|   |...|
  //   ---------------------------------
  // 5 |...|   |...|   |...|   |...|   |
  //   ---------------------------------
  // 4 |   |...|   |...|   |...|   |...|
  //   ---------------------------------
  // 3 |...|   |...|   |...|   |...|   |
  //   ---------------------------------
  // 2 | P |.P.| P |.P.| P |.P.| P |.P.|
  //   ---------------------------------
  // 1 |.R.| N |.B.| Q |.K.| B |.N.| R |
  //   ---------------------------------

  for(rank = Rank8; rank >= Rank1; rank--) {
    offset = 2*LINE_LENGTH + 2*LINE_LENGTH*(Rank8-rank);

    // Add line.
    strncpy(ret + offset, lineSide == White ? whiteLine : blackLine, LINE_LENGTH);

    // Add number.
    ret[offset] = '1'+rank;

    // Add pieces.

    // For the sake of debugging, avoid PieceAt() in case it is buggy.

    offset += 4;
    for(file = FileA; file <= FileH; file++) {
      pos = POSITION(rank, file);

      pieceChr = '\0';

      for(side = White; side <= Black; side++) {
        for(piece = Pawn; piece <= King; piece++) {
          if((chessSet->Sets[side].Boards[piece]&POSBOARD(pos)) == POSBOARD(pos)) {
            switch(piece) {
            case Pawn:
              pieceChr = 'P';
              break;
            case Rook:
              pieceChr = 'R';
              break;
            case Knight:
              pieceChr = 'N';
              break;
            case Bishop:
              pieceChr = 'B';
              break;
            case Queen:
              pieceChr = 'Q';
              break;
            case King:
              pieceChr = 'K';
              break;
            default:
              panic("Impossible.");
            }
            goto loop;
          }
        }
      }
    loop:
      if(pieceChr != '\0') {
        if(side == Black) {
          pieceChr += 32;
        }
        ret[offset] = pieceChr;
      }

       offset += 4;
    }

    // Add dotted line.
    strncpy(ret + offset, dottedLine, LINE_LENGTH);

    lineSide = OPPOSITE(lineSide);
  }

  ret[LINE_LENGTH * LINE_COUNT] = '\0';

  return strdup(ret);
}

char*
StringMove(Move move)
{
  char ret[10];

  if(move == INVALID_MOVE || TYPE(move) == InvalidMoveType) {
    return strdup("-");
  }

  sprintf(ret, "%s%s", StringPosition(FROM(move)),
          StringPosition(TO(move)));

  switch(TYPE(move)) {
  case CastleKingSide:
    return strdup("O-O");
  case CastleQueenSide:
    return strdup("O-O-O");

  case PromoteKnight:
    sprintf(ret+4, "=N");
    break;
  case PromoteBishop:
    sprintf(ret+4, "=B");
    break;
  case PromoteRook:
    sprintf(ret+4, "=R");
    break;
  case PromoteQueen:
    sprintf(ret+4, "=Q");
    break;
  case Normal:
    break;
  default:
    sprintf(ret+4, "??%d", TYPE(move));
  }

  return strdup(ret);
}

// String move in long algebraic form.
char*
StringMoveFull(Move move, Piece piece, bool capture)
{
  char actionChr, pieceChr;
  char *suffix, *from, *to;
  char ret[1+2+1+2+2+1];

  if(move == INVALID_MOVE || TYPE(move) == InvalidMoveType) {
    return strdup("-");
  }

  from = StringPosition(FROM(move));
  to = StringPosition(TO(move));

  switch(TYPE(move)) {
  default:
    suffix = "??";
    break;
  case CastleQueenSide:
    return strdup("O-O-O");
  case CastleKingSide:
    return strdup("O-O");
  case EnPassant:
    suffix = "ep";
    break;
  case PromoteKnight:
    suffix = "=N";
    break;
  case PromoteBishop:
    suffix = "=B";
    break;
  case PromoteRook:
    suffix = "=R";
    break;
  case PromoteQueen:
    suffix = "=Q";
    break;
  case Normal:
    suffix = "";
    break;
  }

  if(capture) {
    actionChr = 'x';
  } else {
    actionChr = '-';
  }

  pieceChr = CharPiece(piece);

  if(pieceChr == 'P' || pieceChr == 'p') {
    sprintf(ret, "%s%c%s%s", from, actionChr, to, suffix);
  } else {
    sprintf(ret, "%c%s%c%s%s", pieceChr, from, actionChr, to, suffix);
  }

  return strdup(ret);
}

char*
StringMoveHistory(MemorySlice *history, bool abbrev)
{
  Move move;
  Memory *curr;
  Side side = White;
  StringBuilder builder = NewStringBuilder();
  int fullMoveCount = 1;

  for(curr = history->Vals; curr != history->Curr; curr++) {
    move = curr->Move;

    if(abbrev) {
      AppendString(&builder, "%s ", StringMove(move));      
    } else {
      switch(side) {
      case White:
        AppendString(&builder, "%d. %s",
                     fullMoveCount, StringMove(move));
        break;
      case Black:
        AppendString(&builder, " %s\n",
                     StringMove(move));

        fullMoveCount++;

        break;
      }
    }

    side = OPPOSITE(side);
  }

  return builder.Length == 0 ? NULL : BuildString(&builder, true);
}

char*
StringPerft(PerftStats *s)
{
  char ret[1000];

  sprintf(ret, ""
          "Count:       %llu\n"
          "Captures:    %llu\n"
          "En Passants: %llu\n"
          "Castles:     %llu\n"
          "Promotions:  %llu\n"
          "Checks:      %llu\n"
          "Checkmates:  %llu\n",
          s->Count, s->Captures, s->EnPassants, s->Castles, s->Promotions, s->Checks,
          s->Checkmates);

  return strdup(ret);
}

char*
StringPiece(Piece piece)
{
  char *ret;

  switch(piece) {
  case Pawn:
    ret = "pawn";
    break;
  case Rook:
    ret = "rook";
    break;
  case Knight:
    ret = "knight";
    break;
  case Bishop:
    ret = "bishop";
    break;
  case Queen:
    ret = "queen";
    break;
  case King:
    ret = "king";
    break;
  case MissingPiece:
    ret = "#missing piece";
    break;
  default:
    ret = "#invalid piece";
    break;
  }

  return strdup(ret);
}

char*
StringPosition(Position pos)
{
  char ret[2+1];

  // Unsigned so we don't need to check < 0.
  if(pos > 63) {
    return strdup("#invalid position");
  }

  if(sprintf(ret, "%c%d", 'a'+FILE(pos), RANK(pos)+1) != 2) {
    panic("Couldn't string position");
  }

  ret[2] = '\0';

  return strdup(ret);
}

char*
StringSide(Side side)
{
  char *ret;

  switch(side) {
  case White:
    ret = "white";
    break;
  case Black:
    ret = "black";
    break;
  default:
    ret = "#invalid side";
    break;
  }

  return strdup(ret);
}
