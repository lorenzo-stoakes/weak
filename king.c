#include "weak.h"

static BitBoard kingSquares[64];

// We don't need to provide king sources, or to consider subsets of kings on the board as there
// is always one king on each board for each side, and it is never removed from the board.

void
InitKing()
{
  BitBoard bitBoard, king;
  Position pos;

  for(pos = A1; pos <= H8; pos++) {
    king = POSBOARD(pos);

    bitBoard = EastOne(king) | WestOne(king);
    king |= bitBoard;
    bitBoard |= NortOne(king) | SoutOne(king);

    kingSquares[pos] = bitBoard;
  }
}

BitBoard
KingAttackersTo(ChessSet *chessSet, Position to)
{
  return (chessSet->Sets[White].Boards[King] |
          chessSet->Sets[Black].Boards[King]) &
    kingSquares[to];
}

BitBoard
KingAttacksFrom(Position king)
{
  return kingSquares[king];
}
