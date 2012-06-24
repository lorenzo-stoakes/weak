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

void
ChessSetPlacePiece(ChessSet *chessSet, Side side, Piece piece, Position pos)
{
  switch(side) {
  case White:
    SetPlacePiece(&chessSet->White, piece, pos);
    break;
  case Black:
    SetPlacePiece(&chessSet->Black, piece, pos);
    break;
  default:
    panic("Invalid side %d.", side);
  }
}

void
ChessSetRemovePiece(ChessSet *chessSet, Side side, Piece piece, Position pos)
{
  switch(side) {
  case White:
    SetRemovePiece(&chessSet->White, piece, pos);
    break;
  case Black:
    SetRemovePiece(&chessSet->Black, piece, pos);
    break;
  default:
    panic("Invalid side %d.", side);
  }  
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
