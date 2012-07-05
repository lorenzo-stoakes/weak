#include "weak.h"

Set
NewBlackSet()
{
  Set ret;

  ret.Occupancy = InitBlackOccupancy;
  ret.EmptySquares = ~InitBlackOccupancy;

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

  ret.Occupancy = InitWhiteOccupancy;
  ret.EmptySquares = ~InitWhiteOccupancy;

  ret.Pawns = InitWhitePawns;
  ret.Rooks = InitWhiteRooks;
  ret.Knights = InitWhiteKnights;
  ret.Bishops = InitWhiteBishops;
  ret.Queens = InitWhiteQueens;
  ret.King = InitWhiteKing;

  return ret;
}

// Determine whether the set has a piece at the specified position, and if so what that piece
// is.
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

// Place piece on set.
void
SetPlacePiece(Set *set, Piece piece, Position pos)
{
  BitBoard bitBoard;

  if(pos > 63) {
    panic("Invaild position %d.", pos);
  }

  bitBoard = POSBOARD(pos);

  set->Occupancy ^= bitBoard;
  set->EmptySquares = ~set->Occupancy;

  switch(piece) {
  case Pawn:
    set->Pawns |= bitBoard;
    break;
  case Rook:
    set->Rooks |= bitBoard;
    break;    
  case Knight:
    set->Knights |= bitBoard;
    break;    
  case Bishop:
    set->Bishops |= bitBoard;
    break;    
  case Queen:
    set->Queens |= bitBoard;
    break;    
  case King:
    set->King |= bitBoard;
    break;    
  default:
    panic("Invalid piece %d.", piece);
  }
}

// Remove piece on set.
void
SetRemovePiece(Set *set, Piece piece, Position pos)
{
  BitBoard complement;

  if(pos > 63) {
    panic("Invaild position %d.", pos);
  }

  complement = ~POSBOARD(pos);

  set->Occupancy &= complement;
  set->EmptySquares = ~set->Occupancy;

  switch(piece) {
  case Pawn:
    set->Pawns &= complement;
    break;
  case Rook:
    set->Rooks &= complement;
    break;
  case Knight:
    set->Knights &= complement;
    break;
  case Bishop:
    set->Bishops &= complement;
    break;
  case Queen:
    set->Queens &= complement;
    break;
  case King:
    set->King &= complement;
    break;
  default:
    panic("Invalid piece %d.", piece);
  }
}
