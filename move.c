#include <string.h>
#include "weak.h"

// Append Move to specified Move slice.
void
AppendMove(MoveSlice *slice, Move move)
{
  // TODO: Avoid duplication :-)

  Move *buffer;

  if(slice->Len > slice->Cap) {
    panic("MoveSlice->Len %d > MoveSlice->Cap %d - Impossible!", slice->Len, slice->Cap);
  }

  // Expand.
  if(slice->Len == slice->Cap) {
    slice->Cap *= 2;
    buffer = (Move*)allocate(sizeof(Move)*slice->Cap);
    memcpy(buffer, slice->Vals, slice->Len);
    release(slice->Vals);
    slice->Vals = buffer;
  }

  slice->Vals[slice->Len] = move;
  slice->Len++;
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
