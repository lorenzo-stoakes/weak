#include <stdio.h>
#include "weak.h"
#include "magic.h"

static FORCE_INLINE BitBoard
magicSquareThreats(Position rook, BitBoard occupancy) {
  BitBoard magic = magicBoard[MAGIC_ROOK][rook];
  BitBoard mask = magicMask[MAGIC_ROOK][rook];
  int shift = magicShift[MAGIC_ROOK][rook];
  BitBoard index = (magic*(occupancy&mask))>>shift;

  // TODO: Include non-inner 6 bit positions!

  return RookThreatBase[rook][index];
}

// Get BitBoard encoding capture targets for *all* rooks on specified side.
BitBoard
AllRookCaptureTargets(ChessSet *chessSet, Side side)
{
  switch(side) {
  case White:
    return RookCaptureTargets(chessSet, side, chessSet->White.Rooks);
  case Black:
    return RookCaptureTargets(chessSet, side, chessSet->Black.Rooks);
  }

  panic("Invalid side %d.", side);
  return EmptyBoard;
}

// Get BitBoard encoding move targets for *all* rooks on specified side.
BitBoard
AllRookMoveTargets(ChessSet *chessSet, Side side)
{
  switch(side) {
  case White:
    return RookMoveTargets(chessSet, side, chessSet->White.Rooks);
  case Black:
    return RookMoveTargets(chessSet, side, chessSet->Black.Rooks);
  }

  panic("Invalid side %d.", side);
  return EmptyBoard;
}

// Get BitBoard encoding all squares threatened by rooks.
BitBoard
AllRookThreats(ChessSet *chessSet, Side side)
{
  BitBoard ret, rooks;
  Position rook;

  switch(side) {
  case White:
    rooks = chessSet->White.Rooks;
    break;
  case Black:
    rooks = chessSet->Black.Rooks;
    break;
  default:
    panic("Unrecognised side %d.", side);
  }

  ret = EmptyBoard;

  while(rooks) {
    rook = PopForward(&rooks);
    ret |= magicSquareThreats(rook, chessSet->Occupancy);
  }

  return ret;
}

// Get BitBoard encoding capture targets for specified rooks without using magic BitBoards.
BitBoard
RookCaptureTargets(ChessSet *chessSet, Side side, BitBoard rooks)
{
  BitBoard opposition;
  BitBoard ret = EmptyBoard;
  Position rook;

  switch(side) {
  case White:
    opposition = chessSet->Black.Occupancy;
    break;
  case Black:
    opposition = chessSet->White.Occupancy;
    break;
  default:
    panic("Invalid side %d.", side);
    break;
  }

  ret = EmptyBoard;

  while(rooks) {
    rook = PopForward(&rooks);

    ret |= magicSquareThreats(rook, chessSet->Occupancy);
  }

  return ret & opposition;
}

// Get BitBoard encoding move targets for specified rooks without using magic BitBoards.
BitBoard
RookMoveTargets(ChessSet *chessSet, Side side, BitBoard rooks)
{
  BitBoard ret = EmptyBoard;
  Position rook;

  while(rooks) {
    rook = PopForward(&rooks);  

    ret |= magicSquareThreats(rook, chessSet->Occupancy);
  }

  return ret & chessSet->EmptySquares;
}

// Get rook threats from the specified square and occupancy.
BitBoard
RookSquareThreats(Position rook, BitBoard occupancy)
{
  BitBoard blockers, east, nort, sout, west;
  BitBoard ret = EmptyBoard;
  Position blocker;

  nort = NortRay(rook);
  blockers = nort & occupancy;
  if(blockers != EmptyBoard) {
    blocker = BitScanForward(blockers);
    nort &= ~NortRay(blocker);
  }
  ret |= nort;

  east = EastRay(rook);
  blockers = east & occupancy;
  if(blockers != EmptyBoard) {
    blocker = BitScanForward(blockers);
    east &= ~EastRay(blocker);
  }
  ret |= east;

  sout = SoutRay(rook);
  blockers = sout & occupancy;
  if(blockers != EmptyBoard) {
    blocker = BitScanBackward(blockers);
    sout &= ~SoutRay(blocker);
  }
  ret |= sout;

  west = WestRay(rook);
  blockers = west & occupancy;
  if(blockers != EmptyBoard) {
    blocker = BitScanBackward(blockers);
    west &= ~WestRay(blocker);
  }
  ret |= west;

  return ret;
}
