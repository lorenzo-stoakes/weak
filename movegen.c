#include "weak.h"

static void castleMoves(Game*, MoveSlice*);
static void pawnMoves(Game*, MoveSlice*, BitBoard);
static void pieceMoves(Piece, Game*, MoveSlice*, BitBoard);

static void evasions(MoveSlice*, Game*);
static void nonEvasions(MoveSlice*, Game*);

// Get all valid moves for the current player.
void
AllMoves(MoveSlice *slice, Game *game)
{
  Move *curr;

  if(game->CheckStats.CheckSources == EmptyBoard) {
    nonEvasions(slice, game);
  } else {
    evasions(slice, game);
  }

  // Filter out illegal moves.
  for(curr = slice->Vals; curr < slice->Curr;) {
    if(!PseudoLegal(game, curr)) {
      // Switch last move with the one we are rejecting.
      slice->Curr--;
      *curr = *slice->Curr;
    } else {
      curr++;
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
pawnMoves(Game *game, MoveSlice *slice, BitBoard mask)
{
  int k;
  BitBoard pushSources, captureSources, pushTargets, captureTargets, enPassantMask, fromBoard,
    toBoard;
  Move move;
  MoveType promotions[] = {PromoteKnight, PromoteBishop, PromoteRook, PromoteQueen};
  Position from, to;
  Rank promotionRank = game->WhosTurn == White ? Rank8 : Rank1;

  pushSources = AllPawnPushSources(&game->ChessSet, game->WhosTurn);
  captureSources = AllPawnCaptureSources(&game->ChessSet, game->WhosTurn);

  // Pushes.
  while(pushSources) {
    from = PopForward(&pushSources);

    pushTargets = PawnPushTargets(&game->ChessSet, game->WhosTurn, POSBOARD(from)) & mask;

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
      } else {
        AppendMove(slice, move);
      }
    }
  }

  // Captures.

  while(captureSources) {
    from = PopForward(&captureSources);

    captureTargets = PawnCaptureTargets(&game->ChessSet, game->WhosTurn, POSBOARD(from)) & mask;

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
          AppendMove(slice, move);
        }
      } else {
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

  while(fromBoard) {
    from = PopForward(&fromBoard);

    move.Piece = Pawn;
    move.From = from;
    move.To = game->EnPassantSquare;
    move.Capture = true;
    move.Type = EnPassant;

    AppendMove(slice, move);
  }
}

static void
pieceMoves(Piece piece, Game *game, MoveSlice *slice, BitBoard mask)
{
  BitBoard captureTargets, moveTargets, pieceBoard;
  Move move;
  Position from, to;

  pieceBoard = game->ChessSet.Sets[game->WhosTurn].Boards[piece];

  move.Piece = piece;
  move.Type = Normal;

  while(pieceBoard) {
    from = PopForward(&pieceBoard);

    moveTargets = GetMoveTargets[piece](&game->ChessSet, game->WhosTurn, POSBOARD(from)) & mask;
    captureTargets = GetCaptureTargets[piece](&game->ChessSet, game->WhosTurn, POSBOARD(from)) & mask;

    move.From = from;

    // Moves.
    move.Capture = false;
    while(moveTargets != EmptyBoard) {
      to = PopForward(&moveTargets);
      move.To = to;

      AppendMove(slice, move);
    }

    // Captures.
    move.Capture = true;
    while(captureTargets != EmptyBoard) {
      to = PopForward(&captureTargets);

      move.To = to;
      AppendMove(slice, move);
    }
  }
}

static void
evasions(MoveSlice *slice, Game *game)
{
  BitBoard attacks, captures, moves, targets;
  BitBoard checks = game->CheckStats.CheckSources;
  BitBoard slideAttacks = EmptyBoard;
  int checkCount = 0;
  Move move;
  Piece piece;
  Position check;
  Position king = game->CheckStats.DefendedKing;
  Side side = game->WhosTurn;
  Side opposite = OPPOSITE(side);

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
          RookAttacksFrom(check, game->ChessSet.Occupancy);
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

  captures = attacks & game->ChessSet.Sets[opposite].Occupancy;

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
  for(piece = Knight; piece < King; piece++) {
    pieceMoves(piece, game, slice, targets);
  }
}

static void
nonEvasions(MoveSlice *slice, Game *game)
{
  Piece piece;

  pawnMoves(game, slice, FullyOccupied);

  for(piece = Knight; piece <= King; piece++) {
    pieceMoves(piece, game, slice, FullyOccupied);
  }

  castleMoves(game, slice);
}
