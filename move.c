#include "weak.h"

static const int INIT_MOVE_COUNT = 100;

MoveSlice
NewMoveSlice()
{
  MoveSlice ret;

  ret.Cap = INIT_MOVE_COUNT;
  ret.Len = 0;
  ret.Vals = (Move*)allocate(sizeof(Move)*INIT_MOVE_COUNT);

  return ret;
}
