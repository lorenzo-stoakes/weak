#include "weak.h"

Game
NewGame(bool debug, Side humanSide)
{
  Game ret;

  ret.CastleKingSideWhite = true;
  ret.CastleQueenSideWhite = true;
  ret.CastleKingSideBlack = true;
  ret.CastleQueenSideBlack = true;
  ret.ChessSet = NewChessSet();
  ret.Debug = debug;
  ret.WhosTurn = White;
  ret.HumanSide = humanSide;
  ret.History = NewMoveHistory();

  return ret;
}
