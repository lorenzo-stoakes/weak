#include "weak.h"

BitBoard
ChessSetOccupancy(ChessSet *chessSet)
{
  return SetOccupancy(chessSet->White) + SetOccupancy(chessSet->Black);
}

BitBoard
EmptySquares(ChessSet *chessSet)
{
  return ~ChessSetOccupancy(chessSet);
}

ChessSet
NewChessSet()
{
  ChessSet ret;

  ret.White = NewWhiteSet();
  ret.Black = NewBlackSet();

  return ret;
}
