#include "weak.h"

static bool        castleLegal(Game*, CastleSide);
static void        initArrays(void);
static bool        pawnLegal(Game*, Move*);
static bool        pieceLegal(Piece, Game*, Move*);
static CastleEvent updateCastlingRights(Game*, Move*);

CheckStats
CalculateCheckStats(Game *game)
{
  BitBoard kingBoard, ourKingBoard;
  BitBoard occupancy = game->ChessSet.Occupancy;
  CheckStats ret;
  Position king, ourKing;
  Side side = game->WhosTurn;
  Side opposite = OPPOSITE(side);

  kingBoard = game->ChessSet.Sets[opposite].Boards[King];
  king = BitScanForward(kingBoard);

  ourKingBoard = game->ChessSet.Sets[side].Boards[King];
  ourKing = BitScanForward(ourKingBoard);

  ret.AttackedKing = king;
  ret.DefendedKing = ourKing;

  // Pieces *we* pin are potential discovered checks.
  ret.Discovered = PinnedPieces(&game->ChessSet, side, false);

  ret.Pinned = PinnedPieces(&game->ChessSet, side, true);

  // Attacks *from* king are equivalent to positions attacking *to* the king.
  ret.CheckSquares[Pawn] = PawnAttacksFrom(king, opposite);
  ret.CheckSquares[Knight] = KnightAttacksFrom(king);
  ret.CheckSquares[Bishop] = BishopAttacksFrom(king, occupancy);
  ret.CheckSquares[Rook] = RookAttacksFrom(king, occupancy);
  ret.CheckSquares[Queen] = ret.CheckSquares[Rook] | ret.CheckSquares[Bishop];
  // We never update king as clearly a king can't give check to another king. Leave it
  // as EmptyBoard.

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

  return LenMoves(&slice) == 0 && game->CheckStats.CheckSources != EmptyBoard;
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
  BitBoard checks;
  bool givesCheck;
  CastleEvent castleEvent;
  CheckStats checkStats;
  Piece piece;
  Position enPassant, king;
  Rank offset;
  Side side = game->WhosTurn;
  Side opposite = OPPOSITE(side);

  checkStats = game->CheckStats;
  givesCheck = GivesCheck(game, move);

  // Store previous en passant square.
  AppendEnpassantSquare(&game->History.EnPassantSquares, game->EnPassantSquare);

  // Assume empty, change if necessary.
  game->EnPassantSquare = EmptyPosition;

  switch(move->Type) {
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

    offset = -1 + 2*side;

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

    RemovePiece(&game->ChessSet, side, move->Piece, move->From);
    PlacePiece(&game->ChessSet, side, move->Piece, move->To);

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

      if(piece == Pawn && RANK(move->From) == Rank2 + (side*5) &&
         RANK(move->To) == Rank4 + (side*1)) {
        game->EnPassantSquare = move->From + (side == White ? 8 : -8);
      }

      break;
    default:
      panic("Impossible.");
    }

    RemovePiece(&game->ChessSet, side, move->Piece, move->From);
    PlacePiece(&game->ChessSet, side, piece, move->To);

    break;

  default:
    panic("Move type %d not recognised.", move->Type);    
  }

  castleEvent = updateCastlingRights(game, move);

  AppendCastleEvent(&game->History.CastleEvents, castleEvent);
  AppendCheckStats(&game->History.CheckStats, game->CheckStats);
  AppendMove(&game->History.Moves, *move);

  checks = EmptyBoard;
  if(givesCheck) {
    king = game->CheckStats.AttackedKing;

    switch(move->Type) {
    case Normal:
    case PromoteKnight:
    case PromoteBishop:
    case PromoteRook:
    case PromoteQueen:
      if((checkStats.CheckSquares[piece] & POSBOARD(move->To)) != EmptyBoard) {
        checks |= POSBOARD(move->To);
      }

      if(checkStats.Discovered != EmptyBoard &&
         (checkStats.Discovered & POSBOARD(move->From)) != EmptyBoard) {

        if(move->Piece != Rook) {
          checks |= RookAttacksFrom(king, game->ChessSet.Occupancy) &
            (game->ChessSet.Sets[side].Boards[Rook] |
             game->ChessSet.Sets[side].Boards[Queen]);
        }
        if(move->Piece != Bishop) {
          checks |= BishopAttacksFrom(king, game->ChessSet.Occupancy) &
            (game->ChessSet.Sets[side].Boards[Bishop] |
             game->ChessSet.Sets[side].Boards[Queen]);
        }
      }

      break;
    default:
      checks = AllAttackersTo(&game->ChessSet, king, game->ChessSet.Occupancy) &
        game->ChessSet.Sets[side].Occupancy;

      break;
    }
  }

  ToggleTurn(game);

  game->CheckStats = CalculateCheckStats(game);
  game->CheckStats.CheckSources = checks;
}

bool
GivesCheck(Game *game, Move *move)
{
  BitBoard bishopish, kingBoard, occNoFrom, rookish;
  BitBoard fromBoard = POSBOARD(move->From);
  BitBoard toBoard = POSBOARD(move->To);
  int offset;
  Position captureSquare, king, kingFrom, kingTo, rookFrom, rookTo;
  Side side;

  // Direct check.
  if((game->CheckStats.CheckSquares[move->Piece] & toBoard) != EmptyBoard) {
    return true;
  }

  // Discovered checks.
  if(game->CheckStats.Discovered != EmptyBoard &&
     (game->CheckStats.Discovered & fromBoard) != EmptyBoard) {
    switch(move->Piece) {
    case Pawn:
    case King:
      // If the piece blocking the discovered check is a rook, knight or bishop, then for it not to
      // be a direct check (considered above), it must be blocking an attack in the direction
      // it cannot move. Therefore any movement will 'discover' the check. Since a queen can
      // attack in all directions, it can't be involved.
      // A pawn or king, however, can move in the same direction as the attack the 'discoverer'
      // threatens, meaning our check is not revealed. So make sure from, to and the king do
      // not sit along the same line!

      if(Aligned(move->From, move->To, game->CheckStats.AttackedKing)) {
        break;
      }

      return true;
    default:
      return true;
    }
  }

  // If this is simply a normal move and we're here, then it's definitely not a checking move.
  if(move->Type == Normal) {
    return false;
  }

  // GivesCheck() is called before the move is executed, so these are valid.
  king = game->CheckStats.AttackedKing;
  kingBoard = POSBOARD(king);
  occNoFrom = game->ChessSet.Occupancy ^ POSBOARD(move->From);
  side = game->WhosTurn;

  switch(move->Type) {
  case PromoteKnight:
    return (KnightAttacksFrom(move->To) & kingBoard) != EmptyBoard;
  case PromoteRook:
    return (RookAttacksFrom(move->To, occNoFrom) & kingBoard) != EmptyBoard;
  case PromoteBishop:
    return (BishopAttacksFrom(move->To, occNoFrom) & kingBoard) != EmptyBoard;
  case PromoteQueen:
    return ((RookAttacksFrom(move->To, occNoFrom) | BishopAttacksFrom(move->To, occNoFrom)) &
            kingBoard) != EmptyBoard;
  case EnPassant:
    captureSquare = POSITION(RANK(move->From), FILE(move->To));
    occNoFrom ^= POSBOARD(captureSquare);
    occNoFrom |= POSBOARD(move->To);

    rookish = game->ChessSet.Sets[side].Boards[Rook] |
      game->ChessSet.Sets[side].Boards[Queen];

    bishopish = game->ChessSet.Sets[side].Boards[Bishop] |
      game->ChessSet.Sets[side].Boards[Queen];

    return ((RookAttacksFrom(king, occNoFrom)&rookish) |
            (BishopAttacksFrom(king, occNoFrom)&bishopish)) != EmptyBoard;
  case CastleQueenSide:
    offset = side*8*7;
    rookFrom = A1 + offset;
    rookTo = D1 + offset;
    kingFrom = E1 + offset;
    kingTo = C1 + offset;

    occNoFrom = game->ChessSet.Occupancy ^ kingFrom ^ rookFrom | rookTo | kingTo;
    return (RookAttacksFrom(rookTo, occNoFrom) & kingBoard) != EmptyBoard;
  case CastleKingSide:
    offset = side*8*7;
    rookFrom = H1 + offset;
    rookTo = F1 + offset;
    kingFrom = E1 + offset;
    kingTo = G1 + offset;

    occNoFrom = game->ChessSet.Occupancy ^ kingFrom ^ rookFrom | rookTo | kingTo;
    return (RookAttacksFrom(rookTo, occNoFrom) & kingBoard) != EmptyBoard;
  default:
    panic("Invalid move type %d at this point.", move->Type);

  }

  return false;
}

void
InitEngine()
{
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

  return move->Piece == Pawn ? pawnLegal(game, move) : pieceLegal(move->Piece, game, move);
}

CheckStats
NewCheckStats()
{
  CheckStats ret;
  Piece piece;

  ret.AttackedKing = EmptyPosition;
  ret.DefendedKing = EmptyPosition;
  for(piece = Pawn; piece <= King; piece++) {
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
  ret.CheckStats.CheckSources = EmptyBoard;
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

  return LenMoves(&slice) == 0 && game->CheckStats.CheckSources != EmptyBoard;
}


bool
PseudoLegal(Game *game, Move *move)
{
  BitBoard bitBoard, opposition;
  Position king;
  Side opposite;

  if(move->Type == EnPassant) {
    opposite = OPPOSITE(game->WhosTurn);
    king = game->CheckStats.DefendedKing;

    // Occupancy after en passant.
    bitBoard = (game->ChessSet.Occupancy ^ POSBOARD(move->From) ^
                POSBOARD(move->To + 8*(1 - 2*opposite))) | POSBOARD(move->To);

    return (RookAttacksFrom(king, bitBoard) &
            (game->ChessSet.Sets[opposite].Boards[Queen] |
             game->ChessSet.Sets[opposite].Boards[Rook])) == EmptyBoard &&
      (BishopAttacksFrom(king, bitBoard) &
       (game->ChessSet.Sets[opposite].Boards[Queen] |
        game->ChessSet.Sets[opposite].Boards[Bishop])) == EmptyBoard;
  }

  if(move->Piece == King) {
    // Castles already checked.
    if(move->Type != Normal) {
      return true;
    }

    opposite = OPPOSITE(game->WhosTurn);
    opposition = game->ChessSet.Sets[opposite].Occupancy;

    return
      (AllAttackersTo(&game->ChessSet, move->To,
                      game->ChessSet.Occupancy) & opposition) == EmptyBoard;
  }

  // A non-king move is legal if its not pinned or is moving in the ray between it and the king.
  return game->CheckStats.Pinned == EmptyBoard ||
    (game->CheckStats.Pinned & POSBOARD(move->From)) == EmptyBoard ||
    Aligned(move->From, move->To, game->CheckStats.DefendedKing);
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
  Position pos, rook;
  Side opposite = OPPOSITE(game->WhosTurn);
  BitBoard opposition = game->ChessSet.Sets[opposite].Occupancy;

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

  // TODO: Reduce duplication between here and movegen.c.
  attackMask = CastlingAttackMasks[game->WhosTurn][castleSide];
  while(attackMask) {
    pos = PopForward(&attackMask);

    if((AllAttackersTo(&game->ChessSet, pos,
                       game->ChessSet.Occupancy) & opposition) != EmptyBoard) {
      return false;
    }
  }

  return true;
}

static void
initArrays()
{
  BitBoard queenThreats;
  int delta;
  int from, pos, to;

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

        for(pos = from + delta; pos != to; pos += delta) {
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
  case PromoteKnight:
  case PromoteBishop:
  case PromoteRook:
  case PromoteQueen:
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
