#include "weak.h"

BitBoard
AllAttackersTo(ChessSet *chessSet, Position pos, BitBoard occupancy)
{
  return PawnAttackersTo(chessSet, pos) | KnightAttackersTo(chessSet, pos) |
    BishopQueenAttackersTo(chessSet, pos, occupancy) |
    RookQueenAttackersTo(chessSet, pos, occupancy) |
    KingAttackersTo(chessSet, pos);
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
PinnedPieces(ChessSet *chessSet, Side side, bool pinned)
{
  BitBoard attackers, bitBoard;
  BitBoard ret        = EmptyBoard;
  Side     opposite   = OPPOSITE(side);
  Side     pinner     = pinned ? opposite : side;
  BitBoard kingBoard  = chessSet->Sets[pinned ? side : opposite].Boards[King];
  Position attacker;
  Position king       = BitScanForward(kingBoard);

  if(kingBoard == EmptyBoard) {
    panic("Empty king board!");
  }

  // If we consider opponents attacks *from* the king's square, this is equivalent to
  // positions in which the pieces in question can attack *to* the king.

  attackers = (chessSet->Sets[pinner].Boards[Rook] |
               chessSet->Sets[pinner].Boards[Queen]) &
    EmptyAttacks[Rook][king];

  attackers |= (chessSet->Sets[pinner].Boards[Bishop] |
               chessSet->Sets[pinner].Boards[Queen]) &
    EmptyAttacks[Bishop][king];

  // If it's just not possible for there to be a pin, we give up here.
  while(attackers != EmptyBoard) {
    attacker = PopForward(&attackers);

    bitBoard = Between[king][attacker] & chessSet->Occupancy;

    if(bitBoard != EmptyBoard && SingleBit(bitBoard) &&
       (bitBoard & chessSet->Sets[side].Occupancy) != EmptyBoard) {
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
