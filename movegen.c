#include "weak.h"

static void bishopMoves(Game*, MoveSlice*, BitBoard, BitBoard, BitBoard, BitBoard);
static void castleMoves(Game*, MoveSlice*);
static void evasions(MoveSlice*, Game*);
static void kingMoves(Game *, MoveSlice *, BitBoard, BitBoard, BitBoard);
static void knightMoves(Game*, MoveSlice*, BitBoard, BitBoard, BitBoard);
static void nonEvasions(MoveSlice*, Game*);
static void pawnMoves(Game*, MoveSlice*, BitBoard);
static void queenMoves(Game*, MoveSlice*, BitBoard, BitBoard, BitBoard, BitBoard);
static void rookMoves(Game*, MoveSlice*, BitBoard, BitBoard, BitBoard, BitBoard);

static void pawnMovesBlackNonEvasion(Game*, MoveSlice*);
static void pawnMovesWhiteNonEvasion(Game*, MoveSlice*);

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

  pawnMoves(game, slice, targets);

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

  pawnMoves(game, slice, FullyOccupied);

  /*
  if(game->WhosTurn == White) {
    pawnMovesWhiteNonEvasion(game, slice);
  } else {
    pawnMovesBlackNonEvasion(game, slice);
  }
  */

  knightMoves(game, slice, empty, opposition,            FullyOccupied);
  bishopMoves(game, slice, empty, opposition, occupancy, FullyOccupied);
  rookMoves  (game, slice, empty, opposition, occupancy, FullyOccupied);
  queenMoves (game, slice, empty, opposition, occupancy, FullyOccupied);
  kingMoves  (game, slice, empty, opposition,            FullyOccupied);

  castleMoves(game, slice);
}

static void
pawnMoves(Game *game, MoveSlice *slice, BitBoard mask)
{
  bool singleOk, doubleOk;
  int k;
  BitBoard pushSources, captureSources, captureTargets, enPassantMask, fromBoard,
    toBoard;
  Move move;
  MoveType promotions[] = {PromoteKnight, PromoteBishop, PromoteRook, PromoteQueen};
  Rank fromRank;
  Rank doublePushRank = game->WhosTurn == White ? Rank2 : Rank7;
  Rank promotionRank = game->WhosTurn == White ? Rank7 : Rank2;
  int offset = game->WhosTurn == White ? 8 : -8;

  BitBoard empty = game->ChessSet.EmptySquares;

  pushSources = AllPawnPushSources(&game->ChessSet, game->WhosTurn);
  captureSources = AllPawnCaptureSources(&game->ChessSet, game->WhosTurn);

  move.Piece = Pawn;
  move.Capture = false;
  move.Type = Normal;

  // Pushes.
  while(pushSources) {
    move.From = PopForward(&pushSources);
    fromRank = RANK(move.From);

    move.To = move.From + offset;

    singleOk = (POSBOARD(move.To)&mask&empty) != EmptyBoard;
    doubleOk = (POSBOARD(move.To+offset)&mask&empty) != EmptyBoard;

    if(fromRank == promotionRank && singleOk) {
      for(k = 0; k < 4; k++) {
        move.Type = promotions[k];
        AppendMove(slice, move);
      }
      move.Type = Normal;
    } else if(fromRank == doublePushRank) {
      if(singleOk) {
        AppendMove(slice, move);
      }
      if(doubleOk) {
        move.To += offset;
        AppendMove(slice, move);
      }
    } else if(singleOk) {
      AppendMove(slice, move);
    }
  }

  // Captures.

  move.Capture = true;

  while(captureSources) {
    move.From = PopForward(&captureSources);

    captureTargets = PawnCaptureTargets(&game->ChessSet, game->WhosTurn,
                                        POSBOARD(move.From)) & mask;

    if(RANK(move.From) == promotionRank) {
      while(captureTargets) {
        move.To = PopForward(&captureTargets);

        for(k = 0; k < 4; k++) {
          move.Type = promotions[k];
          AppendMove(slice, move);
        }
        move.Type = Normal;
      }
    } else {
      while(captureTargets) {
        move.To = PopForward(&captureTargets);

        AppendMove(slice, move);
      }
    }
  }

  // En passant.

  if(game->EnPassantSquare == EmptyPosition) {
    return;
  }

  toBoard = POSBOARD(game->EnPassantSquare);
  enPassantMask = POSBOARD(game->EnPassantSquare + 8*(-1 + 2*game->WhosTurn));

  if(((toBoard | enPassantMask) & mask) == EmptyBoard) {
    return;
  }

  switch(game->WhosTurn) {
  case White:
    fromBoard = SoWeOne(toBoard) | SoEaOne(toBoard);
    break;
  case Black:
    fromBoard = NoWeOne(toBoard) | NoEaOne(toBoard);
    break;
  default:
    panic("Unrecognised side %d.", game->WhosTurn);
  }

  fromBoard &= game->ChessSet.Sets[game->WhosTurn].Boards[Pawn];

  move.Type = EnPassant;
  move.To = game->EnPassantSquare;
  move.Capture = true;

  while(fromBoard) {
    move.From = PopForward(&fromBoard);

    AppendMove(slice, move);
  }
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
pawnMovesWhiteNonEvasion(Game *game, MoveSlice *slice)
{
  BitBoard empty = game->ChessSet.EmptySquares;
  BitBoard opposition = game->ChessSet.Sets[Black].Occupancy;

  BitBoard b1, b2;

  BitBoard pawns = game->ChessSet.Sets[White].Boards[Pawn];

  BitBoard pawnsOn7 = pawns&Rank7Mask;
  BitBoard pawnsNotOn7 = pawns&~Rank7Mask;

  Move move;

  Position to;

  move.Piece = Pawn;
  move.Type = Normal;
  move.Capture = false;

  // Moves.

  b1 = NortOne(pawnsNotOn7) & empty;
  b2 = NortOne(b1 & Rank3Mask) & empty;

  while(b1) {
    to = PopForward(&b1);

    move.From = to-8;
    move.To = to;

    AppendMove(slice, move);
  }

  while(b2) {
    to = PopForward(&b2);

    move.From = to-16;
    move.To = to;

    AppendMove(slice, move);
  }

  // Promotions.
  if(pawnsOn7 != EmptyBoard) {

  }

  // Captures.

  move.Capture = true;

  b1 = NoWeOne(pawnsNotOn7) & opposition;
  b2 = NoEaOne(pawnsNotOn7) & opposition;

  while(b1) {
    to = PopForward(&b1);

    move.From = to-7;
    move.To = to;

    AppendMove(slice, move);
  }

  while(b2) {
    to = PopForward(&b2);

    move.From = to-9;
    move.To = to;

    AppendMove(slice, move);
  }

  // En passant.

}

static void
pawnMovesBlackNonEvasion(Game *game, MoveSlice *slice)
{
  BitBoard empty = game->ChessSet.EmptySquares;
  BitBoard opposition = game->ChessSet.Sets[White].Occupancy;

  BitBoard b1, b2;

  BitBoard pawns = game->ChessSet.Sets[Black].Boards[Pawn];

  BitBoard pawnsOn7 = pawns&Rank2Mask;
  BitBoard pawnsNotOn7 = pawns&~Rank2Mask;

  Move move;

  Position to;

  move.Piece = Pawn;
  move.Type = Normal;
  move.Capture = false;

  // Moves.

  b1 = SoutOne(pawnsNotOn7) & empty;
  b2 = SoutOne(b1 & Rank6Mask) & empty;

  while(b1) {
    to = PopForward(&b1);

    move.From = to+8;
    move.To = to;

    AppendMove(slice, move);
  }

  while(b2) {
    to = PopForward(&b2);

    move.From = to+16;
    move.To = to;

    AppendMove(slice, move);
  }

  // Promotions.
  if(pawnsOn7 != EmptyBoard) {

  }

  // Captures.

  move.Capture = true;

  b1 = SoWeOne(pawnsNotOn7) & opposition;
  b2 = SoEaOne(pawnsNotOn7) & opposition;

  while(b1) {
    to = PopForward(&b1);

    move.From = to+9;
    move.To = to;

    AppendMove(slice, move);
  }

  while(b2) {
    to = PopForward(&b2);

    move.From = to+7;
    move.To = to;

    AppendMove(slice, move);
  }

  // En passant.

}
