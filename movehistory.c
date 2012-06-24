#include "weak.h"


MoveHistory
NewMoveHistory()
{
  MoveHistory ret;

  ret.CastleEvents = NewCastleEventSlice();
  ret.Moves = NewMoveSlice();
  ret.CapturedPieces = NewPieceSlice();

  return ret;
}
