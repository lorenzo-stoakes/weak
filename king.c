#include "weak.h"

static BitBoard kingsSquares(BitBoard);

// We don't need to provide king sources, or to consider subsets of kings on the board as there
// is always one king on each board for each side, and it is never removed from the board.

BitBoard
KingAttackersTo(ChessSet *chessSet, Position to)
{
  return (chessSet->Sets[White].Boards[King] |
          chessSet->Sets[Black].Boards[King]) &
    kingsSquares(POSBOARD(to));
}

BitBoard
KingAttacksFrom(Position king)
{
  return kingsSquares(POSBOARD(king));
}

// Obtain bitboard encoding all squares where the king could move to or capture a piece in
// theory, should those squares be valid for movement/capture.
static BitBoard
  kingsSquares(BitBoard king)
{
  BitBoard ret;

  // TODO: Put these values into a lookup table.
  ret = EastOne(king) | WestOne(king);
  king |= ret;
  ret |= NortOne(king) | SoutOne(king);

  return ret;
}
