#include "weak.h"
#include "magic.h"

static FORCE_INLINE BitBoard magicSquareThreats(Position, BitBoard);

BitBoard
AllBishopCaptureTargets(ChessSet *chessSet, Side side)
{
  return BishopCaptureTargets(chessSet, side, chessSet->Sets[side].Boards[Bishop]);
}

BitBoard
AllBishopMoveTargets(ChessSet *chessSet, Side side)
{
  return BishopMoveTargets(chessSet, side, chessSet->Sets[side].Boards[Bishop]);
}

BitBoard
BishopAttacksFrom(Position bishop, BitBoard occupancy)
{
  return magicSquareThreats(bishop, occupancy);
}

BitBoard
BishopQueenAttackersTo(ChessSet *chessSet, Position to, BitBoard occupancy)
{
  return (chessSet->Sets[White].Boards[Bishop] |
    chessSet->Sets[Black].Boards[Bishop] |
    chessSet->Sets[White].Boards[Queen] |
    chessSet->Sets[Black].Boards[Queen]) &
    magicSquareThreats(to, occupancy);
}

BitBoard
BishopCaptureTargets(ChessSet *chessSet, Side side, BitBoard bishops)
{
  BitBoard opposition = chessSet->Sets[OPPOSITE(side)].Occupancy;
  BitBoard ret = EmptyBoard;
  Position bishop;

  while(bishops) {
    bishop = PopForward(&bishops);

    ret |= magicSquareThreats(bishop, chessSet->Occupancy);
  }

  return ret & opposition;
}

BitBoard
BishopMoveTargets(ChessSet *chessSet, Side side, BitBoard bishops)
{
  BitBoard ret = EmptyBoard;
  Position bishop;

  while(bishops) {
    bishop = PopForward(&bishops);

    ret |= magicSquareThreats(bishop, chessSet->Occupancy);
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

BitBoard
BishopThreats(BitBoard bishops, BitBoard occupancy)
{
  BitBoard ret = EmptyBoard;
  Position bishop;

  while(bishops != EmptyBoard) {
    bishop = PopForward(&bishops);

    ret |= magicSquareThreats(bishop, occupancy);
  }

  return ret;
}

static FORCE_INLINE BitBoard
magicSquareThreats(Position bishop, BitBoard occupancy) {
  BitBoard magic = magicBoard[MAGIC_BISHOP][bishop];
  BitBoard mask = magicMask[MAGIC_BISHOP][bishop];
  int shift = magicShift[MAGIC_BISHOP][bishop];
  BitBoard index = (magic*(occupancy&mask))>>shift;

  return BishopThreatBase[bishop][index];
}
