#include "weak.h"

Set
NewBlackSet()
{
  Set ret;

  ret.Occupancy = InitBlackOccupancy;
  ret.EmptySquares = ~InitBlackOccupancy;

  ret.Boards[Pawn] = InitBlackPawns;
  ret.Boards[Rook] = InitBlackRooks;
  ret.Boards[Knight] = InitBlackKnights;
  ret.Boards[Bishop] = InitBlackBishops;
  ret.Boards[Queen] = InitBlackQueens;
  ret.Boards[King] = InitBlackKing;

  return ret;
}

Set
NewWhiteSet()
{
  Set ret;

  ret.Occupancy = InitWhiteOccupancy;
  ret.EmptySquares = ~InitWhiteOccupancy;

  ret.Boards[Pawn] = InitWhitePawns;
  ret.Boards[Rook] = InitWhiteRooks;
  ret.Boards[Knight] = InitWhiteKnights;
  ret.Boards[Bishop] = InitWhiteBishops;
  ret.Boards[Queen] = InitWhiteQueens;
  ret.Boards[King] = InitWhiteKing;

  return ret;
}

// Determine whether the set has a piece at the specified position, and if so what that piece
// is.
Piece
SetPieceAt(Set *set, Position pos)
{
  Piece piece;

  for(piece = Pawn; piece <= King; piece++) {
    if(PositionOccupied(set->Boards[piece], pos)) {
      return piece;
    }
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

  set->Boards[piece] |= bitBoard;
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

  set->Boards[piece] &= complement;
}
