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
  int i;
  ChessSet ret;
  Piece piece;
  Position pos;
  Side side;

  ret.Sets[White] = NewWhiteSet();
  ret.Sets[Black] = NewBlackSet();
  ret.Occupancy = InitOccupancy;
  ret.EmptySquares = ~InitOccupancy;

  for(pos = A1; pos <= H8; pos++) {
    ret.Squares[pos] = MissingPiece;
    // TODO: This shouldn't matter.
    ret.PiecePositionIndexes[pos] = 0;
  }

  // Clear down piece locations.
  for(side = White; side <= Black; side++) {
    // We ignore king here. King locations stored in CheckStats.
    for(piece = Pawn; piece < King; piece++) {
      for(i = 0; i < MAX_PIECE_LOCATION; i++) {
        ret.PiecePositions[side][piece][i] = EmptyPosition;
      }
    }
  }

  for(side = White; side <= Black; side++) {
    ret.PieceCounts[side][Pawn] = 8;
    ret.PieceCounts[side][Rook] = 2;
    ret.PieceCounts[side][Knight] = 2;
    ret.PieceCounts[side][Bishop] = 2;
    ret.PieceCounts[side][Queen] = 1;
    ret.PieceCounts[side][King] = 1;
  }

  for(pos = A3; pos <= H6; pos++) {
    ret.Squares[pos] = MissingPiece;
    ret.PiecePositionIndexes[pos] = 0;
  }

  for(pos = A2, i=0; pos <= H2; pos++, i++) {
    ret.PiecePositions[White][Pawn][i] = pos;
    ret.PiecePositionIndexes[pos] = i;
    ret.Squares[pos] = Pawn;
  }
  for(pos = A7, i=0; pos <= H7; pos++, i++) {
    ret.PiecePositions[Black][Pawn][i] = pos;
    ret.PiecePositionIndexes[pos] = i;
    ret.Squares[pos] = Pawn;
  }

  ret.Squares[A1] = Rook;
  ret.PiecePositions[White][Rook][0] = A1;
  ret.PiecePositionIndexes[A1] = 0;

  ret.Squares[H1] = Rook;
  ret.PiecePositions[White][Rook][1] = H1;
  ret.PiecePositionIndexes[H1] = 1;

  ret.Squares[A8] = Rook;
  ret.PiecePositions[Black][Rook][0] = A8;
  ret.PiecePositionIndexes[A8] = 0;

  ret.Squares[H8] = Rook;
  ret.PiecePositions[Black][Rook][1] = H8;
  ret.PiecePositionIndexes[H8] = 1;

  ret.Squares[B1] = Knight;
  ret.PiecePositions[White][Knight][0] = B1;
  ret.PiecePositionIndexes[B1] = 0;

  ret.Squares[G1] = Knight;
  ret.PiecePositions[White][Knight][1] = G1;
  ret.PiecePositionIndexes[G1] = 1;

  ret.Squares[B8] = Knight;
  ret.PiecePositions[Black][Knight][0] = B8;
  ret.PiecePositionIndexes[B8] = 0;

  ret.Squares[G8] = Knight;
  ret.PiecePositions[Black][Knight][1] = G8;
  ret.PiecePositionIndexes[G8] = 1;

  ret.Squares[C1] = Bishop;
  ret.PiecePositions[White][Bishop][0] = C1;
  ret.PiecePositionIndexes[C1] = 0;

  ret.Squares[F1] = Bishop;
  ret.PiecePositions[White][Bishop][1] = F1;
  ret.PiecePositionIndexes[F1] = 1;

  ret.Squares[C8] = Bishop;
  ret.PiecePositions[Black][Bishop][0] = C8;
  ret.PiecePositionIndexes[C8] = 0;

  ret.Squares[F8] = Bishop;
  ret.PiecePositions[Black][Bishop][1] = F8;
  ret.PiecePositionIndexes[F8] = 1;

  ret.Squares[D1] = Queen;
  ret.PiecePositions[White][Queen][0] = D1;
  ret.PiecePositionIndexes[D1] = 0;

  ret.Squares[D8] = Queen;
  ret.PiecePositions[Black][Queen][0] = D8;
  ret.PiecePositionIndexes[D8] = 0;

  ret.Squares[E1] = King;
  ret.PiecePositions[White][King][0] = E1;
  ret.PiecePositionIndexes[E1] = 0;

  ret.Squares[E8] = King;
  ret.PiecePositions[White][King][0] = E8;
  ret.PiecePositionIndexes[E8] = 0;  

  UpdateOccupancies(&ret);

  return ret;
}

ChessSet
NewEmptyChessSet()
{
  ChessSet ret;
  int i;
  Piece piece;
  Position pos;
  Side side;

  ret.Sets[White] = NewEmptySet();
  ret.Sets[Black] = NewEmptySet();
  ret.Occupancy = EmptyBoard;
  ret.EmptySquares = FullyOccupied;

  for(pos = A1; pos <= H8; pos++) {
    ret.Squares[pos] = MissingPiece;
    // TODO: This shouldn't matter.
    ret.PiecePositionIndexes[pos] = 0;
  }

  for(side = White; side <= Black; side++) {
    // We ignore king here. King locations stored in CheckStats.
    for(piece = Pawn; piece <= King; piece++) {
      ret.PieceCounts[side][piece] = 0;
      for(i = 0; i < MAX_PIECE_LOCATION; i++) {
        ret.PiecePositions[side][piece][i] = EmptyPosition;
      }
    }
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

BitBoard
PinnedPieces(ChessSet *chessSet, Side side, Position king, bool pinned)
{
  BitBoard attackers, bitBoard;
  BitBoard ret        = EmptyBoard;
  Side     opposite   = OPPOSITE(side);
  Side     pinner     = pinned ? opposite : side;
  Position attacker;

  assert(chessSet->Sets[side].Boards[King] != EmptyBoard);

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
