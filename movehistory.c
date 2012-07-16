#include "weak.h"

MoveHistory
NewMoveHistory()
{
  MoveHistory ret;

  Move *buffer;

  buffer = (Move*)allocate(sizeof(Move), INIT_MOVE_LEN);

  ret.CapturedPieces = NewPieceSlice();
  ret.CastleEvents = NewCastleEventSlice();
  ret.EnPassantSquares = NewEnPassantSlice();
  ret.Moves = NewMoveSlice(buffer, INIT_MOVE_LEN);

  return ret;
}
