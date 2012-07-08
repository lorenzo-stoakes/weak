#include "weak.h"

// Get the bitboard which encodes a mask of all squares threatened by the specified side in the
// chess set.
BitBoard
KingThreats(ChessSet *chessSet, Side side)
{
  return PawnKingThreats(chessSet, side) | RookKingThreats(chessSet, side) |
    KnightKingThreats(chessSet, side) | BishopKingThreats(chessSet, side) |
    QueenKingThreats(chessSet, side) | KingKingThreats(chessSet, side);
}

bool
Checked(ChessSet *chessSet, Side side)
{
  BitBoard king = chessSet->Sets[side].Boards[King];

  return (KingThreats(chessSet, OPPOSITE(side))&king) != EmptyBoard;
}

void
ChessSetPlacePiece(ChessSet *chessSet, Side side, Piece piece, Position pos)
{
  chessSet->Occupancy ^= POSBOARD(pos);
  chessSet->EmptySquares = ~chessSet->Occupancy;

  SetPlacePiece(&chessSet->Sets[side], piece, pos);
}

void
ChessSetRemovePiece(ChessSet *chessSet, Side side, Piece piece, Position pos)
{
  chessSet->Occupancy ^= POSBOARD(pos);
  chessSet->EmptySquares = ~chessSet->Occupancy;

  SetRemovePiece(&chessSet->Sets[side], piece, pos);
}

ChessSet
NewChessSet()
{
  ChessSet ret;

  ret.Sets[White] = NewWhiteSet();
  ret.Sets[Black] = NewBlackSet();
  ret.Occupancy = InitOccupancy;
  ret.EmptySquares = ~InitOccupancy;

  return ret;
}
