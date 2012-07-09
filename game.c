#include "weak.h"

static bool        castleLegal(Game*, bool);
static bool        pawnLegal(Game*, Move*);
static bool        pieceLegal(Piece, Game*, Move*);
static CastleEvent updateCastlingRights(Game*, Move*);

static void castleMoves(Game*, MoveSlice*);
static void pawnMoves(Game*, BitBoard, MoveSlice*);
static void pieceMoves(Piece, Game*, BitBoard, MoveSlice*);

// Get all valid moves for the current player.
void
AllMoves(MoveSlice *slice, Game *game)
{
  BitBoard kingThreats;
  Piece piece;

  kingThreats = KingThreats(&game->ChessSet, OPPOSITE(game->WhosTurn));
  pawnMoves(game, kingThreats, slice);
  for(piece = Knight; piece <= King; piece++) {
    pieceMoves(piece, game, kingThreats, slice);
  }
  castleMoves(game, slice);
}

// Determine whether the current player is checkmated.
bool
Checkmated(Game *game)
{
  Move buffer[INIT_MOVE_LEN];
  MoveSlice slice = NewMoveSlice(buffer, INIT_MOVE_LEN);

  AllMoves(&slice, game);

  return slice.Len == 0 && Checked(&game->ChessSet, game->WhosTurn);
}

void
DoCastleKingSide(Game *game)
{
  int offset = game->WhosTurn*8*7;

  ChessSetRemovePiece(&game->ChessSet, game->WhosTurn, Rook, H1 + offset);
  ChessSetPlacePiece(&game->ChessSet, game->WhosTurn, Rook, F1 + offset);
  ChessSetRemovePiece(&game->ChessSet, game->WhosTurn, King, E1 + offset);
  ChessSetPlacePiece(&game->ChessSet, game->WhosTurn, King, G1 + offset);
}

void
DoCastleQueenSide(Game *game)
{
  int offset = game->WhosTurn*8*7;

  ChessSetRemovePiece(&game->ChessSet, game->WhosTurn, Rook, A1 + offset);
  ChessSetPlacePiece(&game->ChessSet, game->WhosTurn, Rook, D1 + offset);
  ChessSetRemovePiece(&game->ChessSet, game->WhosTurn, King, E1 + offset);
  ChessSetPlacePiece(&game->ChessSet, game->WhosTurn, King, C1 + offset);
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

    offset = -1 + 2*game->WhosTurn;

    enPassant = POSITION(RANK(move->To)+offset, FILE(move->To));

    piece = PieceAt(&game->ChessSet.Sets[opposite], enPassant);
    if(piece == MissingPiece) {
      panic("No piece at %s when attempting en passant.", StringPosition(enPassant));
    } else {
      if(piece != Pawn) {
        panic("Piece taken via en passant is %s, not pawn.", StringPiece(piece));
      }
      ChessSetRemovePiece(&game->ChessSet, opposite, piece, enPassant);
      AppendPiece(&game->History.CapturedPieces, piece);
    }

    ChessSetRemovePiece(&game->ChessSet, game->WhosTurn, move->Piece, move->From);
    ChessSetPlacePiece(&game->ChessSet, game->WhosTurn, move->Piece, move->To);

    break;
  case PromoteKnight:
  case PromoteBishop:
  case PromoteRook:
  case PromoteQueen:
  case Normal:
    if(move->Capture) {
      piece = PieceAt(&game->ChessSet.Sets[opposite], move->To);
      if(piece == MissingPiece) {
        panic("No piece at %s when attempting capture %s.", StringPosition(move->To),
              StringMove(move));
      } else {
        ChessSetRemovePiece(&game->ChessSet, opposite, piece, move->To);
        AppendPiece(&game->History.CapturedPieces, piece);
      }
    }

    switch(move->Type) {
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
    case Normal:
      piece = move->Piece;
      break;
    default:
      panic("Impossible.");
    }

    ChessSetRemovePiece(&game->ChessSet, game->WhosTurn, move->Piece, move->From);
    ChessSetPlacePiece(&game->ChessSet, game->WhosTurn, piece, move->To);

    break;
  }

  AppendCastleEvent(&game->History.CastleEvents, updateCastlingRights(game, move));
  AppendMove(&game->History.Moves, *move);

  ToggleTurn(game);
}

// Determine whether the specified move places the current player into check.
bool
ExposesCheck(Game *game, BitBoard kingThreats, Move *move)
{
  bool checked, ret;
  BitBoard king;
  ChessSet clone;
  Piece piece;
  Side side = game->WhosTurn;

  switch(move->Type) {
  case CastleQueenSide:
  case CastleKingSide:
    // Castles are already checked for potential check scenarios.
    return false;
  case EnPassant:
    // TODO: Find a cleaner means of testing this for en passant.

    DoMove(game, move);
    ret = Checked(&game->ChessSet, OPPOSITE(game->WhosTurn));
    Unmove(game);
    return ret;
  case Normal:
  case PromoteKnight:
  case PromoteBishop:
  case PromoteRook:
  case PromoteQueen:
    break;
  }

  king = game->ChessSet.Sets[side].Boards[King];

  // Are we moving the king?
  if(move->Piece == King) {
    // Then it has to be to a square that is not attacked.
    return (POSBOARD(move->To) & kingThreats) != EmptyBoard;
  }

  checked = (kingThreats & king) != EmptyBoard;

  if(!checked) {
    // If not checked and piece being moved is not attacked, can't expose check.
    if((POSBOARD(move->From) & kingThreats) == EmptyBoard) {
      return false;
    }

    // Not checked and piece being moved *is* being attacked.

    // If piece can't possibly result in revealed check, then can't expose check.
    if(!CanSlideAttack[move->From][kingPos]) {
      return false;
    }

  } else {
    // Is checked and piece being moved is not a king.

    // If move does not block check, then it must leave the check in place.
    if(!move->Capture && !CanSlideAttack[move->To][BitScanForward(king)]) {
      return true;
    }
  }

  clone = game->ChessSet;

  ChessSetRemovePiece(&clone, side, move->Piece, move->From);
  if(move->Capture) {
    piece = PieceAt(&clone.Sets[OPPOSITE(side)], move->To);
    ChessSetRemovePiece(&clone, OPPOSITE(side), piece, move->To);
  }
  ChessSetPlacePiece(&clone, side, move->Piece, move->To);
  return Checked(&clone, side);
}

void
InitCanSlideAttacks()
{
  BitBoard queenThreats;
  Position from, to;

  for(from = A1; from <= H8; from++) {
    for(to = A1; to <= H8; to++) {
      CanSlideAttack[from][to] = false;
    }

    queenThreats = QueenThreats(POSBOARD(from), EmptyBoard);
    while(queenThreats) {
      to = PopForward(&queenThreats);
      CanSlideAttack[from][to] = true;
    }
  }
}

// Is the proposed move legal in this game?
bool
Legal(Game *game, Move *move)
{
  BitBoard kingThreats;
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
  piece = PieceAt(&game->ChessSet.Sets[game->WhosTurn], move->From);
  if(piece == MissingPiece || piece != move->Piece) {
    return false;
  }


  return (move->Piece == Pawn ? pawnLegal(game, move) : pieceLegal(move->Piece, game, move)) &&
    !ExposesCheck(game, kingThreats, move);
}

// Create a new game with an empty board.
Game
NewEmptyGame(bool debug, Side humanSide)
{
  Game ret;

  ret = NewGame(debug, humanSide);
  ret.CastleKingSideWhite = false;
  ret.CastleQueenSideWhite = false;
  ret.CastleKingSideBlack = false;
  ret.CastleQueenSideBlack = false;
  ret.ChessSet = NewEmptyChessSet();

  return ret;
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

// Determine whether the game is in a state of stalemate, i.e. the current player cannot make a
// move.
bool
Stalemated(Game *game)
{
  Move buffer[INIT_MOVE_LEN];
  MoveSlice slice = NewMoveSlice(buffer, INIT_MOVE_LEN);

  AllMoves(&slice, game);

  return slice.Len == 0 && !Checked(&game->ChessSet, game->WhosTurn);
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
          offset = -1 + game->WhosTurn*2;
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

static void
castleMoves(Game *game, MoveSlice *ret)
{
  Move kingSide, queenSide;

  kingSide.Type = CastleKingSide;
  queenSide.Type = CastleQueenSide;

  if(Legal(game, &kingSide)) {
    AppendMove(ret, kingSide);
  }

  if(Legal(game, &queenSide)) {
    AppendMove(ret, queenSide);
  }
}

void
pawnMoves(Game *game, BitBoard kingThreats, MoveSlice *slice)
{
  int k;
  BitBoard pushSources, captureSources, pushTargets, captureTargets, enPassants,
    fromBoard, toBoard;
  Move move;
  MoveType promotions[] = {PromoteKnight, PromoteBishop, PromoteRook, PromoteQueen};
  Position from, to;
  Rank promotionRank = game->WhosTurn == White ? Rank8 : Rank1;

  pushSources = AllPawnPushSources(&game->ChessSet, game->WhosTurn);
  captureSources = AllPawnCaptureSources(&game->ChessSet, game->WhosTurn);

  enPassants = game->ChessSet.Sets[game->WhosTurn].Boards[Pawn] &
    (game->WhosTurn == White ? Rank5Mask : Rank4Mask);

  // Pushes.
  while(pushSources) {
    from = PopForward(&pushSources);

    pushTargets = PawnPushTargets(&game->ChessSet, game->WhosTurn, POSBOARD(from));

    while(pushTargets) {
      to = PopForward(&pushTargets);

      move.Piece = Pawn;
      move.From = from;
      move.To = to;
      move.Capture = false;
      move.Type = Normal;

      if(RANK(to) == promotionRank) {
        for(k = 0; k < 4; k++) {
          move.Type = promotions[k];
            AppendMove(slice, move);
          }
        }
      } else if(!ExposesCheck(game, kingThreats, &move)) {
        AppendMove(slice, move);
      }
    }
  }

  // Captures.

  while(captureSources) {
    from = PopForward(&captureSources);

    captureTargets = PawnCaptureTargets(&game->ChessSet, game->WhosTurn, POSBOARD(from));

    while(captureTargets) {
      to = PopForward(&captureTargets);

      move.Piece = Pawn;
      move.From = from;
      move.To = to;
      move.Capture = true;
      move.Type = Normal;

      if(RANK(to) == promotionRank) {
        for(k = 0; k < 4; k++) {
          move.Type = promotions[k];
          if(!ExposesCheck(game, kingThreats, &move)) {
            AppendMove(slice, move);
          }
        }
      } else if(!ExposesCheck(game, kingThreats, &move)) {
        AppendMove(slice, move);
      }
    }
  }

  // En passant.

  while(enPassants) {
    from = PopForward(&enPassants);

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

    while(toBoard) {
      to = PopForward(&toBoard);

      move.Piece = Pawn;
      move.From = from;
      move.To = to;
      move.Capture = true;
      move.Type = EnPassant;
      if(Legal(game, &move)) {
        AppendMove(slice, move);
      }
    }
  }
}

void
pieceMoves(Piece piece, Game *game, BitBoard kingThreats, MoveSlice *ret)
{
  BitBoard captureTargets, moveTargets, pieceBoard;
  Move move;
  Position from, to;

  pieceBoard = game->ChessSet.Sets[game->WhosTurn].Boards[piece];

  move.Piece = piece;
  move.Type = Normal;

  while(pieceBoard) {
    from = PopForward(&pieceBoard);

    moveTargets = GetMoveTargets[piece](&game->ChessSet, game->WhosTurn, POSBOARD(from));
    captureTargets = GetCaptureTargets[piece](&game->ChessSet, game->WhosTurn, POSBOARD(from));

    move.From = from;    

    // Moves.
    move.Capture = false;        
    while(moveTargets) {
      to = PopForward(&moveTargets);
      move.To = to;

      if(!ExposesCheck(game, kingThreats, &move)) {
        AppendMove(ret, move);
      }
    }

    // Captures.
    move.Capture = true;    
    while(captureTargets) {
      to = PopForward(&captureTargets);

      move.To = to;
      if(!ExposesCheck(game, kingThreats, &move)) {
        AppendMove(ret, move);
      }
    }
  }
}

static bool
castleLegal(Game *game, bool queenSide)
{
  BitBoard attackMask, mask;
  Piece piece;

  if(Checked(&game->ChessSet, game->WhosTurn)) {
    return false;
  }

  switch(game->WhosTurn) {
  case White:
    piece = PieceAt(&game->ChessSet.Sets[game->WhosTurn], E1);
    if(piece != King) {
      return false;
    }

    break;
  case Black:
    piece = PieceAt(&game->ChessSet.Sets[game->WhosTurn], E8);
    if(piece != King) {
      return false;
    }

    break;
  default:
    panic("Unrecognised side %d.", game->WhosTurn);
  }

  if(game->WhosTurn == White) {
    if(queenSide) {
      if(!game->CastleQueenSideWhite) {
        return false;
      }
      piece = PieceAt(&game->ChessSet.Sets[game->WhosTurn], A1);
      if(piece != Rook) {
        return false;
      }

      mask = CastleQueenSideWhiteMask;
      attackMask = CastleQueenSideWhiteAttackMask;
    } else {
      if(!game->CastleKingSideWhite) {
        return false;
      }
      piece = PieceAt(&game->ChessSet.Sets[game->WhosTurn], H1);
      if(piece != Rook) {
        return false;
      }

      mask = CastleQueenSideWhiteMask;
      attackMask = mask;
    }
  } else {
    if(queenSide) {
      if(!game->CastleQueenSideBlack) {
        return false;
      }
      piece = PieceAt(&game->ChessSet.Sets[game->WhosTurn], A8);
      if(piece != Rook) {
        return false;
      }

      mask = CastleQueenSideBlackMask;
      attackMask = CastleQueenSideBlackAttackMask;
    } else {
      if(!game->CastleKingSideBlack) {
        return false;
      }
      piece = PieceAt(&game->ChessSet.Sets[game->WhosTurn], H8);
      if(piece != Rook) {
        return false;
      }

      mask = CastleKingSideBlackMask;
      attackMask = mask;
    }
  }

  // Obstructions clearly prevent castling.
  if((mask&game->ChessSet.Occupancy) != EmptyBoard) {
    return false;
  }

  // If any of the squares the king passes through are threatened by the opponent then
  // castling is prevented.
  if((attackMask&AllThreats(&game->ChessSet, OPPOSITE(game->WhosTurn))) != EmptyBoard) {
    return false;
  }

  return true;
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

static bool
pieceLegal(Piece piece, Game *game, Move *move)
{
  BitBoard from = POSBOARD(move->From), to = POSBOARD(move->To);

  if(move->Capture) {
    return (to&GetCaptureTargets[piece](&game->ChessSet, game->WhosTurn, from)) != EmptyBoard;
  }

  return (to&GetMoveTargets[piece](&game->ChessSet, game->WhosTurn, from)) != EmptyBoard;
}

static CastleEvent
updateCastlingRights(Game *game, Move *move)
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
