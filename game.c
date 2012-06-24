#include "weak.h"

static bool castleLegal(Game*, bool);
static bool pawnLegal(Game*, Move*);
static bool rookLegal(Game*, Move*);
static bool knightLegal(Game*, Move*);
static bool bishopLegal(Game*, Move*);
static bool queenLegal(Game*, Move*);
static bool kingLegal(Game*, Move*);

bool
ExposesCheck(Game *game, Move *move)
{
  return false;
}

bool
Legal(Game *game, Move *move)
{
  bool pieceLegal;
  Piece piece;

  switch(move->Type) {
  default:
    panic("Move type %d not yet implemented.", move->Type);
    break;
  case CastleQueenSide:
    return castleLegal(game, true);
  case CastleKingSide:
    return castleLegal(game, false);
  case EnPassant:
  case PromoteKnight:
  case PromoteBishop:
  case PromoteRook:
  case PromoteQueen:
  case Normal:
    ;
  }

  // Moves which accomplish nothing are illegal.
  if(move->From == move->To) {
    return false;
  }

  // Have to move from an occupied square, and it has to be the piece the move purports
  // it to be.  
  piece = ChessSetPieceAt(&game->ChessSet, game->WhosTurn, move->From);
  if(piece == MissingPiece || piece != move->Piece) {
    return false;
  }
  
  switch(move->Piece) {
  case Pawn:
    pieceLegal = pawnLegal(game, move);
    break;
  case Rook:
    pieceLegal = rookLegal(game, move);
    break;
  case Knight:
    pieceLegal = knightLegal(game, move);
    break;
  case Bishop:
    pieceLegal = bishopLegal(game, move);
    break;
  case Queen:
    pieceLegal = queenLegal(game, move);
    break;
  case King:
    pieceLegal = kingLegal(game, move);
    break;
  default:
    panic("Unrecognised piece %d.", move->Piece);
  }

  return pieceLegal && !ExposesCheck(game, move);
}

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

static bool
castleLegal(Game *game, bool queenSide)
{
  return false;
}

static bool
pawnLegal(Game *game, Move *move)
{
  return false;
}

static bool rookLegal(Game *game, Move *move)
{
  return false;
}

static bool knightLegal(Game *game, Move *move)
{
  return false;
}

static bool bishopLegal(Game *game, Move *move)
{
  return false;
}

static bool queenLegal(Game *game, Move *move)
{
  return false;
}

static bool kingLegal(Game *game, Move *move)
{
  return false;
}
