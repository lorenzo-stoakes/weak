#include "weak.h"

static BitBoard pawnSquares[2][64];

void
InitPawn()
{
  BitBoard bitBoard;
  Position pos;

  for(pos = A1; pos <= H8; pos++) {
    bitBoard = POSBOARD(pos);

    pawnSquares[White][pos] = NoWeOne(bitBoard) | NoEaOne(bitBoard);
    pawnSquares[Black][pos] = SoWeOne(bitBoard) | SoEaOne(bitBoard);
  }
}

BitBoard
PawnAttackersTo(ChessSet *chessSet, Position to)
{
  return (chessSet->Sets[White].Boards[Pawn] & pawnSquares[Black][to]) |
    (chessSet->Sets[Black].Boards[Pawn] & pawnSquares[White][to]);
}

BitBoard
PawnAttacksFrom(Position pawn, Side side)
{
  return pawnSquares[side][pawn];
}
