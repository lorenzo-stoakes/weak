#include "weak.h"

static void              initArrays(void);
static FORCE_INLINE void toggleTurn(Game *game);
static CastleEvent       updateCastlingRights(Game*, Piece, Move, bool);
static FORCE_INLINE void doCastleKingSide(Game *game);
static FORCE_INLINE void doCastleQueenSide(Game *game);

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
  ret.Discovered = PinnedPieces(&game->ChessSet, side, king,    false);
  ret.Pinned     = PinnedPieces(&game->ChessSet, side, ourKing, true);

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
  Move *start=buffer, *end;

  // TODO: When *first* move returned, return false.

  end = AllMoves(start, game);

  return LenMoves(start, end) == 0 && game->CheckStats.CheckSources != EmptyBoard;
}

static FORCE_INLINE void
doCastleKingSide(Game *game)
{
  ChessSet *chessSet = &game->ChessSet;
  int index;
  int offset = game->WhosTurn*8*7;
  Side side = game->WhosTurn;

  RemovePiece(chessSet, side, King, E1 + offset);
  PlacePiece(chessSet, side, King, G1 + offset);
  RemovePiece(chessSet, side, Rook, H1 + offset);
  PlacePiece(chessSet, side, Rook, F1 + offset);

  index = chessSet->PiecePositionIndexes[E1 + offset];

  assert(index >= 0);
  assert(index < MAX_PIECE_LOCATION);

  chessSet->PiecePositionIndexes[G1 + offset] = index;
  chessSet->PiecePositions[side][King][index] = G1 + offset;

  index = chessSet->PiecePositionIndexes[H1 + offset];

  assert(index >= 0);
  assert(index < MAX_PIECE_LOCATION);

  chessSet->PiecePositionIndexes[F1 + offset] = index;
  chessSet->PiecePositions[side][Rook][index] = F1 + offset;

  UpdateOccupancies(&game->ChessSet);
}

static FORCE_INLINE void
doCastleQueenSide(Game *game)
{
  ChessSet *chessSet = &game->ChessSet;
  int index;
  int offset = game->WhosTurn*8*7;
  Side side = game->WhosTurn;

  RemovePiece(chessSet, side, King, E1 + offset);
  PlacePiece(chessSet, side, King, C1 + offset);
  RemovePiece(chessSet, side, Rook, A1 + offset);
  PlacePiece(chessSet, side, Rook, D1 + offset);

  index = chessSet->PiecePositionIndexes[E1 + offset];

  assert(index >= 0);
  assert(index < MAX_PIECE_LOCATION);

  chessSet->PiecePositionIndexes[C1 + offset] = index;
  chessSet->PiecePositions[side][King][index] = C1 + offset;

  index = chessSet->PiecePositionIndexes[A1 + offset];

  assert(index >= 0);
  assert(index < MAX_PIECE_LOCATION);

  chessSet->PiecePositionIndexes[D1 + offset] = index;
  chessSet->PiecePositions[side][Rook][index] = D1 + offset;

  UpdateOccupancies(&game->ChessSet);
}

// Attempt to move piece.
void
DoMove(Game *game, Move move)
{
  BitBoard checks, mask;
  bool givesCheck;
  CastleEvent castleEvent;
  CheckStats checkStats;
  ChessSet *chessSet = &game->ChessSet;
  int indexCaptured, indexLast, indexTo;
  Piece placePiece, capturePiece;
  Position from = FROM(move), to = TO(move);
  Piece originalPiece;
  Piece piece = PieceAt(chessSet, from);
  Position enPassant, king, last;
  Rank offset;
  Side side = game->WhosTurn;
  Side opposite = OPPOSITE(side);

  assert(from >= A1);
  assert(from <= H8);

  assert(to >= A1);
  assert(to <= H8);

  assert(TYPE(move) == CastleKingSide || TYPE(move) == CastleQueenSide || piece >= Pawn);
  assert(TYPE(move) == CastleKingSide || TYPE(move) == CastleQueenSide || piece <= King);

  checkStats = game->CheckStats;
  givesCheck = GivesCheck(game, move);

  // Store previous en passant square.
  AppendEnpassantSquare(&game->History.EnPassantSquares, game->EnPassantSquare);

  // Assume empty, change if necessary.
  game->EnPassantSquare = EmptyPosition;

  switch(TYPE(move)) {
  case CastleKingSide:
    doCastleKingSide(game);
    AppendPiece(&game->History.CapturedPieces, MissingPiece);
    break;
  case CastleQueenSide:
    doCastleQueenSide(game);
    AppendPiece(&game->History.CapturedPieces, MissingPiece);
    break;
  case EnPassant:
    offset = -1 + 2*side;

    enPassant = POSITION(RANK(to)+offset, FILE(to));

    assert(PieceAt(chessSet, enPassant) == Pawn);

    RemovePiece(chessSet, opposite, Pawn, enPassant);
    AppendPiece(&game->History.CapturedPieces, Pawn);

    RemovePiece(chessSet, side, Pawn, from);
    PlacePiece (chessSet, side, Pawn, to);

    // Update occupancies.
    // TODO: Make quicker.
    UpdateOccupancies(chessSet);

    // TODO: Resolve the duplication with the capture code below.

    indexLast = --chessSet->PieceCounts[opposite][Pawn];

    assert(opposite <= Black);

    assert(indexLast >= 0);
    assert(indexLast < MAX_PIECE_LOCATION);

    last = chessSet->PiecePositions[opposite][Pawn][indexLast];
    indexCaptured = chessSet->PiecePositionIndexes[to];

    assert(indexCaptured >= 0);
    assert(indexCaptured < MAX_PIECE_LOCATION);

    chessSet->PiecePositionIndexes[last] = indexCaptured;
    chessSet->PiecePositions[opposite][Pawn][indexCaptured] = last;
    chessSet->PiecePositions[opposite][Pawn][indexLast] = EmptyPosition;

    break;
  case PromoteKnight:
  case PromoteBishop:
  case PromoteRook:
  case PromoteQueen:
  case Normal:
    capturePiece = PieceAt(chessSet, to);

    assert(capturePiece <= King);
    assert(opposite <= Black);

    if(capturePiece == MissingPiece) {
      AppendPiece(&game->History.CapturedPieces, MissingPiece);
    } else {
      // Capture.

      RemovePiece(chessSet, opposite, capturePiece, to);
      AppendPiece(&game->History.CapturedPieces, capturePiece);

      // Update occupancies.
      mask = POSBOARD(to);
      chessSet->Occupancy ^= mask;
      chessSet->Sets[opposite].Occupancy ^= mask;
      chessSet->Sets[opposite].EmptySquares = ~chessSet->Sets[opposite].Occupancy;
      chessSet->PieceOccupancy[capturePiece] ^= mask;

      // Swap the captured piece position and the last piece position.

      // Obtain the index of the position for the last entry in the list of positions for the
      // capture of this piece type, and simultaneously reduce the piece count for this piece
      // type.
      indexLast = --chessSet->PieceCounts[opposite][capturePiece];

      assert(opposite <= Black);

      assert(indexLast >= 0);
      assert(indexLast < MAX_PIECE_LOCATION);
      assert(capturePiece >= Pawn);
      assert(capturePiece <= King);

      // Now get the actual position for the last piece.
      last = chessSet->PiecePositions[opposite][capturePiece][indexLast];
      // Find out the index in our piece position list for the actually captured piece.
      indexCaptured = chessSet->PiecePositionIndexes[to];

      assert(indexCaptured >= 0);
      assert(indexCaptured < MAX_PIECE_LOCATION);

      // Update the index for our last piece to the index for the captured piece.
      chessSet->PiecePositionIndexes[last] = indexCaptured;
      // Now set this newly assigned index to the position of the last piece.
      chessSet->PiecePositions[opposite][capturePiece][indexCaptured] = last;
      // And set the last piece to EmptyPosition, so we can iterate through piece positions looking
      // for an EmptyPosition terminator.
      chessSet->PiecePositions[opposite][capturePiece][indexLast] = EmptyPosition;
    }

    switch(TYPE(move)) {
    case PromoteKnight:
      placePiece = Knight;
      break;
    case PromoteBishop:
      placePiece = Bishop;
      break;
    case PromoteRook:
      placePiece = Rook;
      break;
    case PromoteQueen:
      placePiece = Queen;
      break;
    case Normal:
      placePiece = piece;

      // Update en passant square.
      if(piece == Pawn && RANK(from) == Rank2 + (side*5) &&
         RANK(to) == Rank4 + (side*1)) {
        game->EnPassantSquare = from + (side == White ? 8 : -8);
      }

      break;
    default:
      panic("Impossible.");
    }

    RemovePiece(chessSet, side, piece, from);
    PlacePiece(chessSet, side, placePiece, to);

    // Update occupancies.
    mask = POSBOARD(from) | POSBOARD(to);

    chessSet->Occupancy ^= mask;
    chessSet->EmptySquares = ~chessSet->Occupancy;

    chessSet->Sets[side].Occupancy ^= mask;
    chessSet->Sets[side].EmptySquares = ~chessSet->Sets[side].Occupancy;

    chessSet->PieceOccupancy[piece] ^= POSBOARD(from);
    chessSet->PieceOccupancy[placePiece] ^= POSBOARD(to);

    indexTo = chessSet->PiecePositionIndexes[from];

    assert(indexTo >= 0);
    assert(indexTo < MAX_PIECE_LOCATION);

    chessSet->PiecePositionIndexes[to] = indexTo;
    chessSet->PiecePositions[side][piece][indexTo] = to;

    if(TYPE(move) >= PromoteKnight) {
      // Delete from pawn list by swapping with last and decrementing count (as with capture).
      indexLast = --chessSet->PieceCounts[side][Pawn];

      assert(side <= Black);

      assert(indexLast >= 0);
      assert(indexLast < MAX_PIECE_LOCATION);

      last = chessSet->PiecePositions[side][Pawn][indexLast];
      indexTo = chessSet->PiecePositionIndexes[to];

      assert(indexTo >= 0);
      assert(indexTo < MAX_PIECE_LOCATION);

      chessSet->PiecePositionIndexes[indexLast] = indexTo;
      chessSet->PiecePositions[side][Pawn][indexTo] = last;
      chessSet->PiecePositions[side][Pawn][indexLast] = EmptyPosition;

      // Now add the piece promoted to.
      indexTo = chessSet->PieceCounts[side][placePiece]++;

      assert(indexTo >= 0);
      assert(indexTo < MAX_PIECE_LOCATION);
      assert(placePiece >= Pawn);
      assert(placePiece <= King);

      chessSet->PiecePositions[side][placePiece][indexTo] = to;
    }

    break;

  default:
    panic("Move type %d not recognised.", TYPE(move));
  }

  castleEvent = updateCastlingRights(game, piece, move, piece != MissingPiece);

  AppendCastleEvent(&game->History.CastleEvents, castleEvent);
  AppendCheckStats(&game->History.CheckStats, game->CheckStats);
  AppendMove(&game->History.Moves, move);

  checks = EmptyBoard;
  if(givesCheck) {
    king = game->CheckStats.AttackedKing;

    originalPiece = piece;

    switch(TYPE(move)) {
    case PromoteKnight:
    case PromoteBishop:
    case PromoteRook:
    case PromoteQueen:
      piece = Knight + TYPE(move)-PromoteKnight;
      assert(piece >= Knight && piece <= Queen);
    case Normal:

      if((checkStats.CheckSquares[piece] & POSBOARD(to)) != EmptyBoard) {
        checks |= POSBOARD(to);
      }

      piece = originalPiece;

      if(checkStats.Discovered != EmptyBoard &&
         (checkStats.Discovered & POSBOARD(from)) != EmptyBoard) {
        if(piece != Rook) {
          checks |= RookAttacksFrom(king, chessSet->Occupancy) &
            (game->ChessSet.Sets[side].Boards[Rook] |
             game->ChessSet.Sets[side].Boards[Queen]);
        }
        if(piece != Bishop) {
          checks |= BishopAttacksFrom(king, chessSet->Occupancy) &
            (chessSet->Sets[side].Boards[Bishop] |
             chessSet->Sets[side].Boards[Queen]);
        }
      }

      break;
    default:
      checks = AllAttackersTo(chessSet, king, game->ChessSet.Occupancy) &
        chessSet->Sets[side].Occupancy;

      break;
    }
  }

  toggleTurn(game);

  game->CheckStats = CalculateCheckStats(game);
  game->CheckStats.CheckSources = checks;
}

bool
GivesCheck(Game *game, Move move)
{
  BitBoard bishopish, kingBoard, occNoFrom, rookish;
  BitBoard fromBoard = POSBOARD(FROM(move));
  BitBoard toBoard = POSBOARD(TO(move));
  int offset;
  Piece piece = PieceAt(&game->ChessSet, FROM(move));
  Position captureSquare, king, kingFrom, kingTo, rookFrom, rookTo;
  Side side;

  switch(TYPE(move)) {
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
    piece = Rook;
    break;
  default:
    break;
  }

  // Direct check.
  if((game->CheckStats.CheckSquares[piece] & toBoard) != EmptyBoard) {
    return true;
  }

  // Discovered checks.
  if(game->CheckStats.Discovered != EmptyBoard &&
     (game->CheckStats.Discovered & fromBoard) != EmptyBoard) {
    switch(piece) {
    case Pawn:
    case King:
      // If the piece blocking the discovered check is a rook, knight or bishop, then for it not to
      // be a direct check (considered above), it must be blocking an attack in the direction
      // it cannot move. Therefore any movement will 'discover' the check. Since a queen can
      // attack in all directions, it can't be involved.
      // A pawn or king, however, can move in the same direction as the attack the 'discoverer'
      // threatens, meaning our check is not revealed. So make sure from, to and the king do
      // not sit along the same line!

      if(Aligned(FROM(move), TO(move), game->CheckStats.AttackedKing)) {
        break;
      }

      return true;
    default:
      return true;
    }
  }

  // If this is simply a normal move and we're here, then it's definitely not a checking move.
  if(TYPE(move) == Normal) {
    return false;
  }

  // GivesCheck() is called before the move is executed, so these are valid.
  king = game->CheckStats.AttackedKing;
  kingBoard = POSBOARD(king);
  occNoFrom = game->ChessSet.Occupancy ^ POSBOARD(FROM(move));
  side = game->WhosTurn;

  switch(TYPE(move)) {
  case PromoteKnight:
    return (KnightAttacksFrom(TO(move)) & kingBoard) != EmptyBoard;
  case PromoteRook:
    return (RookAttacksFrom(TO(move), occNoFrom) & kingBoard) != EmptyBoard;
  case PromoteBishop:
    return (BishopAttacksFrom(TO(move), occNoFrom) & kingBoard) != EmptyBoard;
  case PromoteQueen:
    return ((RookAttacksFrom(TO(move), occNoFrom) | BishopAttacksFrom(TO(move), occNoFrom)) &
            kingBoard) != EmptyBoard;
  case EnPassant:
    captureSquare = POSITION(RANK(FROM(move)), FILE(TO(move)));
    occNoFrom ^= POSBOARD(captureSquare);
    occNoFrom |= POSBOARD(TO(move));

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

    occNoFrom = (game->ChessSet.Occupancy ^ kingFrom ^ rookFrom) | (rookTo | kingTo);
    return (RookAttacksFrom(rookTo, occNoFrom) & kingBoard) != EmptyBoard;
  case CastleKingSide:
    offset = side*8*7;
    rookFrom = H1 + offset;
    rookTo = F1 + offset;
    kingFrom = E1 + offset;
    kingTo = G1 + offset;

    occNoFrom = (game->ChessSet.Occupancy ^ kingFrom ^ rookFrom) | (rookTo | kingTo);
    return (RookAttacksFrom(rookTo, occNoFrom) & kingBoard) != EmptyBoard;
  default:
    panic("Invalid move type %d at this point.", TYPE(move));

  }

  return false;
}

void
InitEngine()
{
  InitKing();
  InitKnight();
  InitPawn();
  InitRays();

  // Relies on above.
  InitMagics();
  initArrays();
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
  Move *start=buffer, *end;

  // TODO: When *first* move returned, return false.

  end = AllMoves(start, game);

  return LenMoves(start, end) == 0;
}

bool
PseudoLegal(Game *game, Move move, BitBoard pinned)
{
  BitBoard bitBoard, opposition;
  Piece piece = PieceAt(&game->ChessSet, FROM(move));
  Position king;
  Side opposite;

  if(TYPE(move) == EnPassant) {
    opposite = OPPOSITE(game->WhosTurn);
    king = game->CheckStats.DefendedKing;

    // Occupancy after en passant.
    bitBoard = (game->ChessSet.Occupancy ^ POSBOARD(FROM(move)) ^
                POSBOARD(TO(move) + 8*(1 - 2*opposite))) | POSBOARD(TO(move));

    return (RookAttacksFrom(king, bitBoard) &
            (game->ChessSet.Sets[opposite].Boards[Queen] |
             game->ChessSet.Sets[opposite].Boards[Rook])) == EmptyBoard &&
      (BishopAttacksFrom(king, bitBoard) &
       (game->ChessSet.Sets[opposite].Boards[Queen] |
        game->ChessSet.Sets[opposite].Boards[Bishop])) == EmptyBoard;
  }

  if(piece == King) {
    // Castles already checked.
    if(TYPE(move) != Normal) {
      return true;
    }

    opposite = OPPOSITE(game->WhosTurn);
    opposition = game->ChessSet.Sets[opposite].Occupancy;

    return
      (AllAttackersTo(&game->ChessSet, TO(move),
                      game->ChessSet.Occupancy) & opposition) == EmptyBoard;
  }

  // A non-king move is legal if its not pinned or is moving in the ray between it and the king.

return pinned == EmptyBoard ||
    (pinned & POSBOARD(FROM(move))) == EmptyBoard ||
    Aligned(FROM(move), TO(move), game->CheckStats.DefendedKing);
}

// Toggle whose turn it is.
static FORCE_INLINE void
toggleTurn(Game *game) {
  game->WhosTurn = OPPOSITE(game->WhosTurn);
}

// Attempt to undo move.
void
Unmove(Game *game)
{
  BitBoard mask;
  CastleEvent castleEvent;
  ChessSet *chessSet = &game->ChessSet;
  int indexLast, indexTo;
  Move move;
  Piece captured, piece, removePiece;
  Position from, enPassantTo, last, to;
  Rank offset;
  Side opposite = game->WhosTurn, side;

  // Rollback to previous turn.
  move = PopMove(&game->History.Moves);
  from = FROM(move);
  to = TO(move);
  toggleTurn(game);
  side = game->WhosTurn;

  assert(side <= Black);
  assert(opposite <= Black);

  piece = PieceAt(chessSet, to);

  assert(TYPE(move) == CastleKingSide || TYPE(move) == CastleQueenSide || piece >= Pawn);
  assert(TYPE(move) == CastleKingSide || TYPE(move) == CastleQueenSide || piece <= King);

  captured = PopPiece(&game->History.CapturedPieces);

  switch(TYPE(move)) {
  case EnPassant:
    RemovePiece(chessSet, side, Pawn, to);
    PlacePiece(chessSet, side, Pawn, from);

    offset = -1 + side*2;
    enPassantTo = POSITION(RANK(to)+offset, FILE(to));

    assert(enPassantTo <= H8);

    PlacePiece(chessSet, opposite, Pawn, enPassantTo);

    indexLast = chessSet->PieceCounts[opposite][Pawn]++;
    chessSet->PiecePositionIndexes[enPassantTo] = indexLast;

    assert(indexLast >= 0);
    assert(indexLast < MAX_PIECE_LOCATION);

    chessSet->PiecePositions[opposite][Pawn][indexLast] = to;

    // TODO: Do faster. Do this for now.
    UpdateOccupancies(chessSet);

    break;
  case PromoteKnight:
  case PromoteBishop:
  case PromoteRook:
  case PromoteQueen:
  case Normal:
    indexTo = chessSet->PiecePositionIndexes[from];

    assert(indexTo >= 0);
    assert(indexTo < MAX_PIECE_LOCATION);

    chessSet->PiecePositionIndexes[to] = indexTo;
    chessSet->PiecePositions[side][piece][indexTo] = to;

    if(TYPE(move) >= PromoteKnight) {
      piece = Pawn;
      removePiece = Knight + TYPE(move) - PromoteKnight;

      indexLast = --chessSet->PieceCounts[side][removePiece];

      assert(side <= Black);

      assert(removePiece >= Pawn);
      assert(removePiece <= King);

      assert(indexLast >= 0);
      assert(indexLast < MAX_PIECE_LOCATION);

      last = chessSet->PiecePositions[side][removePiece][indexLast];
      indexTo = chessSet->PiecePositionIndexes[to];

      assert(indexTo >= 0);
      assert(indexTo < MAX_PIECE_LOCATION);

      chessSet->PiecePositionIndexes[last] = indexTo;
      chessSet->PiecePositions[side][removePiece][indexTo] = last;
      chessSet->PiecePositions[side][removePiece][indexLast] = EmptyPosition;

      indexTo = chessSet->PieceCounts[side][Pawn]++;

      assert(indexTo >= 0);
      assert(indexTo < MAX_PIECE_LOCATION);

      chessSet->PiecePositionIndexes[to] = indexTo;
      chessSet->PiecePositions[side][Pawn][indexTo] = to;
    } else {
      removePiece = piece;
    }

    RemovePiece(chessSet, side, removePiece, to);
    PlacePiece(chessSet, side, piece, from);

    // Update occupancies.
    mask = POSBOARD(from) | POSBOARD(to);

    chessSet->Occupancy ^= mask;
    chessSet->EmptySquares = ~chessSet->Occupancy;

    chessSet->Sets[side].Occupancy ^= mask;
    chessSet->Sets[side].EmptySquares = ~chessSet->Sets[side].Occupancy;

    chessSet->PieceOccupancy[piece] ^= POSBOARD(from);
    chessSet->PieceOccupancy[removePiece] ^= POSBOARD(to);

    if(captured != MissingPiece) {
      PlacePiece(chessSet, opposite, captured, to);

      // Update occupancies.
      mask = POSBOARD(to);
      chessSet->Occupancy ^= mask;
      chessSet->Sets[opposite].Occupancy ^= mask;
      chessSet->Sets[opposite].EmptySquares = ~chessSet->Sets[opposite].Occupancy;
      chessSet->PieceOccupancy[captured] ^= mask;

      indexTo = chessSet->PieceCounts[opposite][captured]++;

      assert(indexTo >= 0);
      assert(indexTo < MAX_PIECE_LOCATION);
      assert(captured >= Pawn);
      assert(captured <= King);
      assert(opposite <= Black);

      chessSet->PiecePositionIndexes[to] = indexTo;
      chessSet->PiecePositions[opposite][captured][indexTo] = to;
    }

    break;
  case CastleQueenSide:
    offset = side == White ? 0 : 8*7;

    RemovePiece(chessSet, side, King, C1+offset);
    PlacePiece(chessSet, side, King, E1+offset);
    RemovePiece(chessSet, side, Rook, D1+offset);
    PlacePiece(chessSet, side, Rook, A1+offset);

    indexTo = chessSet->PiecePositionIndexes[C1 + offset];

    assert(indexTo >= 0);
    assert(indexTo < MAX_PIECE_LOCATION);

    chessSet->PiecePositionIndexes[E1 + offset] = indexTo;
    chessSet->PiecePositions[side][King][indexTo] = E1 + offset;

    indexTo = chessSet->PiecePositionIndexes[D1 + offset];

    assert(indexTo >= 0);
    assert(indexTo < MAX_PIECE_LOCATION);

    chessSet->PiecePositionIndexes[A1 + offset] = indexTo;
    chessSet->PiecePositions[side][Rook][indexTo] = A1 + offset;

    // TODO: Do faster. Do this for now.
    UpdateOccupancies(chessSet);

    break;
  case CastleKingSide:
    offset = game->WhosTurn == White ? 0 : 8*7;

    RemovePiece(chessSet, side, King, G1+offset);
    PlacePiece(chessSet, side, King, E1+offset);
    RemovePiece(chessSet, side, Rook, F1+offset);
    PlacePiece(chessSet, side, Rook, H1+offset);

    indexTo = chessSet->PiecePositionIndexes[G1 + offset];

    assert(indexTo >= 0);
    assert(indexTo < MAX_PIECE_LOCATION);

    chessSet->PiecePositionIndexes[E1 + offset] = indexTo;
    chessSet->PiecePositions[side][King][indexTo] = E1 + offset;

    indexTo = chessSet->PiecePositionIndexes[F1 + offset];

    assert(indexTo >= 0);
    assert(indexTo < MAX_PIECE_LOCATION);

    chessSet->PiecePositionIndexes[H1 + offset] = indexTo;
    chessSet->PiecePositions[side][Rook][indexTo] = H1 + offset;

    // TODO: Do faster. Do this for now.
    UpdateOccupancies(chessSet);

    break;
  default:
    panic("Unrecognised move type %d.", TYPE(move));
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
    EmptyAttacks[Bishop][from] = BishopAttacksFrom(from, EmptyBoard);
    EmptyAttacks[Rook][from] = RookAttacksFrom(from, EmptyBoard);
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
          if(pos < 0 || pos > 63) {
            panic("Invalid pos.");
          }
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

static CastleEvent
updateCastlingRights(Game *game, Piece piece, Move move, bool capture)
{
  int offset;
  CastleEvent ret = NoCastleEvent;
  Side side = game->WhosTurn;
  Side opposite = OPPOSITE(side);

  if(TYPE(move) == CastleKingSide || TYPE(move) == CastleQueenSide) {
    if(game->CastlingRights[side][QueenSide]) {
      game->CastlingRights[side][QueenSide] = false;
      ret = side == White ? LostQueenSideWhite : LostQueenSideBlack;
    }

    if(game->CastlingRights[side][KingSide]) {
      game->CastlingRights[side][KingSide] = false;
      ret |= side == White ? LostKingSideWhite : LostKingSideBlack;
    }

    return ret;
  }

  if(capture) {
    if(TO(move) == A1+opposite*8*7 && game->CastlingRights[opposite][QueenSide]) {
      game->CastlingRights[opposite][QueenSide] = false;
      ret = opposite == White ? LostQueenSideWhite : LostQueenSideBlack;
    } else if(TO(move) == H1+opposite*8*7 && game->CastlingRights[opposite][KingSide]) {
      game->CastlingRights[opposite][KingSide] = false;
      ret = opposite == White ? LostKingSideWhite : LostKingSideBlack;
    }
  }

  if(piece != King && piece != Rook) {
    return ret;
  }

  offset = side*8*7;

  if(piece == King) {
    if(FROM(move) != E1+offset) {
      return ret;
    }

    if(game->CastlingRights[side][QueenSide]) {
      ret |= side == White ? LostQueenSideWhite : LostQueenSideBlack;
      game->CastlingRights[side][QueenSide] = false;
    }
    if(game->CastlingRights[side][KingSide]) {
      ret |= side == White ? LostKingSideWhite : LostKingSideBlack;
      game->CastlingRights[side][KingSide] = false;
    }
  } else {
    if(FROM(move) == A1+offset && game->CastlingRights[side][QueenSide]) {
      ret |= side == White ? LostQueenSideWhite : LostQueenSideBlack;
      game->CastlingRights[side][QueenSide] = false;
    }
    if(FROM(move) == H1+offset && game->CastlingRights[side][KingSide]) {
      ret |= side == White ? LostKingSideWhite : LostKingSideBlack;
      game->CastlingRights[side][KingSide] = false;
    }
  }

  return ret;
}
