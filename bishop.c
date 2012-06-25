#include "weak.h"

BitBoard
AllBishopCaptureTargets(ChessSet *chessSet, Side side)
{
  switch(side) {
  case White:
    return BishopCaptureTargets(chessSet, side, chessSet->White.Bishops);
  case Black:
    return BishopCaptureTargets(chessSet, side, chessSet->Black.Bishops);
  }

  panic("Invalid side %d.", side);
  return EmptyBoard;
}

BitBoard
AllBishopMoveTargets(ChessSet *chessSet, Side side)
{
  switch(side) {
  case White:
    return BishopMoveTargets(chessSet, side, chessSet->White.Bishops);
  case Black:
    return BishopMoveTargets(chessSet, side, chessSet->Black.Bishops);
  }

  panic("Invalid side %d.", side);
  return EmptyBoard;
}

BitBoard
AllBishopThreats(ChessSet *chessSet, Side side)
{
  return AllBishopMoveTargets(chessSet, side) | AllBishopCaptureTargets(chessSet, side);
}

BitBoard
BishopCaptureTargets(ChessSet *chessSet, Side side, BitBoard bishops)
{
  BitBoard blockers, occupancy, opposition, noea, nowe, soea, sowe, ret;
  Position bishop, blocker;

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
  }

  ret = EmptyBoard;
  for(bishop = BitScanForward(bishops); bishops; bishop = BitScanForward(bishops)) {
    bishops ^= POSBOARD(bishop);

    noea = NoEaRay(bishop);
    blockers = noea & occupancy;
    if(blockers != EmptyBoard) {
      blocker = BitScanForward(blockers);
      ret |= POSBOARD(blocker);
    }

    nowe = NoWeRay(bishop);
    blockers = nowe & occupancy;
    if(blockers != EmptyBoard) {
      blocker = BitScanForward(blockers);
      ret |= POSBOARD(blocker);
    }

    soea = SoEaRay(bishop);
    blockers = soea & occupancy;
    if(blockers != EmptyBoard) {
      blocker = BitScanBackward(blockers);
      ret |= POSBOARD(blocker);
    }

    sowe = SoWeRay(bishop);
    blockers = sowe & occupancy;
    if(blockers != EmptyBoard) {
      blocker = BitScanBackward(blockers);
      ret |= POSBOARD(blocker);
    }
  }

  return ret & opposition;
}

BitBoard
BishopMoveTargets(ChessSet *chessSet, Side side, BitBoard bishops)
{
  BitBoard blockers, noea, nowe, occupancy, ret, soea, sowe;
  Position bishop, blocker;

  occupancy = ChessSetOccupancy(chessSet);

  ret = EmptyBoard;
  for(bishop = BitScanForward(bishops); bishops; bishop = BitScanForward(bishops)) {
    bishops ^= POSBOARD(bishop);

    noea = NoEaRay(bishop);
    blockers = noea & occupancy;
    if(blockers != EmptyBoard) {
      blocker = BitScanForward(blockers);
      noea ^= blockers | NoEaRay(blocker);
    }
    ret |= noea;

    nowe = NoWeRay(bishop);
    blockers = nowe & occupancy;
    if(blockers != EmptyBoard) {
      blocker = BitScanForward(blockers);
      nowe ^= blockers | NoWeRay(blocker);
    }
    ret |= nowe;

    soea = SoEaRay(bishop);
    blockers = soea & occupancy;
    if(blockers != EmptyBoard) {
      blocker = BitScanBackward(blockers);
      soea ^= blockers | SoEaRay(blocker);
    }
    ret |= soea;

    sowe = SoWeRay(bishop);
    blockers = sowe & occupancy;
    if(blockers != EmptyBoard) {
      blocker = BitScanBackward(blockers);
      sowe ^= blockers | SoWeRay(blocker);
    }
    ret |= sowe;
  }

  return ret;
}
