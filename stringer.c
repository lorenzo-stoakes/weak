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
  Rank rank;
  File file;
  Position pos;
  int newline, index;

  char ret[64+8+1];

  for(pos = A1; pos <= H8; pos++) {
    rank = Rank8 - RANK(pos);
    file = FILE(pos);
    newline = rank;
    index = 8*rank + file + newline;

    if(file == 7) {
        ret[index+1] = '\n';
    }

    if((POSBOARD(pos)&bitBoard) == 0) {
      ret[index] = '.';
    } else {
      ret[index] = '1';
    }
  }
  ret[64+8] = '\0';

  return strdup(ret);
}

// ASCII-art representation of chessboard.
char*
StringChessSet(ChessSet *chessSet)
{
  // Include space for newlines.
  char ret[64+8+1];
  char pieceChr;
  int index, newline;
  File file;
  Piece piece;
  Position pos;
  Rank rank;
  Side side;

  // Outputs chess set as ascii-art.
  // pieces are upper-case if white, lower-case if black.
  // P=pawn, R=rook, N=knight, B=bishop, Q=queen, K=king, .=empty square.

  // e.g., the initial position is output as follows:-

  // rnbqkbnr
  // pppppppp
  // ........
  // ........
  // ........
  // ........
  // PPPPPPPP
  // RNBQKBNR

  for(pos = A1; pos <= H8; pos++) {
    // Vertical flip.
    rank = Rank8 - RANK(pos);
    file = FILE(pos);

	// We need to leave space for a newline after each rank. This is equal to the
	// number of ranks which will appear before this one in the ASCII board,
	// e.g. the vertically flipped rank.
    newline = rank;
    index = 8*rank + file + newline;

    if(file == 7) {
      ret[index+1] = '\n';
    }

    piece = SetPieceAt(&chessSet->White, pos);

    if(piece != MissingPiece) {
      side = White;
    } else {
      side = Black;
      piece = SetPieceAt(&chessSet->Black, pos);
    }

    if(piece == MissingPiece) {
      ret[index] = '.';
    } else {
      pieceChr = CharPiece(piece);

      switch(side) {
      case White:
        ret[index] = pieceChr;
        break;
      case Black:
        ret[index] = tolower(pieceChr);
        break;
      default:
        panic("Impossible.");
      }
    }
  }

  ret[64+8] = '\0';

  for(pos = A1; pos <= H8; pos++) {
    if(ret[pos] == '\0') {
        ret[pos] = '?';
    }
  }

  return strdup(ret);
}

// String move in long algebraic form.
char*
StringMove(Move *move)
{
  char actionChr, pieceChr;
  char *suffix, *from, *to;
  char ret[1+2+1+2+2+1];

  from = StringPosition(move->From);
  to = StringPosition(move->To);

  switch(move->Type) {
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
  
  if(move->Capture) {
    actionChr = 'x';
  } else {
    actionChr = '-';
  }

  pieceChr = CharPiece(move->Piece);

  if(pieceChr == 'P' || pieceChr == 'p') {
    sprintf(ret, "%s%c%s%s", from, actionChr, to, suffix);
  } else {
    sprintf(ret, "%c%s%c%s%s", pieceChr, from, actionChr, to, suffix);
  }

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

  if(sprintf(ret, "%c%d", 'a'+FILE(pos), RANK(pos)) != 2) {
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
