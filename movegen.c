#include "weak.h"

static void bishopMoves(Game*, MoveSlice*, BitBoard, BitBoard, BitBoard, BitBoard);
static void castleMoves(Game*, MoveSlice*);
static void evasions(MoveSlice*, Game*);
static void kingMoves(Game *, MoveSlice *, BitBoard, BitBoard, BitBoard);
static void knightMoves(Game*, MoveSlice*, BitBoard, BitBoard, BitBoard);
static void nonEvasions(MoveSlice*, Game*);
static void queenMoves(Game*, MoveSlice*, BitBoard, BitBoard, BitBoard, BitBoard);
static void rookMoves(Game*, MoveSlice*, BitBoard, BitBoard, BitBoard, BitBoard);

static void pawnMovesBlack(Game*, MoveSlice*, BitBoard);
static void pawnMovesWhite(Game*, MoveSlice*, BitBoard);

// Get all valid moves for the current player.
void
AllMoves(MoveSlice *slice, Game *game)
{
  BitBoard pinned;
  Move *curr;

  if(game->CheckStats.CheckSources == EmptyBoard) {
    nonEvasions(slice, game);
  } else {
    evasions(slice, game);
  }

  pinned = game->CheckStats.Pinned;

  // Filter out illegal moves.
  for(curr = slice->Vals; curr < slice->Curr;) {
    if(!PseudoLegal(game, curr, pinned)) {
      // Switch last move with the one we are rejecting.
      slice->Curr--;
      *curr = *slice->Curr;
    } else {
      curr++;
    }
  }
}


static void
bishopMoves(Game *game, MoveSlice *slice, BitBoard empty, BitBoard opposition,
            BitBoard occupancy, BitBoard mask)
{
  Side side = game->WhosTurn;

  BitBoard attacks, captureTargets, moveTargets;
  BitBoard pieceBoard = game->ChessSet.Sets[side].Boards[Bishop];
  Position from, to;
  Move move;

  move.Piece = Bishop;
  move.Type = Normal;

  while(pieceBoard) {
    from = PopForward(&pieceBoard);

    attacks = BishopAttacksFrom(from, occupancy) & mask;

    move.From = from;

    // Moves.
    moveTargets = attacks & empty;

    move.Capture = false;
    while(moveTargets != EmptyBoard) {
      to = PopForward(&moveTargets);
      move.To = to;

      AppendMove(slice, move);
    }

    // Captures.
    captureTargets = attacks & opposition;
    move.Capture = true;
    while(captureTargets != EmptyBoard) {
      to = PopForward(&captureTargets);
      move.To = to;

      AppendMove(slice, move);
    }
  }
}

static void
castleMoves(Game *game, MoveSlice *ret)
{
  BitBoard attackMask, occupancy, opposition;
  bool good;
  CastleSide castleSide;
  Move move;
  Position king, pos;
  Side side = game->WhosTurn;
  Side opposite;

  king = E1 + side*8*7;

  // Add unneeded fields to prevent garbage in unassigned fields. TODO: Review.
  move.Capture = false;
  move.From = king;
  move.Piece = King;
  move.To = king - 2;

  // If we have the rights and aren't obstructed...
  for(castleSide = KingSide; castleSide <= QueenSide; castleSide++) {
    if(game->CastlingRights[side][castleSide] &&
       (game->ChessSet.Occupancy&CastlingMasks[side][castleSide]) == EmptyBoard) {
      occupancy = game->ChessSet.Occupancy;
      opposite = OPPOSITE(side);
      opposition = game->ChessSet.Sets[opposite].Occupancy;

      good = true;
      // ...Determine whether we are attacked along the attack mask.
      attackMask = CastlingAttackMasks[side][castleSide];
      while(attackMask) {
        pos = PopForward(&attackMask);

        if((AllAttackersTo(&game->ChessSet, pos, occupancy) & opposition) != EmptyBoard) {
          good = false;
          break;
        }
      }

      if(good) {
        move.Type = castleSide == KingSide ? CastleKingSide : CastleQueenSide;
        AppendMove(ret, move);
      }
    }
  }
}

static void
evasions(MoveSlice *slice, Game *game)
{
  BitBoard attacks, captures, moves, targets;
  BitBoard checks = game->CheckStats.CheckSources;
  BitBoard empty = game->ChessSet.EmptySquares;

  BitBoard slideAttacks = EmptyBoard;
  int checkCount = 0;
  Move move;
  Piece piece;
  Position check;
  Position king = game->CheckStats.DefendedKing;
  Side side = game->WhosTurn;
  Side opposite = OPPOSITE(side);
  BitBoard occupancy = game->ChessSet.Occupancy;
  BitBoard opposition = game->ChessSet.Sets[opposite].Occupancy;

  while(checks != EmptyBoard) {
    check = PopForward(&checks);

    checkCount++;

    piece = PieceAt(&game->ChessSet.Sets[opposite], check);

    switch(piece) {
    case Bishop:
      slideAttacks |= EmptyAttacks[Bishop][check];

      break;
    case Rook:
      slideAttacks |= EmptyAttacks[Rook][check];

      break;
    case Queen:
      // If king and queen are far away, i.e. there are squares between them, or they are not
      // on a diagonal, we can remove all squares in all directions as the king can't get to them.
      if(Between[king][check] != EmptyBoard ||
         (EmptyAttacks[Bishop][check] & POSBOARD(king)) == EmptyBoard) {
        slideAttacks |= EmptyAttacks[Queen][check];
      } else {
        slideAttacks |= EmptyAttacks[Bishop][check] |
          RookAttacksFrom(check, occupancy);
      }

      break;
    default:
      break;
    }
  }

  attacks = KingAttacksFrom(king) & ~slideAttacks;

  move.Type = Normal;
  move.Piece = King;
  move.From = king;
  move.Capture = false;

  // King evasion moves.

  moves = attacks & game->ChessSet.EmptySquares;
  while(moves) {
    move.To = PopForward(&moves);

    AppendMove(slice, move);
  }

  move.Capture = true;

  captures = attacks & opposition;
  while(captures) {
    move.To = PopForward(&captures);

    AppendMove(slice, move);
  }

  // If there is more than 1 check, blocking won't achieve anything.
  if(checkCount > 1) {
    return;
  }

  // Blocking/capturing the checking piece.
  // We use check from the loop above, since we have only 1 check this will
  // be the sole checker.
  targets = Between[check][king] | game->CheckStats.CheckSources;

  if(side == White) {
    pawnMovesWhite(game, slice, targets);
  } else {
    pawnMovesBlack(game, slice, targets);
  }

  // King already handled.
  knightMoves(game, slice, empty, opposition, targets);
  bishopMoves(game, slice, empty, opposition, occupancy, targets);
  rookMoves  (game, slice, empty, opposition, occupancy, targets);
  queenMoves (game, slice, empty, opposition, occupancy, targets);
}

static void
kingMoves(Game *game, MoveSlice *slice, BitBoard empty, BitBoard opposition, BitBoard mask)
{
  BitBoard attacks, captureTargets, moveTargets;
  Position to;
  Position from = game->CheckStats.DefendedKing;
  Move move;

  move.Piece = King;
  move.Type = Normal;

  attacks = KingAttacksFrom(from) & mask;

  move.From = from;

  // Moves.
  moveTargets = attacks & empty;
  move.Capture = false;
  while(moveTargets != EmptyBoard) {
    to = PopForward(&moveTargets);
    move.To = to;
    AppendMove(slice, move);
  }

  // Captures.
  captureTargets = attacks & opposition;
  move.Capture = true;
  while(captureTargets != EmptyBoard) {
    to = PopForward(&captureTargets);

    move.To = to;
    AppendMove(slice, move);
  }
}

static void
knightMoves(Game *game, MoveSlice *slice, BitBoard empty, BitBoard opposition, BitBoard mask)
{
  Side side = game->WhosTurn;

  BitBoard attacks, captureTargets, moveTargets;
  BitBoard pieceBoard = game->ChessSet.Sets[side].Boards[Knight];
  Position from, to;
  Move move;

  move.Piece = Knight;
  move.Type = Normal;

  while(pieceBoard) {
    from = PopForward(&pieceBoard);

    attacks = KnightAttacksFrom(from) & mask;

    move.From = from;

    // Moves.
    moveTargets = attacks & empty;
    move.Capture = false;
    while(moveTargets != EmptyBoard) {
      to = PopForward(&moveTargets);
      move.To = to;
      AppendMove(slice, move);
    }

    // Captures.
    captureTargets = attacks & opposition;
    move.Capture = true;
    while(captureTargets != EmptyBoard) {
      to = PopForward(&captureTargets);

      move.To = to;
      AppendMove(slice, move);
    }
  }
}

static void
nonEvasions(MoveSlice *slice, Game *game)
{
  BitBoard empty = game->ChessSet.EmptySquares;
  Side side = game->WhosTurn;
  Side opposite = OPPOSITE(side);
  BitBoard occupancy =  game->ChessSet.Occupancy;
  BitBoard opposition = game->ChessSet.Sets[opposite].Occupancy;

  if(side == White) {
    pawnMovesWhite(game, slice, FullyOccupied);
  } else {
    pawnMovesBlack(game, slice, FullyOccupied);
  }

  knightMoves(game, slice, empty, opposition,            FullyOccupied);
  bishopMoves(game, slice, empty, opposition, occupancy, FullyOccupied);
  rookMoves  (game, slice, empty, opposition, occupancy, FullyOccupied);
  queenMoves (game, slice, empty, opposition, occupancy, FullyOccupied);
  kingMoves  (game, slice, empty, opposition,            FullyOccupied);

  castleMoves(game, slice);
}

static void
queenMoves(Game *game, MoveSlice *slice, BitBoard empty, BitBoard opposition,
            BitBoard occupancy, BitBoard mask)
{
  Side side = game->WhosTurn;

  BitBoard attacks, captureTargets, moveTargets;
  BitBoard pieceBoard = game->ChessSet.Sets[side].Boards[Queen];
  Position from, to;
  Move move;

  move.Piece = Queen;
  move.Type = Normal;

  while(pieceBoard) {
    from = PopForward(&pieceBoard);

    attacks = (BishopAttacksFrom(from, occupancy) | RookAttacksFrom(from, occupancy)) & mask;

    move.From = from;

    // Moves.
    moveTargets = attacks & empty;

    move.Capture = false;
    while(moveTargets != EmptyBoard) {
      to = PopForward(&moveTargets);
      move.To = to;

      AppendMove(slice, move);
    }

    // Captures.
    captureTargets = attacks & opposition;
    move.Capture = true;
    while(captureTargets != EmptyBoard) {
      to = PopForward(&captureTargets);
      move.To = to;

      AppendMove(slice, move);
    }
  }
}

static void
rookMoves(Game *game, MoveSlice *slice, BitBoard empty, BitBoard opposition,
            BitBoard occupancy, BitBoard mask)
{
  Side side = game->WhosTurn;

  BitBoard attacks, captureTargets, moveTargets;
  BitBoard pieceBoard = game->ChessSet.Sets[side].Boards[Rook];
  Position from, to;
  Move move;

  move.Piece = Rook;
  move.Type = Normal;

  while(pieceBoard) {
    from = PopForward(&pieceBoard);

    attacks = RookAttacksFrom(from, occupancy) & mask;

    move.From = from;

    // Moves.
    moveTargets = attacks & empty;

    move.Capture = false;
    while(moveTargets != EmptyBoard) {
      to = PopForward(&moveTargets);
      move.To = to;

      AppendMove(slice, move);
    }

    // Captures.
    captureTargets = attacks & opposition;
    move.Capture = true;
    while(captureTargets != EmptyBoard) {
      to = PopForward(&captureTargets);
      move.To = to;

      AppendMove(slice, move);
    }
  }
}


// EXPERIMENTAL:-


static void
pawnMovesWhite(Game *game, MoveSlice *slice, BitBoard mask)
{
  BitBoard empty = game->ChessSet.EmptySquares;
  BitBoard opposition = game->ChessSet.Sets[Black].Occupancy & mask;

  BitBoard bitBoard1, bitBoard2;

  BitBoard pawns = game->ChessSet.Sets[White].Boards[Pawn];

  BitBoard pawnsOn7 = pawns&Rank7Mask;
  BitBoard pawnsNotOn7 = pawns&~Rank7Mask;

  Move move;

  Position enPassant, to;

  move.Piece = Pawn;
  move.Type = Normal;
  move.Capture = false;

  // Moves.

  bitBoard1 = NortOne(pawnsNotOn7) & empty;
  bitBoard2 = NortOne(bitBoard1 & Rank3Mask) & empty;

  bitBoard1 &= mask;
  bitBoard2 &= mask;

  while(bitBoard1) {
    to = PopForward(&bitBoard1);

    move.From = to-8;
    move.To = to;

    AppendMove(slice, move);
  }

  while(bitBoard2) {
    to = PopForward(&bitBoard2);

    move.From = to-16;
    move.To = to;

    AppendMove(slice, move);
  }

  // Promotions.
  if(pawnsOn7 != EmptyBoard && (mask & Rank8Mask) != EmptyBoard) {
    bitBoard1 = NortOne(pawnsOn7) & empty;

    while(bitBoard1) {
      to = PopForward(&bitBoard1);

      move.From = to-8;
      move.To = to;

      move.Type = PromoteBishop;
      AppendMove(slice, move);

      move.Type = PromoteKnight;
      AppendMove(slice, move);

      move.Type = PromoteRook;
      AppendMove(slice, move);

      move.Type = PromoteQueen;
      AppendMove(slice, move);
    }

    move.Capture = true;

    bitBoard1 = NoWeOne(pawnsOn7) & opposition;
    while(bitBoard1) {
      to = PopForward(&bitBoard1);

      move.From = to-7;
      move.To = to;

      move.Type = PromoteBishop;
      AppendMove(slice, move);

      move.Type = PromoteKnight;
      AppendMove(slice, move);

      move.Type = PromoteRook;
      AppendMove(slice, move);

      move.Type = PromoteQueen;
      AppendMove(slice, move);
    }

    bitBoard2 = NoEaOne(pawnsOn7) & opposition;
    while(bitBoard2) {
      to = PopForward(&bitBoard2);

      move.From = to-9;
      move.To = to;

      move.Type = PromoteBishop;
      AppendMove(slice, move);

      move.Type = PromoteKnight;
      AppendMove(slice, move);

      move.Type = PromoteRook;
      AppendMove(slice, move);

      move.Type = PromoteQueen;
      AppendMove(slice, move);
    }
  }

  // Captures.

  move.Capture = true;
  move.Type = Normal;

  bitBoard1 = NoWeOne(pawnsNotOn7) & opposition;
  bitBoard2 = NoEaOne(pawnsNotOn7) & opposition;

  while(bitBoard1) {
    to = PopForward(&bitBoard1);

    move.From = to-7;
    move.To = to;

    AppendMove(slice, move);
  }

  while(bitBoard2) {
    to = PopForward(&bitBoard2);

    move.From = to-9;
    move.To = to;

    AppendMove(slice, move);
  }

  // En passant.
  enPassant = game->EnPassantSquare;
  if(enPassant != EmptyPosition) {
    if(mask != FullyOccupied && (mask & SoutOne(POSBOARD(enPassant))) != EmptyBoard) {
      return;
    }

    bitBoard1 = pawnsNotOn7 & PawnAttacksFrom(enPassant, Black);

    move.Type = EnPassant;

    while(bitBoard1) {
      move.From = PopForward(&bitBoard1);
      move.To = enPassant;

      AppendMove(slice, move);
    }
  }
}

static void
pawnMovesBlack(Game *game, MoveSlice *slice, BitBoard mask)
{
  BitBoard empty = game->ChessSet.EmptySquares;
  BitBoard opposition = game->ChessSet.Sets[White].Occupancy & mask;

  BitBoard bitBoard1, bitBoard2;

  BitBoard pawns = game->ChessSet.Sets[Black].Boards[Pawn];

  BitBoard pawnsOn2 = pawns&Rank2Mask;
  BitBoard pawnsNotOn2 = pawns&~Rank2Mask;

  Move move;

  Position enPassant, to;

  move.Piece = Pawn;
  move.Type = Normal;
  move.Capture = false;

  // Moves.

  bitBoard1 = SoutOne(pawnsNotOn2) & empty;
  bitBoard2 = SoutOne(bitBoard1 & Rank6Mask) & empty;

  bitBoard1 &= mask;
  bitBoard2 &= mask;

  while(bitBoard1) {
    to = PopForward(&bitBoard1);

    move.From = to+8;
    move.To = to;

    AppendMove(slice, move);
  }

  while(bitBoard2) {
    to = PopForward(&bitBoard2);

    move.From = to+16;
    move.To = to;

    AppendMove(slice, move);
  }

  // Promotions.
  if(pawnsOn2 != EmptyBoard) {
    bitBoard1 = SoutOne(pawnsOn2) & empty & mask;

    while(bitBoard1) {
      to = PopForward(&bitBoard1);

      move.From = to+8;
      move.To = to;

      move.Type = PromoteBishop;
      AppendMove(slice, move);

      move.Type = PromoteKnight;
      AppendMove(slice, move);

      move.Type = PromoteRook;
      AppendMove(slice, move);

      move.Type = PromoteQueen;
      AppendMove(slice, move);
    }

    move.Capture = true;

    bitBoard1 = SoWeOne(pawnsOn2) & opposition & mask;
    bitBoard2 = SoEaOne(pawnsOn2) & opposition & mask;

    while(bitBoard1) {
      to = PopForward(&bitBoard1);

      move.From = to+9;
      move.To = to;

      move.Type = PromoteBishop;
      AppendMove(slice, move);

      move.Type = PromoteKnight;
      AppendMove(slice, move);

      move.Type = PromoteRook;
      AppendMove(slice, move);

      move.Type = PromoteQueen;
      AppendMove(slice, move);
    }

    while(bitBoard2) {
      to = PopForward(&bitBoard2);

      move.From = to+7;
      move.To = to;

      move.Type = PromoteBishop;
      AppendMove(slice, move);

      move.Type = PromoteKnight;
      AppendMove(slice, move);

      move.Type = PromoteRook;
      AppendMove(slice, move);

      move.Type = PromoteQueen;
      AppendMove(slice, move);
    }
  }

  // Captures.

  move.Type = Normal;
  move.Capture = true;

  bitBoard1 = SoWeOne(pawnsNotOn2) & opposition & mask;
  bitBoard2 = SoEaOne(pawnsNotOn2) & opposition & mask;

  while(bitBoard1) {
    to = PopForward(&bitBoard1);

    move.From = to+9;
    move.To = to;

    AppendMove(slice, move);
  }

  while(bitBoard2) {
    to = PopForward(&bitBoard2);

    move.From = to+7;
    move.To = to;

    AppendMove(slice, move);
  }

  // En passant.
  enPassant = game->EnPassantSquare;
  if(enPassant != EmptyPosition) {
    if(mask != FullyOccupied && (mask & SoutOne(POSBOARD(enPassant))) != EmptyBoard) {
      return;
    }

    bitBoard1 = pawnsNotOn2 & PawnAttacksFrom(enPassant, White);

    move.Type = EnPassant;

    while(bitBoard1) {
      move.From = PopForward(&bitBoard1);
      move.To = enPassant;

      AppendMove(slice, move);
    }
  }
}
