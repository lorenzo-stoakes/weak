#include "weak.h"

BitBoard
AllAttackersTo(ChessSet *chessSet, Position pos, BitBoard occupancy)
{
  return PawnAttackersTo(chessSet, pos) | KnightAttackersTo(chessSet, pos) |
    BishopQueenAttackersTo(chessSet, pos, occupancy) |
    RookQueenAttackersTo(chessSet, pos, occupancy) |
    KingAttackersTo(chessSet, pos);
}

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

ChessSet
NewChessSet()
{
  ChessSet ret;
  Position pos;

  ret.Sets[White] = NewWhiteSet();
  ret.Sets[Black] = NewBlackSet();
  ret.Occupancy = InitOccupancy;
  ret.EmptySquares = ~InitOccupancy;

  for(pos = A3; pos <= H6; pos++) {
    ret.Squares[pos] = MissingPiece;
  }

  for(pos = A2; pos <= H2; pos++) {
    ret.Squares[pos] = Pawn;
  }
  for(pos = A7; pos <= H7; pos++) {
    ret.Squares[pos] = Pawn;
  }

  ret.Squares[A1] = Rook;
  ret.Squares[H1] = Rook;
  ret.Squares[A8] = Rook;
  ret.Squares[H8] = Rook;

  ret.Squares[B1] = Knight;
  ret.Squares[G1] = Knight;
  ret.Squares[B8] = Knight;
  ret.Squares[G8] = Knight;

  ret.Squares[C1] = Bishop;
  ret.Squares[F1] = Bishop;
  ret.Squares[C8] = Bishop;
  ret.Squares[F8] = Bishop;

  ret.Squares[D1] = Queen;
  ret.Squares[D8] = Queen;

  ret.Squares[E1] = King;
  ret.Squares[E8] = King;

  UpdateOccupancies(&ret);

  return ret;
}

ChessSet
NewEmptyChessSet()
{
  ChessSet ret;
  Position pos;

  ret.Sets[White] = NewEmptySet();
  ret.Sets[Black] = NewEmptySet();
  ret.Occupancy = EmptyBoard;
  ret.EmptySquares = FullyOccupied;

  for(pos = A1; pos <= H8; pos++) {
    ret.Squares[pos] = MissingPiece;
  }

  UpdateOccupancies(&ret);

  return ret;
}

Set
NewEmptySet()
{
  Piece piece;
  Set ret;

  for(piece = Pawn; piece <= King; piece++) {
    ret.Boards[piece] = EmptyBoard;
  }

  ret.Occupancy = EmptyBoard;
  ret.EmptySquares = FullyOccupied;

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
PieceAt(Set *set, Position pos)
{
  Piece piece;

  for(piece = Pawn; piece <= King; piece++) {
    if(PositionOccupied(set->Boards[piece], pos)) {
      return piece;
    }
  }

  return MissingPiece;
}

BitBoard
PinnedPieces(ChessSet *chessSet, Side side, Position king, bool pinned)
{
  BitBoard attackers, bitBoard;
  BitBoard ret        = EmptyBoard;
  Side     opposite   = OPPOSITE(side);
  Side     pinner     = pinned ? opposite : side;
  Position attacker;

  // If we consider opponents attacks *from* the king's square, this is equivalent to
  // positions in which the pieces in question can attack *to* the king.

  attackers = (chessSet->Sets[pinner].Boards[Rook] |
               chessSet->Sets[pinner].Boards[Queen]) &
    EmptyAttacks[Rook][king];

  attackers |= (chessSet->Sets[pinner].Boards[Bishop] |
               chessSet->Sets[pinner].Boards[Queen]) &
    EmptyAttacks[Bishop][king];

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
UpdateOccupancies(ChessSet *chessSet)
{
  Piece piece;

  chessSet->Sets[White].Occupancy = EmptyBoard;
  chessSet->Sets[Black].Occupancy = EmptyBoard;
  for(piece = Pawn; piece <= King; piece++) {
    chessSet->Sets[White].Occupancy |= chessSet->Sets[White].Boards[piece];
    chessSet->Sets[Black].Occupancy |= chessSet->Sets[Black].Boards[piece];

    chessSet->PieceOccupancy[piece] = chessSet->Sets[White].Boards[piece] |
      chessSet->Sets[Black].Boards[piece];
  }

  chessSet->Sets[White].EmptySquares = ~chessSet->Sets[White].Occupancy;
  chessSet->Sets[Black].EmptySquares = ~chessSet->Sets[Black].Occupancy;

  chessSet->Occupancy = chessSet->Sets[White].Occupancy | chessSet->Sets[Black].Occupancy;
  chessSet->EmptySquares = ~chessSet->Occupancy;
}
