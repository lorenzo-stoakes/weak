#include "weak.h"

static const int INIT_PIECE_COUNT = 32;

PieceSlice
NewPieceSlice()
{
  PieceSlice ret;

  ret.Cap = INIT_PIECE_COUNT;
  ret.Len = 0;
  ret.Vals = (Piece*)allocate(sizeof(Piece)*INIT_PIECE_COUNT);

  return ret;
}
