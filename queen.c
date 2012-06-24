#include "weak.h"

BitBoard
AllQueenCaptureTargets(ChessSet *chessSet, Side side)
{
  switch(side) {
  case White:
    return QueenCaptureTargets(chessSet, side, chessSet->White.Queens);
  case Black:
    return QueenCaptureTargets(chessSet, side, chessSet->Black.Queens);    
  }

  panic("Invalid side %d.", side);
  return EmptyBoard;
}

BitBoard
AllQueenMoveTargets(ChessSet *chessSet, Side side)
{
  switch(side) {
  case White:
    return QueenMoveTargets(chessSet, side, chessSet->White.Queens);
  case Black:
    return QueenMoveTargets(chessSet, side, chessSet->Black.Queens);    
  }

  panic("Invalid side %d.", side);  
  return EmptyBoard;
}

BitBoard
AllQueenThreats(ChessSet *chessSet, Side side)
{
  return AllQueenMoveTargets(chessSet, side) | AllQueenCaptureTargets(chessSet, side);
}

BitBoard
QueenCaptureTargets(ChessSet *chessSet, Side side, BitBoard queens)
{
  return RookCaptureTargets(chessSet, side, queens) |
    BishopCaptureTargets(chessSet, side, queens);
}

BitBoard
QueenMoveTargets(ChessSet *chessSet, Side side, BitBoard queens)
{
  return RookMoveTargets(chessSet, side, queens) | BishopMoveTargets(chessSet, side, queens);
}
