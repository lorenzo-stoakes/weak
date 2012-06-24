#include "weak.h"

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
  return AllRookMoveTargets(chessSet, side) | AllRookCaptureTargets(chessSet, side);
}

// Get BitBoard encoding capture targets for specified rooks.
BitBoard
RookCaptureTargets(ChessSet *chessSet, Side side, BitBoard rooks)
{
  int i;
  BitBoard blockers, east, nort, sout, west, occupancy, opposition, ret;
  Positions positions;
  Position blocker, rook;

  occupancy = ChessSetOccupancy(chessSet);

  switch(side) {
  case White:
    opposition = SetOccupancy(&chessSet->Black);
    break;
  case Black:
    opposition = SetOccupancy(&chessSet->White);
    break;
  default:
    panic("Invalid side %d.", side);
    break;
  }

  ret = EmptyBoard;
  positions = BoardPositions(rooks);
  for(i = 0; i < positions.Len; i++) {
    rook = positions.Vals[i];
    nort = NortRay(rook);
    blockers = nort & occupancy;
    if(blockers != EmptyBoard) {
      blocker = BitScanForward(blockers);
      ret |= POSBOARD(blocker);
    }

    east = EastRay(rook);
    blockers = east & occupancy;
    if(blockers != EmptyBoard) {
      blocker = BitScanForward(blockers);
      ret |= POSBOARD(blocker);
    }

    sout = SoutRay(rook);
    blockers = sout & occupancy;
    if(blockers != EmptyBoard) {
      blocker = BitScanBackward(blockers);
      ret |= POSBOARD(blocker);
    }

    west = WestRay(rook);
    blockers = west & occupancy;
    if(blockers != EmptyBoard) {
      blocker = BitScanBackward(blockers);
      ret |= POSBOARD(blocker);
    }
  }

  return ret & opposition;
}

// Get BitBoard encoding move targets for specified rooks.
BitBoard
RookMoveTargets(ChessSet *chessSet, Side side, BitBoard rooks)
{
  int i;
  BitBoard blockers, east, nort, sout, west, occupancy, ret;
  Positions positions;
  Position blocker, rook;

  occupancy = ChessSetOccupancy(chessSet);

  ret = EmptyBoard;
  positions = BoardPositions(rooks);
  for(i = 0; i < positions.Len; i++) {
    rook = positions.Vals[i];
    nort = NortRay(rook);
    blockers = nort & occupancy;
    if(blockers != EmptyBoard) {
      blocker = BitScanForward(blockers);
      nort ^= blockers | NortRay(blocker);
    }
    ret |= nort;

    east = EastRay(rook);
    blockers = east & occupancy;
    if(blockers != EmptyBoard) {
      blocker = BitScanForward(blockers);
      east ^= blockers | EastRay(blocker);
    }
    ret |= east;

    sout = SoutRay(rook);
    blockers = sout & occupancy;
    if(blockers != EmptyBoard) {
      blocker = BitScanBackward(blockers);
      sout ^= blockers | SoutRay(blocker);
    }
    ret |= sout;

    west = WestRay(rook);
    blockers = west & occupancy;
    if(blockers != EmptyBoard) {
      blocker = BitScanBackward(blockers);
      west ^= blockers | WestRay(blocker);
    }
    ret |= west;
  }

  return ret;
}

