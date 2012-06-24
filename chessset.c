#include "weak.h"

BitBoard
ChessSetOccupancy(ChessSet *chessSet)
{
  return SetOccupancy(&chessSet->White) + SetOccupancy(&chessSet->Black);
}

Piece
ChessSetPieceAt(ChessSet *chessSet, Side side, Position pos)
{
  switch(side) {
  case White:
    return SetPieceAt(&chessSet->White, pos);
  case Black:
    return SetPieceAt(&chessSet->Black, pos);
  }

  panic("Unrecognised side %d.", side);
  return MissingPiece;
}

BitBoard
EmptySquares(ChessSet *chessSet)
{
  return ~ChessSetOccupancy(chessSet);
}

ChessSet
NewChessSet()
{
  ChessSet ret;

  ret.White = NewWhiteSet();
  ret.Black = NewBlackSet();

  return ret;
}
