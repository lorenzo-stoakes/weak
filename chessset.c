#include "weak.h"

bool
Checked(ChessSet *chessSet, Side side)
{
  BitBoard king = chessSet->Sets[side].Boards[King];

  return (KingThreats(chessSet, OPPOSITE(side))&king) != EmptyBoard;
}

// Get the bitboard which encodes a mask of all squares threatened by the specified side in the
// chess set.
BitBoard
KingThreats(ChessSet *chessSet, Side side)
{
  return PawnKingThreats(chessSet, side) | RookKingThreats(chessSet, side) |
    KnightKingThreats(chessSet, side) | BishopKingThreats(chessSet, side) |
    QueenKingThreats(chessSet, side) | KingKingThreats(chessSet, side);
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

ChessSet
NewEmptyChessSet()
{
  ChessSet ret;

  ret.Sets[White] = NewEmptySet();
  ret.Sets[Black] = NewEmptySet();
  ret.Occupancy = EmptyBoard;
  ret.EmptySquares = FullyOccupied;

  return ret;
}

void
PlacePiece(ChessSet *chessSet, Side side, Piece piece, Position pos)
{
  chessSet->Occupancy |= POSBOARD(pos);
  chessSet->EmptySquares = ~chessSet->Occupancy;

  SetPlacePiece(&chessSet->Sets[side], piece, pos);
}

BitBoard
PinnedPieces(ChessSet *chessSet, Side side)
{
  BitBoard attackers, bitBoard;
  BitBoard ret = EmptyBoard;

  BitBoard kingBoard  = chessSet->Sets[side].Boards[King];
  Position attacker;
  Position king       = BitScanForward(kingBoard);
  Side     opposition = OPPOSITE(side);

  if(kingBoard == EmptyBoard) {
    panic("Empty king board!");
  }

  // If we consider opponents attacks *from* the king's square, this is equivalent to
  // positions in which the pieces in question can attack *to* the king.

  attackers = (chessSet->Sets[opposition].Boards[Rook] |
               chessSet->Sets[opposition].Boards[Queen]) &
    EmptyAttacks[Rook][king];

  attackers |= (chessSet->Sets[opposition].Boards[Bishop] |
               chessSet->Sets[opposition].Boards[Queen]) &
    EmptyAttacks[Bishop][king];

  // If it's just not possible for there to be a pin, we give up here.
  while(attackers != EmptyBoard) {
    attacker = PopForward(&attackers);

    bitBoard = Between[king][attacker] & chessSet->Sets[side].Occupancy;

    if(bitBoard != EmptyBoard && SingleBit(bitBoard)) {
      ret |= bitBoard;
    }
  }

  return ret;
}

void
RemovePiece(ChessSet *chessSet, Side side, Piece piece, Position pos)
{
  BitBoard complement = ~POSBOARD(pos);

  chessSet->Occupancy &= complement;
  chessSet->EmptySquares = ~chessSet->Occupancy;

  SetRemovePiece(&chessSet->Sets[side], piece, pos);
}
