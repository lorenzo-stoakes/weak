#include <stdio.h>
#include <strings.h>
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

  char *ret = (char*)allocate(64+8+1);

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

  return ret;
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
  char *ret;

  // Unsigned so we don't need to check < 0.
  if(pos > 63) {
    return strdup("#invalid position");
  }

  ret = (char*)allocate(2+1);

  if(sprintf(ret, "%c%d", 'a'+FILE(pos), RANK(pos)) != 2) {
    panic("Couldn't string position");
  }

  ret[2] = '\0';

  return ret;
}

char*
StringSide(Side side)
{
  switch(side) {
  case White:
    return strdup("white");
  case Black:
    return strdup("black");
  }

  return strdup("#invalid side");
}
