/*
  Weak, a chess perft calculator derived from Stockfish.

  Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
  Copyright (C) 2008-2012 Marco Costalba, Joona Kiiski, Tord Romstad (Stockfish authors)
  Copyright (C) 2011-2012 Lorenzo Stoakes

  Weak is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Weak is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include "weak.h"
#include "magic.h"

static void              initArrays(void);
static FORCE_INLINE void toggleTurn(Game *game);
static CastleEvent       updateCastlingRights(Game*, Piece, Move, bool);
static FORCE_INLINE void doCastleKingSide(Game*);
static FORCE_INLINE void doCastleQueenSide(Game*);

#ifndef NDEBUG
static char*             checkConsistency(Game*, BitBoard, BitBoard);
#endif

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
  ret.CheckSquares[King] = EmptyBoard;

  return ret;
}

bool
Checked(Game *game)
{
  return game->CheckStats.CheckSources;
}

// Determine whether the current player is checkmated.
bool
Checkmated(Game *game)
{
  return Checked(game) && !AnyMoves(game);
}

// Attempt to move piece.
void
DoMove(Game *game, Move move)
{
#ifndef NDEBUG
  static BitBoard doMoveCount;
  char *msg;
#endif

  BitBoard checks, mask;
  bool givesCheck;
  CheckStats checkStats = game->CheckStats;
  ChessSet *chessSet = &game->ChessSet;
  int indexCaptured, indexLast, indexTo;
  Memory memory;
  MoveType type = TYPE(move);
  Piece capturePiece = MissingPiece;
  Position enPassantedPawn, king, last;
  Position from = FROM(move), to = TO(move);
  Piece originalPiece;
  Piece piece = PieceAt(chessSet, from);
  Piece placePiece = piece; // Default to piece unless we know better.
  Rank offset;
  Side side = game->WhosTurn;
  Side opposite = OPPOSITE(side);
#ifndef NDEBUG
  BitBoard modelChecks = EmptyBoard;

  doMoveCount++;
#endif

  // Default to no capture.
  memory.Captured = MissingPiece;

  givesCheck = GivesCheck(game, move);

  // If we did just have an en passant square and are about to invalidate it,
  // then update the hash accordingly.
  if(game->EnPassantSquare != EmptyPosition) {
    game->Hash ^= ZobristEnPassantFileHash[FILE(game->EnPassantSquare)];
  }

  // Store previous en passant square.
  memory.EnPassantSquare = game->EnPassantSquare;

  // Assume empty, change if necessary.
  game->EnPassantSquare = EmptyPosition;

  if(type == CastleKingSide) {
    doCastleKingSide(game);
  } else if(type == CastleQueenSide) {
    doCastleQueenSide(game);
  } else if(type == EnPassant) {
    offset = -1 + 2*side;

    enPassantedPawn = POSITION(RANK(to)+offset, FILE(to));

    RemovePiece(chessSet, opposite, Pawn, enPassantedPawn);
    game->Hash ^= ZobristPositionHash[opposite][Pawn][enPassantedPawn];

    memory.Captured = Pawn;

    RemovePiece(chessSet, side, Pawn, from);
    game->Hash ^= ZobristPositionHash[side][Pawn][from];

    PlacePiece (chessSet, side, Pawn, to);
    game->Hash ^= ZobristPositionHash[side][Pawn][to];

    // TODO: Make quicker.
    UpdateOccupancies(chessSet);

    // TODO: Resolve the duplication with the capture code below.

    indexLast = --chessSet->PieceCounts[opposite][Pawn];

    last = chessSet->PiecePositions[opposite][Pawn][indexLast];
    indexCaptured = chessSet->PiecePositionIndexes[enPassantedPawn];

    chessSet->PiecePositionIndexes[last] = indexCaptured;
    chessSet->PiecePositions[opposite][Pawn][indexCaptured] = last;
    chessSet->PiecePositions[opposite][Pawn][indexLast] = EmptyPosition;

    indexTo = chessSet->PiecePositionIndexes[from];
    chessSet->PiecePositionIndexes[to] = indexTo;
    chessSet->PiecePositions[side][Pawn][indexTo] = to;
  } else {
    capturePiece = PieceAt(chessSet, to);

    if(capturePiece != MissingPiece) {
      // Capture.

      memory.Captured = capturePiece;

      RemovePiece(chessSet, opposite, capturePiece, to);
      game->Hash ^= ZobristPositionHash[opposite][capturePiece][to];

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

      // Now get the actual position for the last piece.
      last = chessSet->PiecePositions[opposite][capturePiece][indexLast];
      // Find out the index in our piece position list for the actually captured piece.
      indexCaptured = chessSet->PiecePositionIndexes[to];

      // Update the index for our last piece to the index for the captured piece.
      chessSet->PiecePositionIndexes[last] = indexCaptured;
      // Now set this newly assigned index to the position of the last piece.
      chessSet->PiecePositions[opposite][capturePiece][indexCaptured] = last;
      // And set the last piece to EmptyPosition, so we can iterate through piece positions looking
      // for an EmptyPosition terminator.
      chessSet->PiecePositions[opposite][capturePiece][indexLast] = EmptyPosition;
    }

    indexTo = chessSet->PiecePositionIndexes[from];
    chessSet->PiecePositionIndexes[to] = indexTo;
    chessSet->PiecePositions[side][piece][indexTo] = to;

    if(type&PromoteMask) {
      placePiece = type - PromoteMask;

      // Delete from pawn list by swapping with last and decrementing count (as with capture).
      indexLast = --chessSet->PieceCounts[side][Pawn];

      last = chessSet->PiecePositions[side][Pawn][indexLast];
      indexTo = chessSet->PiecePositionIndexes[to];

      chessSet->PiecePositionIndexes[last] = indexTo;
      chessSet->PiecePositions[side][Pawn][indexTo] = last;
      chessSet->PiecePositions[side][Pawn][indexLast] = EmptyPosition;

      // Now add the piece promoted to.
      indexTo = chessSet->PieceCounts[side][placePiece]++;

      chessSet->PiecePositionIndexes[to] = indexTo;
      chessSet->PiecePositions[side][placePiece][indexTo] = to;
    } else {
      placePiece = piece;
      // Update en passant square.
      if(piece == Pawn && RANK(from) == Rank2 + (side*5) &&
         RANK(to) == Rank4 + (side*1)) {
        game->EnPassantSquare = from + (side == White ? 8 : -8);
        game->Hash ^= ZobristEnPassantFileHash[FILE(game->EnPassantSquare)];
      }
    }

    game->Hash ^= ZobristPositionHash[side][piece][from];
    RemovePiece(chessSet, side, piece, from);

    game->Hash ^= ZobristPositionHash[side][placePiece][to];
    PlacePiece(chessSet, side, placePiece, to);

    // Update occupancies.
    mask = POSBOARD(from) | POSBOARD(to);

    chessSet->Occupancy ^= mask;
    chessSet->EmptySquares = ~chessSet->Occupancy;

    chessSet->Sets[side].Occupancy ^= mask;
    chessSet->Sets[side].EmptySquares = ~chessSet->Sets[side].Occupancy;

    chessSet->PieceOccupancy[piece] ^= POSBOARD(from);
    chessSet->PieceOccupancy[placePiece] ^= POSBOARD(to);
  }

  memory.CastleEvent = updateCastlingRights(game, piece, move, piece != MissingPiece);
  memory.CheckStats = game->CheckStats;
  memory.Move = move;

  AppendMemory(&game->Memories, memory);

  checks = EmptyBoard;
  if(givesCheck) {
    king = game->CheckStats.AttackedKing;

    // TODO: Examine whether we can't use our 'fast' approach for these cases too.
    if(type == EnPassant || type&CastleMask || type&PromoteMask) {
      checks = AllAttackersTo(chessSet, king, game->ChessSet.Occupancy) &
        chessSet->Sets[side].Occupancy;
    } else {
      originalPiece = piece;
      piece = placePiece;

      if((checkStats.CheckSquares[piece]&POSBOARD(to))) {
        checks |= POSBOARD(to);
      }

      piece = originalPiece;

      if(checkStats.Discovered &&
         (checkStats.Discovered&POSBOARD(from))) {
        if(piece != Rook) {
          checks |= RookAttacksFrom(king, chessSet->Occupancy) &
            (chessSet->Sets[side].Boards[Rook] |
             chessSet->Sets[side].Boards[Queen]);
        }
        if(piece != Bishop) {
          checks |= BishopAttacksFrom(king, chessSet->Occupancy) &
            (chessSet->Sets[side].Boards[Bishop] |
             chessSet->Sets[side].Boards[Queen]);
        }
      }
    }

#ifndef NDEBUG
    modelChecks = AllAttackersTo(chessSet, king, game->ChessSet.Occupancy) &
        chessSet->Sets[side].Occupancy;
#endif
  }

#ifndef NDEBUG
  if((msg = checkConsistency(game, checks, modelChecks)) != NULL) {
    printf("Inconsistency in %s's DoMove of %s at doMoveCount %lu:-\n\n",
           StringSide(side),
           StringMoveFull(move, piece, capturePiece != MissingPiece),
           doMoveCount);
    puts(StringChessSet(chessSet));
    puts(msg);
    abort();
  }
#endif

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
  Piece piece;
  Position captureSquare, king, kingFrom, kingTo, rookFrom, rookTo;
  Side side;
  MoveType type = TYPE(move);

  if(type&PromoteMask) {
    piece = type - PromoteMask;
  } else {
    piece = PieceAt(&game->ChessSet, FROM(move));
  }

  // Direct check.
  if(game->CheckStats.CheckSquares[piece]&toBoard) {
    return true;
  }

  // Discovered checks.
  if(game->CheckStats.Discovered && (game->CheckStats.Discovered&fromBoard)) {
    switch(piece) {
    case Pawn:
    case King:
      // If the piece blocking the discovered check is a rook, knight or bishop, then for it
      // not to be a direct check (considered above), it must be blocking an attack in the
      // direction it cannot move. Therefore any movement will 'discover' the check. Since a
      // queen can attack in all directions, it can't be involved.

      // A pawn or king, however, can move in the same direction as the attack the 'discoverer'
      // threatens, meaning our check is not revealed. So make sure from, to and the king do
      // not sit along the same line!

      if(Aligned(FROM(move), TO(move), game->CheckStats.AttackedKing)) {
        return false;
      }

      return true;
    default:
      return true;
    }
  }

  // If this is simply a normal move and we're here, then it's definitely not a checking move.
  if(type == Normal) {
    return false;
  }

  // GivesCheck() is called before the move is executed, so these are valid.
  king = game->CheckStats.AttackedKing;
  kingBoard = POSBOARD(king);
  occNoFrom = game->ChessSet.Occupancy^fromBoard;
  side = game->WhosTurn;

  switch(type) {
  case PromoteKnight:
    return KnightAttacksFrom(TO(move))&kingBoard;
  case PromoteRook:
    return RookAttacksFrom(TO(move), occNoFrom)&kingBoard;
  case PromoteBishop:
    return BishopAttacksFrom(TO(move), occNoFrom)&kingBoard;
  case PromoteQueen:
    return (RookAttacksFrom(TO(move), occNoFrom)|BishopAttacksFrom(TO(move), occNoFrom))&
            kingBoard;
  case EnPassant:
    // We want to consider attacks on the king after we have removed the captured piece.
    captureSquare = POSITION(RANK(FROM(move)), FILE(TO(move)));
    occNoFrom ^= POSBOARD(captureSquare);

    // Add position where pawn is placed in occupancy. Use |= as this could be a capture.
    occNoFrom |= toBoard;

    // Now consider pieces that could have a revealed check - queen, rook, bishop.

    rookish = game->ChessSet.Sets[side].Boards[Rook] |
      game->ChessSet.Sets[side].Boards[Queen];

    bishopish = game->ChessSet.Sets[side].Boards[Bishop] |
      game->ChessSet.Sets[side].Boards[Queen];

    // Again, attacks *from* a square are equivalent to attacks to that square by the piece
    // in question.
    return (RookAttacksFrom(king, occNoFrom)&rookish) |
      (BishopAttacksFrom(king, occNoFrom)&bishopish);
  case CastleQueenSide:
    offset = side*8*7;
    rookFrom = A1 + offset;
    rookTo = D1 + offset;
    kingFrom = E1 + offset;
    kingTo = C1 + offset;

    occNoFrom = (game->ChessSet.Occupancy^kingFrom^rookFrom)|(rookTo|kingTo);
    return RookAttacksFrom(rookTo, occNoFrom)&kingBoard;
  case CastleKingSide:
    offset = side*8*7;
    rookFrom = H1 + offset;
    rookTo = F1 + offset;
    kingFrom = E1 + offset;
    kingTo = G1 + offset;

    occNoFrom = (game->ChessSet.Occupancy^kingFrom^rookFrom)|(rookTo|kingTo);
    return RookAttacksFrom(rookTo, occNoFrom)&kingBoard;
  default:
    panic("Invalid move type %d at this point.", type);
  }

  panic("Unreachable.");
  return false;
}

void
InitEngine()
{
  InitTrans();    
  InitZobrist();
  InitKing();
  InitKnight();
  InitPawn();
  InitRays();

  // Relies on above.
  InitMagics();
  initArrays();
}

bool
Legal(Game *game, Move move)
{
  ChessSet *chessSet = &game->ChessSet;
  Move    currMove;
  Move     *curr, *end;
  Move     moves[INIT_MOVE_LEN];
  MoveType currType;
  MoveType type = TYPE(move);
  Rank     enpassantRank, promoteRank;
  Piece    piece;
  Position from = FROM(move);
  Position to   = TO(move);
  Side     side = game->WhosTurn;

  if(from == to) {
    return false;
  }

  piece = PieceAt(chessSet, from);

  if(piece == MissingPiece) {
    return false;
  }

  if(type&PromoteMask) {
    promoteRank = side == White ? Rank7 : Rank2;
    if(piece != Pawn || RANK(from) != promoteRank) {
      return false;
    }
  }

  if(type == EnPassant) {
    enpassantRank = side == White ? Rank5 : Rank4;
    if(piece != Pawn || RANK(from) != enpassantRank || PieceAt(chessSet, to) != Pawn) {
      return false;
    }
  }

  // We don't need to be particularly efficient here, as this is only called when the
  // user has input a move.
  end = AllMoves(moves, game);

  for(curr = moves; curr != end; curr++) {
    currMove = *curr;
    currType = TYPE(currMove);

    if(currMove == move || (((type&CastleMask) && !(type&PromoteMask)) && type == currType)) {
      return true;
    }
  }

  return false;
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

  ret.CheckStats.DefendedKing = EmptyPosition;
  ret.CheckStats.AttackedKing = EmptyPosition;

  for(side = White; side <= Black; side++) {
    for(castleSide = KingSide; castleSide <= QueenSide; castleSide++) {
      ret.CastlingRights[side][castleSide] = false;
    }
  }

  ret.ChessSet = NewEmptyChessSet();

  ret.Hash = HashGame(&ret);

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

  ret.ChessSet = NewChessSet();

  for(side = White; side <= Black; side++) {
    for(castleSide = KingSide; castleSide <= QueenSide; castleSide++) {
      ret.CastlingRights[side][castleSide] = true;
    }
  }

  ret.CheckStats = NewCheckStats();
  ret.CheckStats.CheckSources = EmptyBoard;
  ret.CheckStats.DefendedKing = E1;
  ret.CheckStats.AttackedKing = E8;

  ret.Debug = debug;
  ret.EnPassantSquare = EmptyPosition;
  ret.Memories = NewMemorySlice();
  ret.HumanSide = humanSide;
  ret.WhosTurn = White;

  ret.Hash = HashGame(&ret);

  return ret;
}

// Determine whether the game is in a state of stalemate, i.e. the current player cannot make a
// move.
bool
Stalemated(Game *game)
{
  return !AnyMoves(game);
}

// Check legality of pseudo legal moves.
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

    return !(RookAttacksFrom(king, bitBoard) &
            (game->ChessSet.Sets[opposite].Boards[Queen] |
             game->ChessSet.Sets[opposite].Boards[Rook])) &&
      !(BishopAttacksFrom(king, bitBoard) &
        (game->ChessSet.Sets[opposite].Boards[Queen] |
         game->ChessSet.Sets[opposite].Boards[Bishop]));
  }

  if(piece == King) {
    // Castles already checked.
    if(TYPE(move) != Normal) {
      return true;
    }

    opposite = OPPOSITE(game->WhosTurn);
    opposition = game->ChessSet.Sets[opposite].Occupancy;

    return
      !(AllAttackersTo(&game->ChessSet, TO(move),
                      game->ChessSet.Occupancy)&opposition);
  }

  // A non-king move is legal if its not pinned or is moving in the ray between it and the king.

return !pinned ||
    !(pinned&POSBOARD(FROM(move))) ||
    Aligned(FROM(move), TO(move), game->CheckStats.DefendedKing);
}

// Attempt to undo move.
void
Unmove(Game *game)
{
#ifndef NDEBUG
  char *msg;
  static BitBoard unmoveCount;
#endif

  BitBoard mask;
  CastleEvent castleEvent;
  ChessSet *chessSet = &game->ChessSet;
  int indexLast, indexTo;
  Memory memory;
  Move move;
  Piece capturePiece, piece, removePiece;
  Position from, enPassantedPawn, last, to;
  Rank offset;
  Side opposite = game->WhosTurn, side;

#ifndef NDEBUG
  unmoveCount++;
#endif

  memory = PopMemory(&game->Memories);
  move = memory.Move;
  capturePiece = memory.Captured;

  // Rollback to previous turn.
  from = FROM(move);
  to = TO(move);
  toggleTurn(game);
  side = game->WhosTurn;

  piece = PieceAt(chessSet, to);

  switch(TYPE(move)) {
  case EnPassant:
    RemovePiece(chessSet, side, Pawn, to);
    game->Hash ^= ZobristPositionHash[side][Pawn][to];

    PlacePiece(chessSet, side, Pawn, from);
    game->Hash ^= ZobristPositionHash[side][Pawn][from];

    offset = -1 + side*2;
    enPassantedPawn = POSITION(RANK(to)+offset, FILE(to));

    PlacePiece(chessSet, opposite, Pawn, enPassantedPawn);
    game->Hash ^= ZobristPositionHash[opposite][Pawn][enPassantedPawn];

    indexLast = chessSet->PieceCounts[opposite][Pawn]++;
    chessSet->PiecePositionIndexes[enPassantedPawn] = indexLast;
    chessSet->PiecePositions[opposite][Pawn][indexLast] = enPassantedPawn;

    indexTo = chessSet->PiecePositionIndexes[to];
    chessSet->PiecePositionIndexes[from] = indexTo;
    chessSet->PiecePositions[side][piece][indexTo] = from;

    // TODO: Do faster. Do this for now.
    UpdateOccupancies(chessSet);

    break;
  case PromoteKnight:
  case PromoteBishop:
  case PromoteRook:
  case PromoteQueen:
  case Normal:
    if(TYPE(move) >= PromoteKnight) {
      piece = Pawn;
      removePiece = Knight + TYPE(move) - PromoteKnight;

      indexLast = --chessSet->PieceCounts[side][removePiece];

      last = chessSet->PiecePositions[side][removePiece][indexLast];
      indexTo = chessSet->PiecePositionIndexes[to];

      chessSet->PiecePositionIndexes[last] = indexTo;
      chessSet->PiecePositions[side][removePiece][indexTo] = last;
      chessSet->PiecePositions[side][removePiece][indexLast] = EmptyPosition;

      indexTo = chessSet->PieceCounts[side][Pawn]++;

      chessSet->PiecePositionIndexes[to] = indexTo;
      chessSet->PiecePositions[side][Pawn][indexTo] = to;
    } else {
      removePiece = piece;
    }

    RemovePiece(chessSet, side, removePiece, to);
    game->Hash ^= ZobristPositionHash[side][removePiece][to];

    PlacePiece(chessSet, side, piece, from);
    game->Hash ^= ZobristPositionHash[side][piece][from];

    indexTo = chessSet->PiecePositionIndexes[to];
    chessSet->PiecePositionIndexes[from] = indexTo;
    chessSet->PiecePositions[side][piece][indexTo] = from;

    // Update occupancies.
    mask = POSBOARD(from)|POSBOARD(to);

    chessSet->Occupancy ^= mask;
    chessSet->EmptySquares = ~chessSet->Occupancy;

    chessSet->Sets[side].Occupancy ^= mask;
    chessSet->Sets[side].EmptySquares = ~chessSet->Sets[side].Occupancy;

    chessSet->PieceOccupancy[piece] ^= POSBOARD(from);
    chessSet->PieceOccupancy[removePiece] ^= POSBOARD(to);

    if(capturePiece != MissingPiece) {
      PlacePiece(chessSet, opposite, capturePiece, to);
      game->Hash ^= ZobristPositionHash[opposite][capturePiece][to];

      // Update occupancies.
      mask = POSBOARD(to);
      chessSet->Occupancy ^= mask;
      chessSet->EmptySquares = ~chessSet->Occupancy;
      chessSet->Sets[opposite].Occupancy ^= mask;
      chessSet->Sets[opposite].EmptySquares = ~chessSet->Sets[opposite].Occupancy;
      chessSet->PieceOccupancy[capturePiece] ^= mask;

      indexTo = chessSet->PieceCounts[opposite][capturePiece]++;

      chessSet->PiecePositionIndexes[to] = indexTo;
      chessSet->PiecePositions[opposite][capturePiece][indexTo] = to;
    }

    break;
  case CastleQueenSide:
    offset = side == White ? 0 : 8*7;

    RemovePiece(chessSet, side, King, C1+offset);
    game->Hash ^= ZobristPositionHash[side][King][C1+offset];

    PlacePiece(chessSet, side, King, E1+offset);
    game->Hash ^= ZobristPositionHash[side][King][E1+offset];

    RemovePiece(chessSet, side, Rook, D1+offset);
    game->Hash ^= ZobristPositionHash[side][Rook][D1+offset];

    PlacePiece(chessSet, side, Rook, A1+offset);
    game->Hash ^= ZobristPositionHash[side][Rook][A1+offset];

    indexTo = chessSet->PiecePositionIndexes[C1 + offset];

    chessSet->PiecePositionIndexes[E1 + offset] = indexTo;
    chessSet->PiecePositions[side][King][indexTo] = E1 + offset;

    indexTo = chessSet->PiecePositionIndexes[D1 + offset];

    chessSet->PiecePositionIndexes[A1 + offset] = indexTo;
    chessSet->PiecePositions[side][Rook][indexTo] = A1 + offset;

    // TODO: Do faster. Do this for now.
    UpdateOccupancies(chessSet);

    break;
  case CastleKingSide:
    offset = game->WhosTurn == White ? 0 : 8*7;

    RemovePiece(chessSet, side, King, G1+offset);
    game->Hash ^= ZobristPositionHash[side][King][G1+offset];

    PlacePiece(chessSet, side, King, E1+offset);
    game->Hash ^= ZobristPositionHash[side][King][E1+offset];

    RemovePiece(chessSet, side, Rook, F1+offset);
    game->Hash ^= ZobristPositionHash[side][Rook][F1+offset];

    PlacePiece(chessSet, side, Rook, H1+offset);
    game->Hash ^= ZobristPositionHash[side][Rook][H1+offset];

    indexTo = chessSet->PiecePositionIndexes[G1 + offset];

    chessSet->PiecePositionIndexes[E1 + offset] = indexTo;
    chessSet->PiecePositions[side][King][indexTo] = E1 + offset;

    indexTo = chessSet->PiecePositionIndexes[F1 + offset];

    chessSet->PiecePositionIndexes[H1 + offset] = indexTo;
    chessSet->PiecePositions[side][Rook][indexTo] = H1 + offset;

    // TODO: Do faster. Do this for now.
    UpdateOccupancies(chessSet);

    break;
  default:
    panic("Unrecognised move type %d.", TYPE(move));
  }

  game->CheckStats = memory.CheckStats;

  if(game->EnPassantSquare != EmptyPosition) {
    game->Hash ^= ZobristEnPassantFileHash[FILE(game->EnPassantSquare)];
  }

  game->EnPassantSquare = memory.EnPassantSquare;

  if(game->EnPassantSquare != EmptyPosition) {
    game->Hash ^= ZobristEnPassantFileHash[FILE(game->EnPassantSquare)];    
  }

  castleEvent = memory.CastleEvent;

  if(castleEvent != NoCastleEvent) {
    if(castleEvent&LostKingSideWhite) {
      game->CastlingRights[White][KingSide] = true;
      game->Hash ^= ZobristCastlingHash[White][KingSide];
    }
    if(castleEvent&LostQueenSideWhite) {
      game->CastlingRights[White][QueenSide] = true;
      game->Hash ^= ZobristCastlingHash[White][QueenSide];
    }
    if(castleEvent&LostKingSideBlack) {
      game->CastlingRights[Black][KingSide] = true;
      game->Hash ^= ZobristCastlingHash[Black][KingSide];
    }
    if(castleEvent&LostQueenSideBlack) {
      game->CastlingRights[Black][QueenSide] = true;
      game->Hash ^= ZobristCastlingHash[Black][QueenSide];
    }
  }

#ifndef NDEBUG
  if((msg = checkConsistency(game, EmptyBoard, EmptyBoard)) != NULL) {
    printf("Inconsistency in %s's Unmove of %s at unmoveCount %lu:-\n\n",
           StringSide(side),
           StringMoveFull(move, piece, capturePiece != MissingPiece),
           unmoveCount);
    puts(StringChessSet(chessSet));
    puts(msg);
    abort();
  }
#endif
}

static FORCE_INLINE void
doCastleKingSide(Game *game)
{
  ChessSet *chessSet = &game->ChessSet;
  int index;
  int offset = game->WhosTurn*8*7;
  Side side = game->WhosTurn;

  RemovePiece(chessSet, side, King, E1 + offset);
  game->Hash ^= ZobristPositionHash[side][King][E1+offset];

  PlacePiece(chessSet, side, King, G1 + offset);
  game->Hash ^= ZobristPositionHash[side][King][G1+offset];

  RemovePiece(chessSet, side, Rook, H1 + offset);
  game->Hash ^= ZobristPositionHash[side][Rook][H1+offset];

  PlacePiece(chessSet, side, Rook, F1 + offset);
  game->Hash ^= ZobristPositionHash[side][Rook][F1+offset];

  index = chessSet->PiecePositionIndexes[E1 + offset];

  chessSet->PiecePositionIndexes[G1 + offset] = index;
  chessSet->PiecePositions[side][King][index] = G1 + offset;

  index = chessSet->PiecePositionIndexes[H1 + offset];

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
  game->Hash ^= ZobristPositionHash[side][King][E1+offset];

  PlacePiece(chessSet, side, King, C1 + offset);
  game->Hash ^= ZobristPositionHash[side][King][C1+offset];

  RemovePiece(chessSet, side, Rook, A1 + offset);
  game->Hash ^= ZobristPositionHash[side][Rook][A1+offset];

  PlacePiece(chessSet, side, Rook, D1 + offset);
  game->Hash ^= ZobristPositionHash[side][Rook][D1+offset];

  index = chessSet->PiecePositionIndexes[E1 + offset];

  chessSet->PiecePositionIndexes[C1 + offset] = index;
  chessSet->PiecePositions[side][King][index] = C1 + offset;

  index = chessSet->PiecePositionIndexes[A1 + offset];

  chessSet->PiecePositionIndexes[D1 + offset] = index;
  chessSet->PiecePositions[side][Rook][index] = D1 + offset;

  UpdateOccupancies(&game->ChessSet);
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

      if(!(queenThreats&POSBOARD(to))) {
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
  unsigned int offset;
  CastleEvent ret = NoCastleEvent;
  Side side = game->WhosTurn;
  Side opposite = OPPOSITE(side);

  if(TYPE(move) == CastleKingSide || TYPE(move) == CastleQueenSide) {
    if(game->CastlingRights[side][QueenSide]) {
      game->CastlingRights[side][QueenSide] = false;
      game->Hash ^= ZobristCastlingHash[side][QueenSide];

      ret = side == White ? LostQueenSideWhite : LostQueenSideBlack;
    }

    if(game->CastlingRights[side][KingSide]) {
      game->CastlingRights[side][KingSide] = false;
      game->Hash ^= ZobristCastlingHash[side][KingSide];

      ret |= side == White ? LostKingSideWhite : LostKingSideBlack;
    }

    return ret;
  }

  if(capture) {
    if(TO(move) == A1+opposite*8*7 && game->CastlingRights[opposite][QueenSide]) {
      game->CastlingRights[opposite][QueenSide] = false;
      game->Hash ^= ZobristCastlingHash[opposite][QueenSide];

      ret = opposite == White ? LostQueenSideWhite : LostQueenSideBlack;
    } else if(TO(move) == H1+opposite*8*7 && game->CastlingRights[opposite][KingSide]) {
      game->CastlingRights[opposite][KingSide] = false;
      game->Hash ^= ZobristCastlingHash[opposite][KingSide];

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
      game->Hash ^= ZobristCastlingHash[side][QueenSide];

    }
    if(game->CastlingRights[side][KingSide]) {
      ret |= side == White ? LostKingSideWhite : LostKingSideBlack;
      game->CastlingRights[side][KingSide] = false;
      game->Hash ^= ZobristCastlingHash[side][KingSide];
    }
  } else {
    if(FROM(move) == A1+offset && game->CastlingRights[side][QueenSide]) {
      ret |= side == White ? LostQueenSideWhite : LostQueenSideBlack;
      game->CastlingRights[side][QueenSide] = false;
      game->Hash ^= ZobristCastlingHash[side][QueenSide];
    }
    if(FROM(move) == H1+offset && game->CastlingRights[side][KingSide]) {
      ret |= side == White ? LostKingSideWhite : LostKingSideBlack;
      game->CastlingRights[side][KingSide] = false;
      game->Hash ^= ZobristCastlingHash[side][KingSide];
    }
  }

  return ret;
}

// Toggle whose turn it is.
static FORCE_INLINE void
toggleTurn(Game *game) {
  game->WhosTurn = OPPOSITE(game->WhosTurn);
  game->Hash ^= ZobristBlackHash;
}

#ifndef NDEBUG

// Helper debug function which checks the consistency of the game object to ensure
// everything should be as it is, returning a message if not.
static char*
checkConsistency(Game *game, BitBoard checks, BitBoard modelChecks)
{
  BitBoard bitBoard;
  BitBoard occupancy = EmptyBoard, emptySquares = EmptyBoard;
  BitBoard sideOccupancies[2] = {EmptyBoard, EmptyBoard};
  BitBoard sideEmptySquares[2] = {EmptyBoard, EmptyBoard};
  bool seen;
  ChessSet *chessSet = &game->ChessSet;
  int i, index, pieceCount, seenCount;
  Side side;
  Piece piece, seenPiece;
  Position pos;
  Position *currPos;
  StringBuilder builder = NewStringBuilder();

  // TODO: Reduce duplication.

  // Occupancies checks.

  for(side = White; side <= Black; side++) {
    for(piece = Pawn; piece <= King; piece++) {
      occupancy |= chessSet->Sets[side].Boards[piece];
      sideOccupancies[side] |= chessSet->Sets[side].Boards[piece];
    }

    sideEmptySquares[side] = ~sideOccupancies[side];

    if(chessSet->Sets[side].Occupancy != sideOccupancies[side]) {
      AppendString(&builder, "%s's occupancy is incorrect.\n\n"
                   "Expected:-\n\n"
                   "%s\n"
                   "Actual:-\n\n"
                   "%s\n",
                   StringSide(side),
                   StringBitBoard(sideOccupancies[side]),
                   StringBitBoard(chessSet->Sets[side].Occupancy));
    }

    if(chessSet->Sets[side].EmptySquares != sideEmptySquares[side]) {
      AppendString(&builder, "%s's empty squares are incorrect.\n\n"
                   "Expected:-\n\n"
                   "%s\n"
                   "Actual:-\n\n"
                   "%s\n",
                   StringSide(side),
                   StringBitBoard(sideEmptySquares[side]),
                   StringBitBoard(chessSet->Sets[side].EmptySquares));
    }
  }

  for(piece = Pawn; piece <= King; piece++) {
    bitBoard = chessSet->Sets[White].Boards[piece] | chessSet->Sets[Black].Boards[piece];
    if(chessSet->PieceOccupancy[piece] !=
       bitBoard) {
      AppendString(&builder, "%s piece occupancy is incorrect.\n\n"
                   "Expected:-\n\n"
                   "%s\n"
                   "Actual:-\n\n"
                   "%s\n",
                   StringPiece(piece),
                   StringBitBoard(bitBoard),
                   StringBitBoard(chessSet->PieceOccupancy[piece]));
    }
  }

  emptySquares = ~occupancy;
  if(emptySquares != chessSet->EmptySquares) {
    if(chessSet->EmptySquares != emptySquares) {
      AppendString(&builder, "Overall empty squares are incorrect.\n\n"
                   "Expected:-\n\n"
                   "%s\n"
                   "Actual:-\n\n"
                   "%s\n",
                   StringBitBoard(emptySquares),
                   StringBitBoard(chessSet->EmptySquares));
    }
  }

  for(pos = A1; pos <= H8; pos++) {
    seenCount = 0;
    seenPiece = MissingPiece;

    for(piece = Pawn; piece <= King; piece++) {
      bitBoard = chessSet->Sets[White].Boards[piece] |
        chessSet->Sets[Black].Boards[piece];

      if((POSBOARD(pos)&bitBoard) == POSBOARD(pos)) {
        seenCount++;
        seenPiece = piece;

        if(seenCount > 1) {
          AppendString(&builder, "%s overlaps at %s.\n",
                       StringPiece(piece), StringPosition(pos));
        }
      }
    }

    if(chessSet->Squares[pos] != seenPiece) {
      AppendString(&builder, "%s at %s, but Squares[%s] = %s.\n",
                   StringPiece(seenPiece), StringPosition(pos),
                   StringPosition(pos), StringPiece(chessSet->Squares[pos]));
    }
  }

  // Piece position checks.

  for(side = White; side <= Black; side++) {
    for(piece = Pawn; piece <= King; piece++) {
      pieceCount = PopCount(chessSet->Sets[side].Boards[piece]);

      if(pieceCount != chessSet->PieceCounts[side][piece]) {
        AppendString(&builder, "%d %s %ss on the board, but PieceCounts[%s][%s] = %d.\n",
                     pieceCount, StringSide(side), StringPiece(piece),
                     StringSide(side), StringPiece(piece),
                     chessSet->PieceCounts[side][piece]);
      }

      currPos = chessSet->PiecePositions[side][piece];

      bitBoard = EmptyBoard;
      for(i = 0; i < MAX_PIECE_LOCATION && *currPos != EmptyPosition; i++, currPos++) {
        pos = *currPos;

        bitBoard |= POSBOARD(pos);
      }

      if(i > MAX_PIECE_LOCATION) {
        AppendString(&builder, "Ran off end of PiecePositions[%s][%s], "
                     "MAX_PIECE_LOCATION = %d.\n",
                     StringSide(side), StringPiece(piece), MAX_PIECE_LOCATION);
      } else if(bitBoard != chessSet->Sets[side].Boards[piece]) {
        AppendString(&builder, "PiecePositions[%s][%s] gives incorrect results.\n\n"
                     "Expected:-\n\n"
                     "%s\n"
                     "Actual:-\n\n"
                     "%s\n",
                     StringSide(side), StringPiece(piece),
                     StringBitBoard(chessSet->Sets[side].Boards[piece]),
                     StringBitBoard(bitBoard));
      }
    }
  }

  // We've already checked overlaps here, so not a problem to not consider that.
  for(pos = A1; pos <= H8; pos++) {
    seen = false;

    // TODO: Use goto?
    for(side = White; side <= Black; side++) {
      for(piece = Pawn; piece <= King; piece++) {
        if(chessSet->Sets[side].Boards[piece]&POSBOARD(pos)) {
          seen = true;
          break;
        }
        if(seen) {
          break;
        }
      }
      if(seen) {
        break;
      }
    }

    // Empty square. PiecePositionIndexes can be left stale here so nothing to check.
    if(!seen) {
      continue;
    }

    index = chessSet->PiecePositionIndexes[pos];

    if(index < 0) {
      AppendString(&builder, "Index %d < 0 for position %s.\n", index, StringPosition(pos));
    } else if(index >= MAX_PIECE_LOCATION) {
      AppendString(&builder, "Index %d for position %s exceeds MAX_PIECE_LOCATION = %d.\n",
                   index, StringPosition(pos), MAX_PIECE_LOCATION);
    } else if(chessSet->PiecePositions[side][piece][index] != pos) {
      AppendString(&builder, "Index %d for position %s != PiecePositions[%s][%s][%d] = %s.\n",
                   index, StringPosition(pos), StringSide(side), StringPiece(piece), index,
                   StringPosition(chessSet->PiecePositions[side][piece][index]));
    }
  }

  if(checks != modelChecks) {
    AppendString(&builder, "Incorrect check source squares.\n\n"
                 "Expected:-\n\n"
                 "%s\n"
                 "Actual:-\n\n"
                 "%s",
                 StringBitBoard(modelChecks), StringBitBoard(checks));
  }

  // Check hashes.
  if(game->Hash != HashGame(game)) {
    AppendString(&builder, "Incorrect hash - calculated 0x%lx, game->Hash is 0x%lx.\n",
                 HashGame(game), game->Hash);
  }

  if(builder.Length == 0) {
    return NULL;
  }

  AppendString(&builder, "\nMove History:-\n\n"
               "%s",
               StringMoveHistory(&game->Memories, false));

  return BuildString(&builder, true);
}

#endif
