#include "weak.h"

BitBoard
SetOccupancy(Set set)
{
  return set.Pawns | set.Rooks | set.Knights | set.Bishops | set.Queens | set.King;
}

Set
NewBlackSet()
{
  Set ret;

  ret.Pawns = InitBlackPawns;
  ret.Rooks = InitBlackRooks;
  ret.Knights = InitBlackKnights;
  ret.Bishops = InitBlackBishops;
  ret.Queens = InitBlackQueens;
  ret.King = InitBlackKing;

  return ret;
}

Set
NewWhiteSet()
{
  Set ret;

  ret.Pawns = InitWhitePawns;
  ret.Rooks = InitWhiteRooks;
  ret.Knights = InitWhiteKnights;
  ret.Bishops = InitWhiteBishops;
  ret.Queens = InitWhiteQueens;
  ret.King = InitWhiteKing;

  return ret;
}

Piece
SetPieceAt(Set *set, Position pos)
{
  if(PositionOccupied(set->Pawns, pos)) {
    return Pawn;
  }
  if(PositionOccupied(set->Rooks, pos)) {
    return Rook;
  }
  if(PositionOccupied(set->Knights, pos)) {
    return Knight;
  }
  if(PositionOccupied(set->Bishops, pos)) {
    return Bishop;
  }
  if(PositionOccupied(set->Queens, pos)) {
    return Queen;
  }
  if(PositionOccupied(set->King, pos)) {
    return King;
  }

  return MissingPiece;
}
