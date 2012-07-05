#include "weak.h"

MoveHistory
NewMoveHistory()
{
  MoveHistory ret;

  Move *buffer;

  buffer = (Move*)allocate(sizeof(Move)*INIT_MOVE_LEN);

  ret.CastleEvents = NewCastleEventSlice();
  ret.Moves = NewMoveSlice(buffer, INIT_MOVE_LEN);
  ret.CapturedPieces = NewPieceSlice();

  return ret;
}
