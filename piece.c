#include "weak.h"

static const int INIT_PIECE_COUNT = 32;

// Append Piece to specified Piece slice.
PieceSlice
AppendPiece(PieceSlice slice, Piece piece)
{
  PieceSlice ret;

  // Never need to expand slice, as there can never be more than 32 pieces on the board.
  if(slice.Len >= slice.Cap) {
    panic("Impossible condition, have %d pieces.", slice.Len);
  }

  ret.Vals = slice.Vals;    
  ret.Vals[slice.Len] = piece;
  ret.Len = slice.Len+1;
  ret.Cap = slice.Cap;  

  return ret;
}

Piece
PopPiece(PieceSlice *slice)
{
  Piece ret;

  if(slice->Len <= 0) {
    panic("Invaild slice length %d on PopPiece().", slice->Len);    
  }

  ret = slice->Vals[slice->Len-1];
  slice->Len--;

  return ret;
}

PieceSlice
NewPieceSlice()
{
  PieceSlice ret;

  ret.Cap = INIT_PIECE_COUNT;
  ret.Len = 0;
  ret.Vals = (Piece*)allocate(sizeof(Piece)*INIT_PIECE_COUNT);

  return ret;
}
