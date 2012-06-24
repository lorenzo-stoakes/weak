#include <string.h>
#include "weak.h"

static const int INIT_MOVE_COUNT = 100;

// Append Move to specified Move slice.
MoveSlice
AppendMove(MoveSlice slice, Move move)
{
  // TODO: Avoid duplication :-)  

  MoveSlice ret;

  if(slice.Len > slice.Cap) {
    panic("MoveSlice.Len %d > MoveSlice.Cap %d - Impossible!", slice.Len, slice.Cap);
  }

  // Expand.
  if(slice.Len == slice.Cap) {
    ret.Cap = slice.Cap*2;
    ret.Vals = (Move*)allocate(sizeof(Move)*ret.Cap);
    memcpy(ret.Vals, slice.Vals, slice.Len);
    release(slice.Vals);
  } else {
    ret.Vals = slice.Vals;
    ret.Cap = slice.Cap;
  }

  ret.Vals[slice.Len] = move;
  ret.Len = slice.Len+1;

  return ret;
}

MoveSlice
NewMoveSlice()
{
  MoveSlice ret;

  ret.Cap = INIT_MOVE_COUNT;
  ret.Len = 0;
  ret.Vals = (Move*)allocate(sizeof(Move)*INIT_MOVE_COUNT);

  return ret;
}

Move
PopMove(MoveSlice *slice)
{
  Move ret;

  if(slice->Len <= 0) {
    panic("Invaild slice length %d on PopMove().", slice->Len);
  }

  ret = slice->Vals[slice->Len-1];
  slice->Len--;

  return ret;
}
