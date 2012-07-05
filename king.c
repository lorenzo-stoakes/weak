#include "weak.h"

static BitBoard kingsSquares(ChessSet*, Side);

// We don't need to provide king sources, or to consider subsets of kings on the board as there
// is always one king on each board for each side, and it is never removed from the board.

// Get BitBoard encoding all squares threatened by king. Doesn't take into account threats of
// moves which would put the king into check.
BitBoard
AllKingThreats(ChessSet *chessSet, Side side)
{
  // King capture and movement threats happen to be the same.  
  return kingsSquares(chessSet, side);
}

// Target squares for king movement. Doesn't take into account moves which put the king in
// check.
BitBoard
KingCaptureTargets(ChessSet *chessSet, Side side, BitBoard _)
{
  BitBoard ret;

  ret = kingsSquares(chessSet, side);
  switch(side) {
  case White:
    return ret & chessSet->Black.Occupancy;
  case Black:
    return ret & chessSet->White.Occupancy;
  }

  panic("Invalid side %d.", side);
  return EmptyBoard;
}

// Target squares for king movement. Doesn't take into account moves which put the king in
// check.
BitBoard
KingMoveTargets(ChessSet *chessSet, Side side, BitBoard _)
{
  // Can't move into other pieces.
  return kingsSquares(chessSet, side) & chessSet->EmptySquares;
}

// Obtain bitboard encoding all squares where the king could move to or capture a piece in
// theory, should those squares be valid for movement/capture.
static BitBoard
kingsSquares(ChessSet *chessSet, Side side)
{
  BitBoard kingSet, ret;

  switch(side) {
  case White:
    kingSet = chessSet->White.King;
    break;
  case Black:
    kingSet = chessSet->Black.King;
    break;
  default:
    panic("Invalid side %d.", side);
  }

  // TODO: Put these values into a lookup table.
  ret = EastOne(kingSet) | WestOne(kingSet);
  kingSet |= ret;
  ret |= NortOne(kingSet) | SoutOne(kingSet);

  return ret;
}
