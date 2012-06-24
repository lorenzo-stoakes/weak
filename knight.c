#include "weak.h"

static BitBoard knightSquares[64];

void
InitKnight()
{
  BitBoard attacks, bitBoard, east, west;
  Position pos;

  for(pos = A1; pos <= H8; pos++) {
    bitBoard = POSBOARD(pos);

    // ..x.x..
    // .x...x.
    // ...N...
    // .x...x.
    // ..x.x..

    east = EastOne(bitBoard);
    west = WestOne(bitBoard);

    // ..x.x..
    // .x...x.
    // ..*N*..
    // .x...x.
    // ..x.x..

    attacks = (east | west) << 16;
    attacks |= (east | west) >> 16;

    // ..*.*..
    // .x...x.
    // ...N...
    // .x...x.
    // ..*.*..

    east = EastOne(east);
    west = WestOne(west);

    // ..*.*..
    // .x...x.
    // .*.N.*.
    // .x...x.
    // ..*.*..

    attacks |= (east | west) << 8;
    attacks |= (east | west) >> 8;

    // ..*.*..
    // .*...*.
    // ...N...
    // .*...*.
    // ..*.*..

    knightSquares[pos] = attacks;
  }
}

// Get BitBoard encoding capture targets for *all* knights on specified side.
BitBoard
AllKnightCaptureTargets(ChessSet *chessSet, Side side)
{
  switch(side) {
  case White:
    return KnightCaptureTargets(chessSet, side, chessSet->White.Knights);
  case Black:
    return KnightCaptureTargets(chessSet, side, chessSet->Black.Knights);
  }

  panic("Invalid side %d.", side);
  return EmptyBoard;
}

// Get BitBoard encoding move targets for *all* knights on specified side.
BitBoard
AllKnightMoveTargets(ChessSet *chessSet, Side side)
{
  switch(side) {
  case White:
    return KnightMoveTargets(chessSet, side, chessSet->White.Knights);
  case Black:
    return KnightMoveTargets(chessSet, side, chessSet->Black.Knights);
  }

  panic("Invalid side %d.", side);
  return EmptyBoard;
}

// Get BitBoard encoding all squares threatened by knights.
BitBoard
AllKnightThreats(ChessSet *chessSet, Side side)
{
  return AllKnightMoveTargets(chessSet, side) | AllKnightCaptureTargets(chessSet, side);
}

BitBoard
KnightCaptureTargets(ChessSet *chessSet, Side side, BitBoard knights)
{
  int i;
  Position knight;
  Positions positions;
  BitBoard opposition, ret;

  switch(side) {
  case White:
    opposition = SetOccupancy(&chessSet->Black);
    break;
  case Black:
    opposition = SetOccupancy(&chessSet->White);
    break;
  default:
    panic("Invalid side %d.", side);
  }

  ret = EmptyBoard;
  positions = BoardPositions(knights);
  for(i = 0; i < positions.Len; i++) {
    knight = positions.Vals[i];
    ret |= knightSquares[knight] & opposition;
  }

  return ret;
}

BitBoard
KnightMoveTargets(ChessSet *chessSet, Side side, BitBoard knights)
{
  int i;
  BitBoard ret;
  Position knight;
  Positions positions;

  ret = EmptyBoard;
  positions = BoardPositions(knights);
  for(i = 0; i < positions.Len; i++) {
    knight = positions.Vals[i];
    ret |= knightSquares[knight];
  }

  return ret & EmptySquares(chessSet);
}
