#include "weak.h"

static const int INIT_CASTLE_EVENT_COUNT = 4;

CastleEventSlice
NewCastleEventSlice()
{
  CastleEventSlice ret;

  ret.Len = 0;
  ret.Cap = INIT_CASTLE_EVENT_COUNT;
  ret.Vals = (CastleEvent*)allocate(sizeof(CastleEvent)*INIT_CASTLE_EVENT_COUNT);

  return ret;
}

MoveHistory
NewMoveHistory()
{
  MoveHistory ret;

  ret.CastleEvents = NewCastleEventSlice();
  ret.Moves = NewMoveSlice();
  ret.CapturedPieces = NewPieceSlice();

  return ret;
}
