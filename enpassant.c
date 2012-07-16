#include <strings.h>
#include "weak.h"

static const int INIT_ENPASSANT_COUNT = 100;

static void expand(EnPassantSlice*);

void
AppendEnpassantSquare(EnPassantSlice *slice, Position pos)
{
  // TODO: Avoid duplication :-)

  if(slice->Len > slice->Cap) {
    panic("EnPassantSlice->Len %d > EnPassantSlice->Cap %d - Impossible!", slice->Len,
          slice->Cap);
  }

  if(slice->Len == slice->Cap) {
    expand(slice);
  }

  slice->Vals[slice->Len] = pos;
  slice->Len++;
}

EnPassantSlice
NewEnPassantSlice()
{
  EnPassantSlice ret;

  ret.Len = 0;
  ret.Cap = INIT_ENPASSANT_COUNT;
  ret.Vals = (Position*)allocate(sizeof(Position), INIT_ENPASSANT_COUNT);

  return ret;
}

Position
PopEnPassantSquare(EnPassantSlice *slice)
{
  Position ret;

  if(slice->Len <= 0) {
    panic("Invaild slice length %d on PopEnPassantSquare().", slice->Len);
  }

  ret = slice->Vals[slice->Len-1];
  slice->Len--;

  return ret;
}

static void
expand(EnPassantSlice *slice)
{
  Position *buffer;

  slice->Cap *= 2;
  buffer = (Position*)allocate(sizeof(Position), slice->Cap);
  memcpy(buffer, slice->Vals, slice->Len*sizeof(Position*));
  release(slice->Vals);
  slice->Vals = buffer;
}
