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
  // TODO: Clean up the duplication, for God's sake :-)

  int i, j, k;
  BitBoard bishops, captureTargets, enPassantPawns, fromBoard, kings, knights, moveTargets,
    pawnCaptureSources, pawnPushSources, pawnCaptureTargets, pawnPushTargets, queens, rooks,
    toBoard;
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

  // Rooks.

  switch(game->WhosTurn) {
  case White:
    rooks = game->ChessSet.White.Rooks;
    break;
  case Black:
    rooks = game->ChessSet.Black.Rooks;
    break;
  default:
    panic("Invalid side %d.", game->WhosTurn);
  }

  sourcePositions = BoardPositions(rooks);
  for(i = 0; i < sourcePositions.Len; i++) {
    from = sourcePositions.Vals[i];

    moveTargets = RookMoveTargets(&game->ChessSet, game->WhosTurn, POSBOARD(from));
    captureTargets = RookCaptureTargets(&game->ChessSet, game->WhosTurn, POSBOARD(from));

    // Moves.
    targetPositions = BoardPositions(moveTargets);
    for(j = 0; j < targetPositions.Len; j++) {
      to = targetPositions.Vals[j];

      move.Piece = Rook;
      move.From = from;
      move.To = to;
      move.Capture = false;
      move.Type = Normal;
      if(Legal(game, &move)) {
        ret = AppendMove(ret, move);
      }
    }

    // Captures.
    targetPositions = BoardPositions(captureTargets);
    for(j = 0; j < targetPositions.Len; j++) {
      to = targetPositions.Vals[j];

      move.Piece = Rook;
      move.From = from;
      move.To = to;
      move.Capture = true;
      move.Type = Normal;
      if(Legal(game, &move)) {
        ret = AppendMove(ret, move);
      }
    }
  }

  // Knights.

  switch(game->WhosTurn) {
  case White:
    knights = game->ChessSet.White.Knights;
    break;
  case Black:
    knights = game->ChessSet.Black.Knights;
    break;
  default:
    panic("Invalid side %d.", game->WhosTurn);
  }

  sourcePositions = BoardPositions(knights);
  for(i = 0; i < sourcePositions.Len; i++) {
    from = sourcePositions.Vals[i];

    moveTargets = KnightMoveTargets(&game->ChessSet, game->WhosTurn, POSBOARD(from));
    captureTargets = KnightCaptureTargets(&game->ChessSet, game->WhosTurn, POSBOARD(from));

    // Moves.
    targetPositions = BoardPositions(moveTargets);
    for(j = 0; j < targetPositions.Len; j++) {
      to = targetPositions.Vals[j];

      move.Piece = Knight;
      move.From = from;
      move.To = to;
      move.Capture = false;
      move.Type = Normal;
      if(Legal(game, &move)) {
        ret = AppendMove(ret, move);
      }
    }

    // Captures.
    targetPositions = BoardPositions(captureTargets);
    for(j = 0; j < targetPositions.Len; j++) {
      to = targetPositions.Vals[j];

      move.Piece = Knight;
      move.From = from;
      move.To = to;
      move.Capture = true;
      move.Type = Normal;
      if(Legal(game, &move)) {
        ret = AppendMove(ret, move);
      }
    }
  }

  // Bishops.

  switch(game->WhosTurn) {
  case White:
    bishops = game->ChessSet.White.Bishops;
    break;
  case Black:
    bishops = game->ChessSet.Black.Bishops;
    break;
  default:
    panic("Invalid side %d.", game->WhosTurn);
  }

  sourcePositions = BoardPositions(bishops);
  for(i = 0; i < sourcePositions.Len; i++) {
    from = sourcePositions.Vals[i];

    moveTargets = BishopMoveTargets(&game->ChessSet, game->WhosTurn, POSBOARD(from));
    captureTargets = BishopCaptureTargets(&game->ChessSet, game->WhosTurn, POSBOARD(from));

    // Moves.
    targetPositions = BoardPositions(moveTargets);
    for(j = 0; j < targetPositions.Len; j++) {
      to = targetPositions.Vals[j];

      move.Piece = Bishop;
      move.From = from;
      move.To = to;
      move.Capture = false;
      move.Type = Normal;
      if(Legal(game, &move)) {
        ret = AppendMove(ret, move);
      }
    }

    // Captures.
    targetPositions = BoardPositions(captureTargets);
    for(j = 0; j < targetPositions.Len; j++) {
      to = targetPositions.Vals[j];

      move.Piece = Bishop;
      move.From = from;
      move.To = to;
      move.Capture = true;
      move.Type = Normal;
      if(Legal(game, &move)) {
        ret = AppendMove(ret, move);
      }
    }
  }

  // Queens.

  switch(game->WhosTurn) {
  case White:
    queens = game->ChessSet.White.Queens;
    break;
  case Black:
    queens = game->ChessSet.Black.Queens;
    break;
  default:
    panic("Invalid side %d.", game->WhosTurn);
  }

  sourcePositions = BoardPositions(queens);
  for(i = 0; i < sourcePositions.Len; i++) {
    from = sourcePositions.Vals[i];

    moveTargets = QueenMoveTargets(&game->ChessSet, game->WhosTurn, POSBOARD(from));
    captureTargets = QueenCaptureTargets(&game->ChessSet, game->WhosTurn, POSBOARD(from));

    // Moves.
    targetPositions = BoardPositions(moveTargets);
    for(j = 0; j < targetPositions.Len; j++) {
      to = targetPositions.Vals[j];

      move.Piece = Queen;
      move.From = from;
      move.To = to;
      move.Capture = false;
      move.Type = Normal;
      if(Legal(game, &move)) {
        ret = AppendMove(ret, move);
      }
    }

    // Captures.
    targetPositions = BoardPositions(captureTargets);
    for(j = 0; j < targetPositions.Len; j++) {
      to = targetPositions.Vals[j];

      move.Piece = Queen;
      move.From = from;
      move.To = to;
      move.Capture = true;
      move.Type = Normal;
      if(Legal(game, &move)) {
        ret = AppendMove(ret, move);
      }
    }
  }

    // Kings.

  switch(game->WhosTurn) {
  case White:
    kings = game->ChessSet.White.King;
    break;
  case Black:
    kings = game->ChessSet.Black.King;
    break;
  default:
    panic("Invalid side %d.", game->WhosTurn);
  }

  // We allow 0 kings for the purposes of testing.
  // TODO: Review.

  sourcePositions = BoardPositions(kings);
  if(sourcePositions.Len > 1) {
    panic("%s has %d kings, expected 0 or 1.", StringSide(game->WhosTurn),
          sourcePositions.Len);
  }

  if(sourcePositions.Len == 1) {
    from = sourcePositions.Vals[0];

    moveTargets = KingMoveTargets(&game->ChessSet, game->WhosTurn);
    captureTargets = KingCaptureTargets(&game->ChessSet, game->WhosTurn);

    // Moves.
    targetPositions = BoardPositions(moveTargets);
    for(j = 0; j < targetPositions.Len; j++) {
      to = targetPositions.Vals[j];

      move.Piece = King;
      move.From = from;
      move.To = to;
      move.Capture = false;
      move.Type = Normal;
      if(Legal(game, &move)) {
        ret = AppendMove(ret, move);
      }
    }

    // Captures.
    targetPositions = BoardPositions(captureTargets);
    for(j = 0; j < targetPositions.Len; j++) {
      to = targetPositions.Vals[j];

      move.Piece = King;
      move.From = from;
      move.To = to;
      move.Capture = true;
      move.Type = Normal;
      if(Legal(game, &move)) {
        ret = AppendMove(ret, move);
      }
    }    
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
  bool ret;

  DoMove(game, move);
  ret = Checked(&game->ChessSet, OPPOSITE(game->WhosTurn));
  Unmove(game);

  return ret;
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
  game->History.Moves = AppendMove(game->History.Moves, *move);
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

// Attempt to undo move.
void
Unmove(Game *game)
{
  CastleEvent castleEvent;
  Move move;
  Piece captured, piece;
  Position to;
  Rank offset;
  Side opposite;

  // Rollback to previous turn.
  move = PopMove(&game->History.Moves);
  ToggleTurn(game);

  switch(move.Type) {
  case EnPassant:
  case Normal:
    piece = move.Piece;
    break;
  case PromoteKnight:
    piece = Knight;
    break;
  case PromoteBishop:
    piece = Bishop;
    break;
  case PromoteRook:
    piece = Rook;
    break;
  case PromoteQueen:
    piece = Queen;
    break;
  case CastleQueenSide:
  case CastleKingSide:
    ;
  }

  switch(move.Type) {
  case EnPassant:
  case PromoteKnight:
  case PromoteBishop:
  case PromoteRook:
  case PromoteQueen:
  case Normal:
    ChessSetRemovePiece(&game->ChessSet, game->WhosTurn, piece, move.To);
    ChessSetPlacePiece(&game->ChessSet, game->WhosTurn, move.Piece, move.From);

    if(move.Capture) {
        captured = PopPiece(&game->History.CapturedPieces);

        opposite = OPPOSITE(game->WhosTurn);
        if(move.Type == EnPassant) {
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
          to = POSITION(RANK(move.To)+offset, FILE(move.To));
          ChessSetPlacePiece(&game->ChessSet, opposite, captured, to);
        } else {
          ChessSetPlacePiece(&game->ChessSet, opposite, captured, move.To);
        }
    }
    break;
  case CastleQueenSide:
    if(game->WhosTurn == White) {
      ChessSetRemovePiece(&game->ChessSet, game->WhosTurn, Rook, D1);
      ChessSetPlacePiece(&game->ChessSet, game->WhosTurn, Rook, A1);
      ChessSetRemovePiece(&game->ChessSet, game->WhosTurn, King, C1);
      ChessSetPlacePiece(&game->ChessSet, game->WhosTurn, King, E1);
    } else if(game->WhosTurn == Black) {
      ChessSetRemovePiece(&game->ChessSet, game->WhosTurn, Rook, D8);
      ChessSetPlacePiece(&game->ChessSet, game->WhosTurn, Rook, A8);
      ChessSetRemovePiece(&game->ChessSet, game->WhosTurn, King, C8);
      ChessSetPlacePiece(&game->ChessSet, game->WhosTurn, King, E8);
    } else {
      panic("Invalid side %d.", game->WhosTurn);
    }
    break;
  case CastleKingSide:
    if(game->WhosTurn == White) {
      ChessSetRemovePiece(&game->ChessSet, game->WhosTurn, Rook, F1);
      ChessSetPlacePiece(&game->ChessSet, game->WhosTurn, Rook, H1);
      ChessSetRemovePiece(&game->ChessSet, game->WhosTurn, King, G1);
      ChessSetPlacePiece(&game->ChessSet, game->WhosTurn, King, E1);
    } else if(game->WhosTurn == Black) {
      ChessSetRemovePiece(&game->ChessSet, game->WhosTurn, Rook, F8);
      ChessSetPlacePiece(&game->ChessSet, game->WhosTurn, Rook, H8);
      ChessSetRemovePiece(&game->ChessSet, game->WhosTurn, King, G8);
      ChessSetPlacePiece(&game->ChessSet, game->WhosTurn, King, E8);
    } else {
      panic("Invalid side %d.", game->WhosTurn);
    }
    break;
  default:
    panic("Unrecognised move type %d.", move.Type);
  }

  castleEvent = PopCastleEvent(&game->History.CastleEvents);

  switch(game->WhosTurn) {
  case White:
    if((castleEvent&LostQueenSideWhite) == LostQueenSideWhite) {
      game->CastleQueenSideWhite = true;
    }
    if((castleEvent&LostKingSideWhite) == LostKingSideWhite) {
      game->CastleKingSideWhite = true;
    }
    break;
  case Black:
    if((castleEvent&LostQueenSideBlack) == LostQueenSideBlack) {
      game->CastleQueenSideBlack = true;
    }
    if((castleEvent&LostKingSideBlack) == LostKingSideBlack) {
      game->CastleKingSideBlack = true;
    }
    break;
  default:
    panic("Invalid side %d.", game->WhosTurn);
  }
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
  BitBoard from, to;

  from = POSBOARD(move->From);
  to = POSBOARD(move->To);

  if(move->Capture) {
    return (to&RookCaptureTargets(&game->ChessSet, game->WhosTurn, from)) != EmptyBoard;
  }

  return (to&RookMoveTargets(&game->ChessSet, game->WhosTurn, from)) != EmptyBoard;
}

static bool knightLegal(Game *game, Move *move)
{
  BitBoard from, to;

  from = POSBOARD(move->From);
  to = POSBOARD(move->To);

  if(move->Capture) {
    return (to&KnightCaptureTargets(&game->ChessSet, game->WhosTurn, from)) != EmptyBoard;
  }

  return (to&KnightMoveTargets(&game->ChessSet, game->WhosTurn, from)) != EmptyBoard;
}

static bool bishopLegal(Game *game, Move *move)
{
  BitBoard from, to;

  from = POSBOARD(move->From);
  to = POSBOARD(move->To);

  if(move->Capture) {
    return (to&BishopCaptureTargets(&game->ChessSet, game->WhosTurn, from)) != EmptyBoard;
  }

  return (to&BishopMoveTargets(&game->ChessSet, game->WhosTurn, from)) != EmptyBoard;  
}

static bool queenLegal(Game *game, Move *move)
{
  BitBoard from, to;

  from = POSBOARD(move->From);
  to = POSBOARD(move->To);

  if(move->Capture) {
    return (to&QueenCaptureTargets(&game->ChessSet, game->WhosTurn, from)) != EmptyBoard;
  }

  return (to&QueenMoveTargets(&game->ChessSet, game->WhosTurn, from)) != EmptyBoard;  
}

static bool kingLegal(Game *game, Move *move)
{
  BitBoard from, to;

  from = POSBOARD(move->From);
  to = POSBOARD(move->To);

  if(move->Capture) {
    return (to&KingCaptureTargets(&game->ChessSet, game->WhosTurn)) != EmptyBoard;
  }

  return (to&KingMoveTargets(&game->ChessSet, game->WhosTurn)) != EmptyBoard;  
}

static CastleEvent updateCastlingRights(Game *game, Move *move)
{
  CastleEvent ret = NoCastleEvent;

  switch(move->Type) {
  default:
    return NoCastleEvent;
  case Normal:
    if(move->Piece != King && move->Piece != Rook) {
      return NoCastleEvent;
    }
    break;
  case CastleKingSide:
  case CastleQueenSide:
    if(game->WhosTurn == White) {
        return LostKingSideWhite | LostQueenSideWhite;
    } else if(game->WhosTurn == Black) {
        return LostKingSideBlack | LostQueenSideBlack;
    } else {
      panic("Unrecognised side %d.", game->WhosTurn);
    }
  }

  switch(game->WhosTurn) {
  case White:
    if(move->Piece == King && move->From == E1) {
      if(game->CastleQueenSideWhite) {
        ret = LostQueenSideWhite;
        game->CastleQueenSideWhite = false;
      }
      if(game->CastleKingSideWhite) {
        ret |= LostKingSideWhite;
        game->CastleKingSideWhite = false;
      }
    } else if(game->CastleQueenSideWhite && move->Piece == Rook && move->From == A1) {
      ret = LostQueenSideWhite;
      game->CastleQueenSideWhite = false;
    } else if(game->CastleKingSideWhite && move->Piece == Rook && move->From == H1) {
      ret = LostKingSideWhite;
      game->CastleKingSideWhite = false;
    }
    break;
  case Black:
    if(move->Piece == King && move->From == E8) {
      if(game->CastleQueenSideBlack) {
        ret = LostQueenSideBlack;
        game->CastleQueenSideBlack = false;
      }
      if(game->CastleKingSideBlack) {
        ret |= LostKingSideBlack;
        game->CastleKingSideBlack = false;
      }
    } else if(game->CastleQueenSideBlack && move->Piece == Rook && move->From == A8) {
      ret = LostQueenSideBlack;
      game->CastleQueenSideBlack = false;
    } else if(game->CastleKingSideBlack && move->Piece == Rook && move->From == H8) {
      ret = LostKingSideBlack;
      game->CastleKingSideBlack = false;
    }
    break;
  default:
    panic("Invalid side %d.", game->WhosTurn);
  }

  return ret;
}
