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
  return KnightCaptureTargets(chessSet, side, chessSet->Sets[side].Boards[Knight]);
}

// Get BitBoard encoding move targets for *all* knights on specified side.
BitBoard
AllKnightMoveTargets(ChessSet *chessSet, Side side)
{
  return KnightMoveTargets(chessSet, side, chessSet->Sets[side].Boards[Knight]);
}

// Get BitBoard encoding all squares threatened by knights.
BitBoard
AllKnightThreats(ChessSet *chessSet, Side side)
{
  BitBoard knights = chessSet->Sets[side].Boards[Knight];
  BitBoard ret = EmptyBoard;
  Position knight;

  while(knights) {
    knight = PopForward(&knights);

    ret |= knightSquares[knight];
  }

  return ret;
}

BitBoard
KnightCaptureTargets(ChessSet *chessSet, Side side, BitBoard knights)
{
  Position knight;
  BitBoard ret;
  BitBoard opposition = chessSet->Sets[OPPOSITE(side)].Occupancy;

  ret = EmptyBoard;

  while(knights) {
    knight = PopForward(&knights);

    ret |= knightSquares[knight] & opposition;
  }

  return ret;
}

BitBoard
KnightMoveTargets(ChessSet *chessSet, Side side, BitBoard knights)
{
  BitBoard ret;
  Position knight;

  ret = EmptyBoard;

  while(knights) {
    knight = PopForward(&knights);

    ret |= knightSquares[knight];
  }

  return ret & chessSet->EmptySquares;
}
