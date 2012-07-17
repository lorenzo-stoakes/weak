#include "weak.h"

static bool        castleLegal(Game*, CastleSide);
static bool        pawnLegal(Game*, Move*);
static bool        pieceLegal(Piece, Game*, Move*);
static CastleEvent updateCastlingRights(Game*, Move*);

// Determine whether the current player is checkmated.
bool
Checkmated(Game *game)
{
  Move buffer[INIT_MOVE_LEN];
  MoveSlice slice = NewMoveSlice(buffer, INIT_MOVE_LEN);

  // TODO: When *first* move returned, return false.

  AllMoves(&slice, game);

  return slice.Len == 0 && Checked(&game->ChessSet, game->WhosTurn);
}

void
DoCastleKingSide(Game *game)
{
  int offset = game->WhosTurn*8*7;

  ChessSetRemovePiece(&game->ChessSet, game->WhosTurn, King, E1 + offset);
  ChessSetPlacePiece(&game->ChessSet, game->WhosTurn, King, G1 + offset);
  ChessSetRemovePiece(&game->ChessSet, game->WhosTurn, Rook, H1 + offset);
  ChessSetPlacePiece(&game->ChessSet, game->WhosTurn, Rook, F1 + offset);
}

void
DoCastleQueenSide(Game *game)
{
  int offset = game->WhosTurn*8*7;

  ChessSetRemovePiece(&game->ChessSet, game->WhosTurn, King, E1 + offset);
  ChessSetPlacePiece(&game->ChessSet, game->WhosTurn, King, C1 + offset);
  ChessSetRemovePiece(&game->ChessSet, game->WhosTurn, Rook, A1 + offset);
  ChessSetPlacePiece(&game->ChessSet, game->WhosTurn, Rook, D1 + offset);
}

// Attempt to move piece.
void
DoMove(Game *game, Move *move)
{
  CastleEvent castleEvent;
  Piece piece;
  Position enPassant;
  Rank offset;
  Side opposite = OPPOSITE(game->WhosTurn);

  // Store previous en passant square.
  AppendEnpassantSquare(&game->History.EnPassantSquares, game->EnPassantSquare);

  // Assume empty, change if necessary.
  game->EnPassantSquare = EmptyPosition;

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

      if(piece == Pawn && RANK(move->From) == Rank2 + (game->WhosTurn*5) &&
         RANK(move->To) == Rank4 + (game->WhosTurn*1)) {
        game->EnPassantSquare = move->From + (game->WhosTurn == White ? 8 : -8);
      }

      break;
    default:
      panic("Impossible.");
    }

    ChessSetRemovePiece(&game->ChessSet, game->WhosTurn, move->Piece, move->From);
    ChessSetPlacePiece(&game->ChessSet, game->WhosTurn, piece, move->To);

    break;
  }

  castleEvent = updateCastlingRights(game, move);
  AppendCastleEvent(&game->History.CastleEvents, castleEvent);

  AppendMove(&game->History.Moves, *move);

  ToggleTurn(game);
}

// Determine whether the specified move places the current player into check.
bool
ExposesCheck(Game *game, BitBoard kingThreats, Move *move)
{
  bool checked;
  BitBoard king;
  ChessSet clone;
  Piece piece;
  Position kingPos;
  Side side = game->WhosTurn;

  switch(move->Type) {
  case CastleQueenSide:
  case CastleKingSide:
    // Castles are already checked for potential check scenarios.
    return false;
  case EnPassant:
    break;
  case Normal:
    // Are we moving the king?
    if(move->Piece == King) {
      // Then it has to be to a square that is not attacked.
      return (POSBOARD(move->To) & kingThreats) != EmptyBoard;
    }
    break;
  case PromoteKnight:
  case PromoteBishop:
  case PromoteRook:
  case PromoteQueen:
    break;
  }

  king = game->ChessSet.Sets[side].Boards[King];
  checked = (kingThreats & king) != EmptyBoard;

  if(!checked) {
    // If not checked and piece being moved is not attacked, can't expose check.
    if(move->Type != EnPassant && (POSBOARD(move->From) & kingThreats) == EmptyBoard) {
      return false;
    }

    // Not checked and non-king piece being moved *is* being attacked.

    kingPos = BitScanForward(king);
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

  if(move->Type == EnPassant) {
    ChessSetRemovePiece(&clone, OPPOSITE(side), Pawn,
                        move->To + (game->WhosTurn == White ? -8 : 8));
  } else if(move->Capture) {
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
    return castleLegal(game, QueenSide);
  case CastleKingSide:
    return castleLegal(game, KingSide);
  case EnPassant:
    if(move->Piece != Pawn || !move->Capture) {
      return false;
    }
  case PromoteKnight:
  case PromoteBishop:
  case PromoteRook:
  case PromoteQueen:
  case Normal:
    break;
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

  kingThreats = KingThreats(&game->ChessSet, OPPOSITE(game->WhosTurn));

  return (move->Piece == Pawn ? pawnLegal(game, move) : pieceLegal(move->Piece, game, move)) &&
    !ExposesCheck(game, kingThreats, move);
}

// Create a new game with an empty board.
Game
NewEmptyGame(bool debug, Side humanSide)
{
  CastleSide castleSide;
  Game ret;
  Side side;

  ret = NewGame(debug, humanSide);

  for(side = White; side <= Black; side++) {
    for(castleSide = KingSide; castleSide <= QueenSide; castleSide++) {
      ret.CastlingRights[side][castleSide] = false;
    }
  }

  ret.ChessSet = NewEmptyChessSet();

  return ret;
}

// Create new game with specified human side and white + black's pieces in standard initial
// positions.
Game
NewGame(bool debug, Side humanSide)
{
  CastleSide castleSide;
  Game ret;
  Side side;

  for(side = White; side <= Black; side++) {
    for(castleSide = KingSide; castleSide <= QueenSide; castleSide++) {
      ret.CastlingRights[side][castleSide] = true;
    }
  }

  ret.ChessSet = NewChessSet();
  ret.Debug = debug;
  ret.EnPassantSquare = EmptyPosition;
  ret.History = NewMoveHistory();
  ret.HumanSide = humanSide;
  ret.WhosTurn = White;

  return ret;
}

// Determine whether the game is in a state of stalemate, i.e. the current player cannot make a
// move.
bool
Stalemated(Game *game)
{
  Move buffer[INIT_MOVE_LEN];
  MoveSlice slice = NewMoveSlice(buffer, INIT_MOVE_LEN);

  // TODO: When *first* move returned, return false.

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
    break;
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
    offset = game->WhosTurn == White ? 0 : 8*7;

    ChessSetRemovePiece(&game->ChessSet, game->WhosTurn, King, C1+offset);
    ChessSetPlacePiece(&game->ChessSet, game->WhosTurn, King, E1+offset);
    ChessSetRemovePiece(&game->ChessSet, game->WhosTurn, Rook, D1+offset);
    ChessSetPlacePiece(&game->ChessSet, game->WhosTurn, Rook, A1+offset);

    break;
  case CastleKingSide:
    offset = game->WhosTurn == White ? 0 : 8*7;

    ChessSetRemovePiece(&game->ChessSet, game->WhosTurn, King, G1+offset);
    ChessSetPlacePiece(&game->ChessSet, game->WhosTurn, King, E1+offset);
    ChessSetRemovePiece(&game->ChessSet, game->WhosTurn, Rook, F1+offset);
    ChessSetPlacePiece(&game->ChessSet, game->WhosTurn, Rook, H1+offset);

    break;
  default:
    panic("Unrecognised move type %d.", move.Type);
  }

  game->EnPassantSquare = PopEnPassantSquare(&game->History.EnPassantSquares);

  castleEvent = PopCastleEvent(&game->History.CastleEvents);

  if(castleEvent == NoCastleEvent) {
    return;
  }

  if(castleEvent&LostKingSideWhite) {
    game->CastlingRights[White][KingSide] = true;
  }
  if(castleEvent&LostQueenSideWhite) {
    game->CastlingRights[White][QueenSide] = true;
  }

  if(castleEvent&LostKingSideBlack) {
    game->CastlingRights[Black][KingSide] = true;
  }
  if(castleEvent&LostQueenSideBlack) {
    game->CastlingRights[Black][QueenSide] = true;
  }
}

static bool
castleLegal(Game *game, CastleSide castleSide)
{
  BitBoard attackMask, mask;
  int offset;
  Position rook;

  if(Checked(&game->ChessSet, game->WhosTurn)) {
    return false;
  }
  if(!game->CastlingRights[game->WhosTurn][castleSide]) {
    return false;
  }

  offset = game->WhosTurn*7*8;

  if((game->ChessSet.Sets[game->WhosTurn].Boards[King]&POSBOARD(E1+offset)) == EmptyBoard) {
    return false;
  }

  rook = castleSide == QueenSide ? A1 : H1;
  rook += offset;

  if((game->ChessSet.Sets[game->WhosTurn].Boards[Rook]&POSBOARD(rook)) == EmptyBoard) {
    return false;
  }

  // Obstructions clearly prevent castling.
  mask = CastlingMasks[game->WhosTurn][castleSide];
  if((mask&game->ChessSet.Occupancy) != EmptyBoard) {
    return false;
  }

  // If any of the squares the king passes through are threatened by the opponent then
  // castling is prevented.
  attackMask = CastlingAttackMasks[game->WhosTurn][castleSide];
  if((attackMask&KingThreats(&game->ChessSet, OPPOSITE(game->WhosTurn))) != EmptyBoard) {
    return false;
  }

  return true;
}

// Is this pawn move legal?
static bool
pawnLegal(Game *game, Move *move)
{
  BitBoard sources, targets, validToRankMask;
  BitBoard fromBoard = POSBOARD(move->From), toBoard = POSBOARD(move->To);
  Rank expectedRank, rank;

  switch(move->Type) {
  default:
    panic("Move type %d in pawnLegal.", move->Type);
  case EnPassant:
    // En passants are captures. Full stop.
    if(!move->Capture) {
      return false;
    }

    // Has to be to the en passant square, of course!
    if(move->To != game->EnPassantSquare) {
      return false;
    }

    // From has to be from one of the two possible sources for an attack on the en passant
    // square.
    switch(game->WhosTurn) {
    case White:
      return ((SoWeOne(toBoard)|SoEaOne(toBoard))&fromBoard) != EmptyBoard;
    case Black:
      return ((NoWeOne(toBoard)|NoEaOne(toBoard))&fromBoard) != EmptyBoard;
    default:
      panic("Invalid side %d.", game->WhosTurn);
    }

    panic("Impossible.");
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
  int offset;
  CastleEvent ret = NoCastleEvent;
  Side side = game->WhosTurn;

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
    switch(side) {
    case White:
      if(game->CastlingRights[White][KingSide]) {
        game->CastlingRights[White][KingSide] = false;
        ret = LostKingSideWhite;
      }
      if(game->CastlingRights[White][QueenSide]) {
        game->CastlingRights[White][QueenSide] = false;
        ret |= LostQueenSideWhite;
      }

      return ret;
    case Black:
      if(game->CastlingRights[Black][KingSide]) {
        game->CastlingRights[Black][KingSide] = false;
        ret = LostKingSideBlack;
      }
      if(game->CastlingRights[Black][QueenSide]) {
        game->CastlingRights[Black][QueenSide] = false;
        ret |= LostQueenSideBlack;
      }

      return ret;
    default:
      panic("Unrecognised side %d.", game->WhosTurn);
    }
  }

  offset = game->WhosTurn*8*7;

  if(move->Piece == King && move->From == E1+offset) {
    if(game->CastlingRights[side][QueenSide]) {
      ret = game->WhosTurn == White ? LostQueenSideWhite : LostQueenSideBlack;
      game->CastlingRights[side][QueenSide] = false;
    }
    if(game->CastlingRights[side][KingSide]) {
      ret |= side == White ? LostKingSideWhite : LostKingSideBlack;
      game->CastlingRights[side][KingSide] = false;
    }
  } else if(game->CastlingRights[side][QueenSide] && move->Piece == Rook &&
            move->From == A1+offset) {
    ret = side == White ? LostQueenSideWhite : LostQueenSideBlack;
    game->CastlingRights[side][QueenSide] = false;
  } else if(game->CastlingRights[side][KingSide] && move->Piece == Rook &&
            move->From == H1+offset) {
    ret = side == White ? LostKingSideWhite : LostKingSideBlack;
    game->CastlingRights[side][KingSide] = false;
  }

  return ret;
}
