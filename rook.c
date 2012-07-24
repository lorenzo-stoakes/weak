#include <stdio.h>
#include "weak.h"
#include "magic.h"

static FORCE_INLINE BitBoard magicSquareThreats(Position, BitBoard);

// Get BitBoard encoding capture targets for *all* rooks on specified side.
BitBoard
AllRookCaptureTargets(ChessSet *chessSet, Side side)
{
  return RookCaptureTargets(chessSet, side, chessSet->Sets[side].Boards[Rook]);
}

// Get BitBoard encoding move targets for *all* rooks on specified side.
BitBoard
AllRookMoveTargets(ChessSet *chessSet, Side side)
{
  return RookMoveTargets(chessSet, side, chessSet->Sets[side].Boards[Rook]);
}

BitBoard
RookAttacksFrom(Position rook, BitBoard occupancy)
{
  return magicSquareThreats(rook, occupancy);
}

BitBoard
RookQueenAttackersTo(ChessSet *chessSet, Position to, BitBoard occupancy)
{
  return (chessSet->Sets[White].Boards[Rook] |
    chessSet->Sets[Black].Boards[Rook] |
    chessSet->Sets[White].Boards[Queen] |
    chessSet->Sets[Black].Boards[Queen]) &
    magicSquareThreats(to, occupancy);
}

// Get BitBoard encoding capture targets for specified rooks without using magic BitBoards.
BitBoard
RookCaptureTargets(ChessSet *chessSet, Side side, BitBoard rooks)
{
  BitBoard opposition = chessSet->Sets[OPPOSITE(side)].Occupancy;
  BitBoard ret = EmptyBoard;
  Position rook;

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

// Get rook threats for the specified rooks.
BitBoard
RookThreats(BitBoard rooks, BitBoard occupancy)
{
  BitBoard ret = EmptyBoard;
  Position rook;

  while(rooks != EmptyBoard) {
    rook = PopForward(&rooks);
    ret |= magicSquareThreats(rook, occupancy);
  }

  return ret;
}

static FORCE_INLINE BitBoard
magicSquareThreats(Position rook, BitBoard occupancy) {
  BitBoard magic = magicBoard[MAGIC_ROOK][rook];
  BitBoard mask = magicMask[MAGIC_ROOK][rook];
  int shift = magicShift[MAGIC_ROOK][rook];
  BitBoard index = (magic*(occupancy&mask))>>shift;

  return RookThreatBase[rook][index];
}
