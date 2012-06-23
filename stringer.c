#include <stdio.h>
#include <strings.h>
#include "weak.h"

char*
StringBitBoard(BitBoard bitBoard)
{
  Rank rank;
  File file;
  Position pos;
  int newline, index;

  char *ret = (char*)allocate(64+8);

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

  return ret;
}

char*
StringPosition(Position pos)
{
  char *ret;

  // Unsigned so we don't need to check < 0.
  if(pos > 63) {
    return strdup("#invalid position");
  }

  ret = (char*)allocate(2);

  if(sprintf(ret, "%c%d", 'a'+FILE(pos), RANK(pos)) != 2) {
    panic("Couldn't string position");
  }

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
