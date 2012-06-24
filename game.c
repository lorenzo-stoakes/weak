#include "weak.h"

static bool castleLegal(Game*, bool);
static bool pawnLegal(Game*, Move*);
static bool rookLegal(Game*, Move*);
static bool knightLegal(Game*, Move*);
static bool bishopLegal(Game*, Move*);
static bool queenLegal(Game*, Move*);
static bool kingLegal(Game*, Move*);
static CastleEvent updateCastlingRights(Game*, Move*);

// Get all valid moves for the current player.
MoveSlice
AllMoves(Game *game)
{
  int i, j, k;
  BitBoard enPassantPawns, pawnPushSources, pawnCaptureSources, pawnPushTargets,
    pawnCaptureTargets, fromBoard, toBoard;
  Move move;
  Position from, to;
  Positions sourcePositions, targetPositions;
  Rank promotionRank;
  MoveType promotions[] = {PromoteKnight, PromoteBishop, PromoteRook, PromoteQueen};
  MoveSlice ret = NewMoveSlice();

  // Pawns.

  pawnPushSources = AllPawnPushSources(&game->ChessSet, game->WhosTurn);
  pawnCaptureSources = AllPawnCaptureSources(&game->ChessSet, game->WhosTurn);

  switch(game->WhosTurn) {
  case White:
    enPassantPawns = game->ChessSet.White.Pawns & Rank5Mask;
    promotionRank = Rank8;
    break;
  case Black:
    enPassantPawns = game->ChessSet.Black.Pawns & Rank4Mask;
    promotionRank = Rank1;
    break;
  default:
    panic("Unrecognised side %d.", game->WhosTurn);
  }

  // Pushes.
  sourcePositions = BoardPositions(pawnPushSources);
  for(i = 0; i < sourcePositions.Len; i++) {
    from = sourcePositions.Vals[i];
    pawnPushTargets = PawnPushTargets(&game->ChessSet, game->WhosTurn, POSBOARD(from));
    targetPositions = BoardPositions(pawnPushTargets);
    for(j = 0; j < targetPositions.Len; j++) {
      to = targetPositions.Vals[j];
      move.Piece = Pawn;
      move.From = from;
      move.To = to;
      move.Capture = false;
      move.Type = Normal;

      if(RANK(to) == promotionRank) {
        for(k = 0; k < 4; k++) {
          move.Type = promotions[k];
          if(Legal(game, &move)) {
            ret = AppendMove(ret, move);
          }
        }
      } else if(Legal(game, &move)) {
        ret = AppendMove(ret, move);
      }
    }
    release(targetPositions.Vals);
  }
  release(sourcePositions.Vals);

  // Captures.
  sourcePositions = BoardPositions(pawnCaptureSources);
  for(i = 0; i < sourcePositions.Len; i++) {
    from = sourcePositions.Vals[i];
    pawnCaptureTargets = PawnCaptureTargets(&game->ChessSet, game->WhosTurn, POSBOARD(from));
    targetPositions = BoardPositions(pawnCaptureTargets);
    for(j = 0; j < targetPositions.Len; j++) {
      to = targetPositions.Vals[j];
      move.Piece = Pawn;
      move.From = from;
      move.To = to;
      move.Capture = true;
      move.Type = Normal;

      if(RANK(to) == promotionRank) {
        for(k = 0; k < 4; k++) {
          move.Type = promotions[k];
          if(Legal(game, &move)) {
            ret = AppendMove(ret, move);
          }
        }
      } else if(Legal(game, &move)) {
        ret = AppendMove(ret, move);
      }
    }
    release(targetPositions.Vals);
  }
  release(sourcePositions.Vals);

  // En passant.
  if(enPassantPawns != EmptyBoard) {
    sourcePositions = BoardPositions(enPassantPawns);
    for(i = 0; i < sourcePositions.Len; i++) {
      from = sourcePositions.Vals[i];
      fromBoard = POSBOARD(from);
      switch(game->WhosTurn) {
      case White:
        toBoard = NoWeOne(fromBoard) | NoEaOne(fromBoard);
        break;
      case Black:
        toBoard = SoWeOne(fromBoard) | SoEaOne(fromBoard);
        break;
      default:
        panic("Unrecognised side %d.", game->WhosTurn);
        break;
      }

      targetPositions = BoardPositions(toBoard);
      for(j = 0; j < targetPositions.Len; j++) {
        to = targetPositions.Vals[j];
        move.Piece = Pawn;
        move.From = from;
        move.To = to;
        move.Capture = true;
        move.Type = EnPassant;
        if(Legal(game, &move)) {
          ret = AppendMove(ret, move);
        }
      }
      release(targetPositions.Vals);
    }
    release(sourcePositions.Vals);
  }

  return ret;
}

// Determine whether the current player is checkmated.
bool
Checkmated(Game *game)
{
  return Checked(&game->ChessSet, game->WhosTurn) && AllMoves(game).Len == 0;
}

// Determine whether the game is in a state of stalemate, i.e. the current player cannot make a
// move.
bool
Stalemated(Game *game)
{
  return !Checked(&game->ChessSet, game->WhosTurn) && AllMoves(game).Len == 0;
}

void
DoCastleKingSide(Game *game)
{
  panic("Not implemented.");
}

void
DoCastleQueenSide(Game *game)
{
  panic("Not implemented.");
}

// Determine whether the specified move places the current player into check.
bool
ExposesCheck(Game *game, Move *move)
{
  return false;
}

// Is the proposed move legal in this game?
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

// Attempt to move piece.
void
DoMove(Game *game, Move *move)
{
  Piece piece;
  Position enPassant;
  Rank offset;
  Side opposite = OPPOSITE(game->WhosTurn);

  switch(move->Type) {
  default:
    panic("Move type %d not recognised.", move->Type);
  case CastleKingSide:
    DoCastleKingSide(game);
    break;
  case CastleQueenSide:
    DoCastleQueenSide(game);
    break;
  case EnPassant:
    // Panic, because in the usual course of the game .Legal() would have picked
    // these up, and the move makes no sense without these conditions being true.
    if(move->Piece != Pawn) {
      panic("Expected pawn, en passant performed on piece %s.", move->Piece);
    }
    if(!move->Capture) {
      panic("En passant move, but not capture.");
    }

    switch(game->WhosTurn) {
    case White:
      offset = -1;
      break;
    case Black:
      offset = 1;
      break;
    default:
      panic("Unrecognised side %d.", game->WhosTurn);
    }

    enPassant = POSITION(RANK(move->To)+offset, FILE(move->To));

    piece = ChessSetPieceAt(&game->ChessSet, opposite, enPassant);
    if(piece == MissingPiece) {
      panic("No piece at %s when attempting en passant.", StringPosition(enPassant));
    } else {
      if(piece != Pawn) {
        panic("Piece taken via en passant is %s, not pawn.", piece);
      }
      ChessSetRemovePiece(&game->ChessSet, opposite, piece, enPassant);
      game->History.CapturedPieces = AppendPiece(game->History.CapturedPieces, piece);
    }

    break;
  case PromoteKnight:
  case PromoteBishop:
  case PromoteRook:
  case PromoteQueen:
  case Normal:
    if(move->Capture) {
      piece = ChessSetPieceAt(&game->ChessSet, opposite, move->To);
      if(piece == MissingPiece) {
        panic("No piece at %s when attempting capture %s.", StringPosition(move->To),
              StringMove(move));
      } else {
        ChessSetRemovePiece(&game->ChessSet, opposite, piece, move->To);
        game->History.CapturedPieces = AppendPiece(game->History.CapturedPieces, piece);
      }
    }

    if(move->Type == Normal) {
      piece = move->Piece;
    } else if(move->Type == PromoteKnight) {
      piece = Knight;
    } else if(move->Type == PromoteBishop) {
      piece = Bishop;
    } else if(move->Type == PromoteRook) {
      piece = Rook;
    } else if(move->Type == PromoteQueen) {
      piece = Queen;
    } else {
      panic("Impossible.");
    }

    ChessSetRemovePiece(&game->ChessSet, game->WhosTurn, move->Piece, move->From);
    ChessSetPlacePiece(&game->ChessSet, game->WhosTurn, piece, move->To);
  }

  game->History.CastleEvents = AppendCastleEvent(game->History.CastleEvents,
                                                 updateCastlingRights(game, move));
  ToggleTurn(game);
}

// Create new game with specified human side and white + black's pieces in standard initial
// positions.
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

// Toggle whose turn it is.
void
ToggleTurn(Game *game) {
  game->WhosTurn = OPPOSITE(game->WhosTurn);
}

void
Unmove(Game *game)
{
  panic("Not implemented.");
}

static bool
castleLegal(Game *game, bool queenSide)
{
  return false;
}

// Is this pawn move legal?
static bool
pawnLegal(Game *game, Move *move)
{
  BitBoard expectedFromBoard, lastToBoard, sources, targets, validToRankMask;
  BitBoard fromBoard = POSBOARD(move->From), toBoard = POSBOARD(move->To);
  int dist;
  MoveSlice hist;
  Move lastMove;
  Position expectedTo, lastFrom, lastTo;
  Rank expectedRank, offset, rank;

  switch(move->Type) {
  default:
    panic("Move type %d in pawnLegal.", move->Type);
  case EnPassant:
    // En passants are captures. Full stop.
    if(!move->Capture) {
      return false;
    }

    // We have to check the last move to see whether en passant is valid.
    hist = game->History.Moves;
    if(hist.Len == 0) {
      return false;
    }
    lastMove = hist.Vals[hist.Len-1];
    lastFrom = lastMove.From;
    lastTo = lastMove.To;
    if(lastMove.Piece != Pawn) {
      return false;
    }
    dist = RANK(lastFrom) - RANK(lastTo);
    if(dist != 2 && dist != -2) {
      return false;
    }

    // En passant has to originate from either side of where the pawn has now moved.
    lastToBoard = POSBOARD(lastTo);
    expectedFromBoard = WestOne(lastToBoard) | EastOne(lastToBoard);
    if((fromBoard&expectedFromBoard) != fromBoard) {
      return false;
    }

    // Have to capture into square immediately behind just moved opposing pawn.
    if(game->WhosTurn == White) {
      offset = -1;
    } else if(game->WhosTurn == Black) {
      offset = 1;
    } else {
      panic("Unrecognised side %d.", game->WhosTurn);
    }
    expectedTo = POSITION(RANK(lastFrom)+offset, FILE(lastFrom));
    if(move->To != expectedTo) {
      return false;
    }

    return true;
  case PromoteKnight:
  case PromoteBishop:
  case PromoteRook:
  case PromoteQueen:
    rank = RANK(move->To);
    if(game->WhosTurn == White) {
      expectedRank = Rank8;
      validToRankMask = Rank8Mask;
    } else if(game->WhosTurn == Black) {
      expectedRank = Rank1;
      validToRankMask = Rank1Mask;
    } else {
      panic("Unrecognised side %d.", game->WhosTurn);
    }
  if(rank != expectedRank) {
    return false;
  }
  break;
  case Normal:
    if(game->WhosTurn == White) {
      validToRankMask = NotRank8Mask;
    } else if(game->WhosTurn == Black) {
      validToRankMask = NotRank1Mask;
    } else {
      panic("Unrecognised side %d.", game->WhosTurn);
    }
    break;
  }

  if(move->Capture) {
    sources = PawnCaptureSources(&game->ChessSet, game->WhosTurn, fromBoard);
    targets = PawnCaptureTargets(&game->ChessSet, game->WhosTurn, fromBoard);
  } else {
    sources = PawnPushSources(&game->ChessSet, game->WhosTurn, fromBoard);
    targets = PawnPushTargets(&game->ChessSet, game->WhosTurn, fromBoard);
  }

  // Since sources are referenced against our single candidate pawn, their being non-empty
  // means the pawn is able to move. If the pawn's valid targets and our 'to' position
  // intersect, then the to position is valid.
  return sources != EmptyBoard && (targets&toBoard&validToRankMask) != EmptyBoard;
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

static CastleEvent updateCastlingRights(Game *game, Move move)
{
  return NoCastleEvent;
}
