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
  BitBoard king = chessSet->Sets[side].Boards[King];

  return (AllThreats(chessSet, OPPOSITE(side))&king) != EmptyBoard;
}

Piece
ChessSetPieceAt(ChessSet *chessSet, Side side, Position pos)
{
  return SetPieceAt(&chessSet->Sets[side], pos);
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
