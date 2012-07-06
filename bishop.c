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
  BitBoard ret, bishops;
  Position bishop;

  switch(side) {
  case White:
    bishops = chessSet->White.Bishops;
    break;
  case Black:
    bishops = chessSet->Black.Bishops;
    break;
  default:
    panic("Unrecognised side %d.", side);
  }

  ret = EmptyBoard;
  for(; bishops; bishops ^= POSBOARD(bishop)) {
    bishop = BitScanForward(bishops);

    ret |= BishopSquareThreats(bishop, chessSet->Occupancy);
  }

  return ret;
}

BitBoard
BishopCaptureTargets(ChessSet *chessSet, Side side, BitBoard bishops)
{
  BitBoard opposition;
  BitBoard ret = EmptyBoard;
  Position bishop;

  switch(side) {
  case White:
    opposition = chessSet->Black.Occupancy;
    break;
  case Black:
    opposition = chessSet->White.Occupancy;
    break;
  default:
    panic("Invalid side %d.", side);
  }

  for(; bishops; bishops ^= POSBOARD(bishop)) {
    bishop = BitScanForward(bishops);

    ret |= BishopSquareThreats(bishop, chessSet->Occupancy);
  }

  return ret & opposition;
}

BitBoard
BishopMoveTargets(ChessSet *chessSet, Side side, BitBoard bishops)
{
  BitBoard ret = EmptyBoard;
  Position bishop;

  for(; bishops; bishops ^= POSBOARD(bishop)) {
    bishop = BitScanForward(bishops);

    ret |= BishopSquareThreats(bishop, chessSet->Occupancy);
  }

  return ret & chessSet->EmptySquares;
}

BitBoard
BishopSquareThreats(Position bishop, BitBoard occupancy)
{
  BitBoard blockers, noea, nowe, soea, sowe;
  BitBoard ret = EmptyBoard;
  Position blocker;

  noea = NoEaRay(bishop);
  blockers = noea & occupancy;
  if(blockers != EmptyBoard) {
    blocker = BitScanForward(blockers);
    noea &= ~NoEaRay(blocker);
  }
  ret |= noea;

  nowe = NoWeRay(bishop);
  blockers = nowe & occupancy;
  if(blockers != EmptyBoard) {
    blocker = BitScanForward(blockers);
    nowe &= ~NoWeRay(blocker);
  }
  ret |= nowe;

  soea = SoEaRay(bishop);
  blockers = soea & occupancy;
  if(blockers != EmptyBoard) {
    blocker = BitScanBackward(blockers);
    soea &= ~SoEaRay(blocker);
  }
  ret |= soea;

  sowe = SoWeRay(bishop);
  blockers = sowe & occupancy;
  if(blockers != EmptyBoard) {
    blocker = BitScanBackward(blockers);
    sowe &= ~SoWeRay(blocker);
  }
  ret |= sowe;

  return ret;
}
