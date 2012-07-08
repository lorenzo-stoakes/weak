#include <string.h>
#include "weak.h"

static int expand(MoveSlice*);

// Append Move to specified Move slice.
void
AppendMove(MoveSlice *slice, Move move)
{
  // TODO: Avoid duplication :-)

  if(slice->Len > slice->Cap) {
    panic("MoveSlice->Len %d > MoveSlice->Cap %d - Impossible!", slice->Len, slice->Cap);
  }

  if(slice->Len == slice->Cap) {
    expand(slice);
  }

  slice->Vals[slice->Len] = move;
  slice->Len++;
}

void
AppendMoves(MoveSlice *dst, MoveSlice *src)
{
  while(dst->Cap < dst->Len + src->Len) {
    expand(dst);
  }

  memcpy(dst->Vals + dst->Len, src->Vals, src->Len);
}

MoveSlice
NewMoveSlice(Move *buffer, int cap)
{
  MoveSlice ret;

  ret.Cap = cap;
  ret.Len = 0;
  ret.Vals = buffer;

  return ret;
}

Move
PopMove(MoveSlice *slice)
{
  Move ret;

  if(slice->Len <= 0) {
    panic("Invalid slice length %d on PopMove().", slice->Len);
  }

  ret = slice->Vals[slice->Len-1];
  slice->Len--;

  return ret;
}

static int
expand(MoveSlice *slice)
{
  Move *buffer;  

  slice->Cap *= 2;
  buffer = (Move*)allocate(sizeof(Move), slice->Cap);
  memcpy(buffer, slice->Vals, slice->Len);
  release(slice->Vals);
  slice->Vals = buffer;

  return slice->Cap;
}
