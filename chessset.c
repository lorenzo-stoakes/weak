#include "weak.h"

// Get the bitboard which encodes a mask of all squares threatened by the specified side in the
// chess set.
BitBoard
AllThreats(ChessSet *chessSet, Side side)
{
  return AllPawnThreats(chessSet, side) | AllRookThreats(chessSet, side) |
    AllKnightThreats(chessSet, side) | AllBishopThreats(chessSet, side) |
    AllQueenThreats(chessSet, side) | AllKingThreats(chessSet, side);
}

bool
Checked(ChessSet *chessSet, Side side)
{
  BitBoard king;
  switch(side) {
  case White:
    king = chessSet->White.King;
    break;
  case Black:
    king = chessSet->Black.King;
    break;
  default:
    panic("Invalid side %d.", side);
  }

  return (AllThreats(chessSet, OPPOSITE(side))&king) != EmptyBoard;
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
  chessSet->Occupancy ^= POSBOARD(pos);
  chessSet->EmptySquares = ~chessSet->Occupancy;

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
  chessSet->Occupancy ^= POSBOARD(pos);
  chessSet->EmptySquares = ~chessSet->Occupancy;

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

ChessSet
NewChessSet()
{
  ChessSet ret;

  ret.White = NewWhiteSet();
  ret.Black = NewBlackSet();
  ret.Occupancy = InitOccupancy;
  ret.EmptySquares = ~InitOccupancy;

  return ret;
}
