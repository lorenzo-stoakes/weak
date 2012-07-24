#include "weak.h"

static bool        castleLegal(Game*, CastleSide);
static void        initArrays(void);
static bool        pawnLegal(Game*, Move*);
static bool        pieceLegal(Piece, Game*, Move*);
static CastleEvent updateCastlingRights(Game*, Move*);

CheckStats
CalculateCheckStats(Game *game)
{
  BitBoard kingBoard;
  BitBoard occupancy = game->ChessSet.Occupancy;
  CheckStats ret;
  Position king;
  Side opposition = OPPOSITE(game->WhosTurn);

  kingBoard = game->ChessSet.Sets[opposition].Boards[King];

  king = BitScanForward(kingBoard);

  ret.AttackedKing = king;

  // Pieces *we* pin are potential discovered checks.
  ret.Discovered = PinnedPieces(&game->ChessSet, OPPOSITE(game->WhosTurn));
  ret.Pinned = PinnedPieces(&game->ChessSet, game->WhosTurn);

  // Attacks *from* king are equivalent to positions attacking *to* the king.
  ret.CheckSquares[Pawn] = PawnAttacksFrom(king, opposite);
  ret.CheckSquares[Knight] = KnightAttacksFrom(king);
  ret.CheckSquares[Bishop] = BishopAttacksFrom(king, occupancy);
  ret.CheckSquares[Rook] = RookAttacksFrom(king, occupancy);
  ret.CheckSquares[Queen] = ret.CheckSquares[Rook] | ret.CheckSquares[Bishop];

  return ret;
}

// Determine whether the current player is checkmated.
bool
Checkmated(Game *game)
{
  Move buffer[INIT_MOVE_LEN];
  MoveSlice slice = NewMoveSlice(buffer);

  // TODO: When *first* move returned, return false.

  AllMoves(&slice, game);

  return LenMoves(&slice) == 0 && Checked(&game->ChessSet, game->WhosTurn);
}

void
DoCastleKingSide(Game *game)
{
  int offset = game->WhosTurn*8*7;

  RemovePiece(&game->ChessSet, game->WhosTurn, King, E1 + offset);
  PlacePiece(&game->ChessSet, game->WhosTurn, King, G1 + offset);
  RemovePiece(&game->ChessSet, game->WhosTurn, Rook, H1 + offset);
  PlacePiece(&game->ChessSet, game->WhosTurn, Rook, F1 + offset);
}

void
DoCastleQueenSide(Game *game)
{
  int offset = game->WhosTurn*8*7;

  RemovePiece(&game->ChessSet, game->WhosTurn, King, E1 + offset);
  PlacePiece(&game->ChessSet, game->WhosTurn, King, C1 + offset);
  RemovePiece(&game->ChessSet, game->WhosTurn, Rook, A1 + offset);
  PlacePiece(&game->ChessSet, game->WhosTurn, Rook, D1 + offset);
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
      RemovePiece(&game->ChessSet, opposite, piece, enPassant);
      AppendPiece(&game->History.CapturedPieces, piece);
    }

    RemovePiece(&game->ChessSet, game->WhosTurn, move->Piece, move->From);
    PlacePiece(&game->ChessSet, game->WhosTurn, move->Piece, move->To);

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
        RemovePiece(&game->ChessSet, opposite, piece, move->To);
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

    RemovePiece(&game->ChessSet, game->WhosTurn, move->Piece, move->From);
    PlacePiece(&game->ChessSet, game->WhosTurn, piece, move->To);

    break;
  }

  castleEvent = updateCastlingRights(game, move);

  AppendCastleEvent(&game->History.CastleEvents, castleEvent);
  AppendCheckStats(&game->History.CheckStats, game->CheckStats);    
  AppendMove(&game->History.Moves, *move);

  game->CheckStats = CalculateCheckStats(game);

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

  RemovePiece(&clone, side, move->Piece, move->From);

  if(move->Type == EnPassant) {
    RemovePiece(&clone, OPPOSITE(side), Pawn,
                        move->To + (game->WhosTurn == White ? -8 : 8));
  } else if(move->Capture) {
    piece = PieceAt(&clone.Sets[OPPOSITE(side)], move->To);
    RemovePiece(&clone, OPPOSITE(side), piece, move->To);
  }

  PlacePiece(&clone, side, move->Piece, move->To);

  return Checked(&clone, side);
}

void
InitEngine()
{
  GetMoveTargets[Pawn] = NULL;
  GetMoveTargets[Knight] = &KnightMoveTargets;
  GetMoveTargets[Bishop] = &BishopMoveTargets;
  GetMoveTargets[Rook] = &RookMoveTargets;
  GetMoveTargets[Queen] = &QueenMoveTargets;
  GetMoveTargets[King] = &KingMoveTargets;

  GetCaptureTargets[Pawn] = NULL;
  GetCaptureTargets[Knight] = &KnightCaptureTargets;
  GetCaptureTargets[Bishop] = &BishopCaptureTargets;
  GetCaptureTargets[Rook] = &RookCaptureTargets;
  GetCaptureTargets[Queen] = &QueenCaptureTargets;
  GetCaptureTargets[King] = &KingCaptureTargets;

  InitKnight();
  InitRays();

  // Relies on above.
  InitMagics();
  initArrays();
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

CheckStats
NewCheckStats()
{
  CheckStats ret;
  Piece piece;

  ret.AttackedKing = EmptyPosition;  
  ret.Checks = EmptyBoard;
  for(piece = Pawn; piece < King; piece++) {
    ret.CheckSquares[piece] = EmptyBoard;
  }
  ret.Discovered = EmptyBoard;
  ret.Pinned = EmptyBoard;

  return ret;
}

// Create a new game with an empty board.
Game
NewEmptyGame(bool debug, Side humanSide)
{
  CastleSide castleSide;
  Game ret;
  Side side;

  ret = NewGame(debug, humanSide);

  ret.CheckStats.AttackedKing = EmptyPosition;

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

  ret.CheckStats = NewCheckStats();
  ret.CheckStats.AttackedKing = E1;
  ret.ChessSet = NewChessSet();
  ret.Debug = debug;
  ret.EnPassantSquare = EmptyPosition;
  ret.History = NewMoveHistory();
  ret.HumanSide = humanSide;
  ret.WhosTurn = White;

  return ret;
}

MoveHistory
NewMoveHistory()
{
  MoveHistory ret;

  Move *buffer;

  buffer = (Move*)allocate(sizeof(Move), INIT_MOVE_LEN);

  ret.CapturedPieces = NewPieceSlice();
  ret.CastleEvents = NewCastleEventSlice();
  ret.CheckStats = NewCheckStatsSlice();
  ret.EnPassantSquares = NewEnPassantSlice();
  ret.Moves = NewMoveSlice(buffer);

  return ret;
}

// Determine whether the game is in a state of stalemate, i.e. the current player cannot make a
// move.
bool
Stalemated(Game *game)
{
  Move buffer[INIT_MOVE_LEN];
  MoveSlice slice = NewMoveSlice(buffer);

  // TODO: When *first* move returned, return false.

  AllMoves(&slice, game);

  return LenMoves(&slice) == 0 && !Checked(&game->ChessSet, game->WhosTurn);
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
    RemovePiece(&game->ChessSet, game->WhosTurn, piece, move.To);
    PlacePiece(&game->ChessSet, game->WhosTurn, move.Piece, move.From);

    if(move.Capture) {
        captured = PopPiece(&game->History.CapturedPieces);

        opposite = OPPOSITE(game->WhosTurn);
        if(move.Type == EnPassant) {
          offset = -1 + game->WhosTurn*2;
          to = POSITION(RANK(move.To)+offset, FILE(move.To));
          PlacePiece(&game->ChessSet, opposite, captured, to);
        } else {
          PlacePiece(&game->ChessSet, opposite, captured, move.To);
        }
    }

    break;
  case CastleQueenSide:
    offset = game->WhosTurn == White ? 0 : 8*7;

    RemovePiece(&game->ChessSet, game->WhosTurn, King, C1+offset);
    PlacePiece(&game->ChessSet, game->WhosTurn, King, E1+offset);
    RemovePiece(&game->ChessSet, game->WhosTurn, Rook, D1+offset);
    PlacePiece(&game->ChessSet, game->WhosTurn, Rook, A1+offset);

    break;
  case CastleKingSide:
    offset = game->WhosTurn == White ? 0 : 8*7;

    RemovePiece(&game->ChessSet, game->WhosTurn, King, G1+offset);
    PlacePiece(&game->ChessSet, game->WhosTurn, King, E1+offset);
    RemovePiece(&game->ChessSet, game->WhosTurn, Rook, F1+offset);
    PlacePiece(&game->ChessSet, game->WhosTurn, Rook, H1+offset);

    break;
  default:
    panic("Unrecognised move type %d.", move.Type);
  }

  game->CheckStats = PopCheckStats(&game->History.CheckStats);
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

  if(!game->CastlingRights[game->WhosTurn][castleSide]) {
    return false;
  }

  if(game->CheckStats.CheckSources != EmptyBoard) {
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

static void
initArrays()
{
  BitBoard queenThreats;
  int delta;
  Position from, pos, to;

  for(from = A1; from <= H8; from++) {
    for(to = A1; to <= H8; to++) {
      Distance[from][to] = Max(FileDistance(from, to), RankDistance(from, to));
    }
  }

  for(from = A1; from <= H8; from++) {
    EmptyAttacks[Bishop][from] = BishopThreats(POSBOARD(from), EmptyBoard);
    EmptyAttacks[Rook][from] = RookThreats(POSBOARD(from), EmptyBoard);
    EmptyAttacks[Queen][from] = EmptyAttacks[Bishop][from] | EmptyAttacks[Rook][from];

    queenThreats = EmptyAttacks[Queen][from];

    for(to = A1; to <= H8; to++) {
      CanSlideAttack[from][to] = false;

      if((queenThreats&POSBOARD(to)) == EmptyBoard) {
        Between[from][to] = EmptyBoard;
      } else {
        // We determine step size in two parts - sign and amount. The sign is determined by
        // whether the target square (to) is larger than the source square(from) or not. We
        // get that from (to - from), and additionally the bit distance between the
        // positions. We then divide by the number of squares between the two and thus we get
        // the delta we need to apply.
        delta = (to - from) / Distance[from][to];
        for(pos = from + delta; pos < to; pos += delta) {
          Between[from][to] |= POSBOARD(pos);
        }
      }
    }

    while(queenThreats) {
      to = PopForward(&queenThreats);
      CanSlideAttack[from][to] = true;
    }
  }
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

  // TODO: Refactor.

  switch(move->Type) {
  default:
    return NoCastleEvent;
  case Normal:
    if(move->Capture) {
      switch(side) {
      case White:
        if(move->To == H8 && game->CastlingRights[Black][KingSide]) {
          game->CastlingRights[Black][KingSide] = false;
          ret |= LostKingSideBlack;
        } else if(move->To == A8 && game->CastlingRights[Black][QueenSide]) {
          game->CastlingRights[Black][QueenSide] = false;
          ret |= LostQueenSideBlack;
        }

        break;
      case Black:
        if(move->To == H1 && game->CastlingRights[White][KingSide]) {
          game->CastlingRights[White][KingSide] = false;
          ret |= LostKingSideWhite;
        } else if(move->To == A1 && game->CastlingRights[White][QueenSide]) {
          game->CastlingRights[White][QueenSide] = false;
          ret |= LostQueenSideWhite;
        }

        break;
      default:
        panic("Unrecognised side %d.", side);
      }
    } else if(move->Piece != King && move->Piece != Rook) {
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
