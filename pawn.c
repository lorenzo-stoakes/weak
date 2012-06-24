#include "weak.h"

static BitBoard
singlePushSources(Side side, ChessSet *chessSet, BitBoard pawns)
{
  switch(side) {
  case White:
    return SoutOne(EmptySquares(chessSet)) & pawns;
  case Black:
    return NortOne(EmptySquares(chessSet)) & pawns;
  }

  panic("Invalid side %d.", side);
  return EmptyBoard;  
}

static BitBoard
doublePushSources(Side side, ChessSet *chessSet, BitBoard pawns)
{
  BitBoard singlePushes = singlePushSources(side, chessSet, pawns);

  switch(side) {
  case White:
    return NortOne(singlePushes) & EmptySquares(chessSet) & Rank4Mask;
  case Black:
    return SoutOne(singlePushes) & EmptySquares(chessSet) & Rank5Mask;
  }

  panic("Invalid side %d.", side);
  return EmptyBoard;
}

BitBoard
AllPawnPushSources(Side side, ChessSet *chessSet)
{
  switch(side) {
  case White:
    return PawnPushSources(side, chessSet, chessSet->White.Pawns);
  case Black:
    return PawnPushSources(side, chessSet, chessSet->Black.Pawns);
  }

  panic("Invalid side %s.", StringSide(side));
  return EmptyBoard;
}

BitBoard
PawnPushSources(Side side, ChessSet *chessSet, BitBoard pawns)
{
  return singlePushSources(side, chessSet, pawns) |
    doublePushSources(side, chessSet, pawns);
}

