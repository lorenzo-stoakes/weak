#include "weak.h"

static BitBoard
pawnSinglePushSources(Side side, ChessSet *chessSet, BitBoard pawns)
{
  return EmptyBoard;  
}

static BitBoard
pawnDoublePushSources(Side side, ChessSet *chessSet, BitBoard pawns)
{
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
}

BitBoard
PawnPushSources(Side side, ChessSet *chessSet, BitBoard pawns)
{
  return pawnSinglePushSources(side, chessSet, pawns) |
    pawnDoublePushSources(side, chessSet, pawns);
}

