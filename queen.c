#include "weak.h"

BitBoard
AllQueenCaptureTargets(ChessSet *chessSet, Side side)
{
  return QueenCaptureTargets(chessSet, side, chessSet->Sets[side].Boards[Queen]);
}

BitBoard
AllQueenMoveTargets(ChessSet *chessSet, Side side)
{
  return QueenMoveTargets(chessSet, side, chessSet->Sets[side].Boards[Queen]);
}

BitBoard
QueenKingThreats(ChessSet *chessSet, Side side)
{
  return QueenThreats(chessSet->Sets[side].Boards[Queen],
                      chessSet->Occupancy ^ chessSet->Sets[OPPOSITE(side)].Boards[King]);
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

BitBoard
QueenThreats(BitBoard queens, BitBoard occupancy) {
  return RookThreats(queens, occupancy) | BishopThreats(queens, occupancy);
}
