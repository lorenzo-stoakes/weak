#include "weak.h"

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


static void
castleMoves(Game *game, MoveSlice *ret)
{
  BitBoard initKing, initRooks;
  bool doQueenSide = true, doKingSide = true;
  Move kingSide, queenSide;
  Position king;
  Side side = game->WhosTurn;
  // TODO: Add further checks, reduce duplication between this + castleLegal.

  king = E1 + side*8*7;

  // Add unneeded fields to prevent garbage in unassigned fields. TODO: Review
  kingSide.Capture = false;
  kingSide.From = king;
  kingSide.Piece = King;
  kingSide.Type = CastleKingSide;
  kingSide.To = king + 2;

  queenSide.Capture = false;
  queenSide.From = king;
  queenSide.Piece = King;
  queenSide.Type = CastleQueenSide;
  queenSide.To = king - 2;

  switch(side) {
  case White:
    initKing = InitWhiteKing;
    initRooks = InitWhiteRooks;

    break;
  case Black:
    initKing = InitBlackKing;
    initRooks = InitBlackRooks;

    break;
  default:
    panic("Unrecognised side %d.", game->WhosTurn);
  }

  // Some simple checks, before taking the expensive route.

  // If the king has moved, or both rooks aren't present, we can't castle.
  // TODO: Ridiculously coarse test, improve!
  if((game->ChessSet.Occupancy & initKing) == EmptyBoard ||
     (game->ChessSet.Occupancy & initRooks) == EmptyBoard) {
    return;
  }

  // If we are obstructed, or don't have appropriate rights, can't castle.
  if((game->ChessSet.Occupancy & CastlingMasks[side][KingSide]) != EmptyBoard ||
     !game->CastlingRights[side][KingSide]) {
    doKingSide = false;
  }
  if((game->ChessSet.Occupancy & CastlingMasks[side][QueenSide]) != EmptyBoard ||
     !game->CastlingRights[side][QueenSide]) {
    doQueenSide = false;
  }

  if(doKingSide && Legal(game, &kingSide)) {
      AppendMove(ret, kingSide);
  }

  if(doQueenSide && Legal(game, &queenSide)) {
    AppendMove(ret, queenSide);
  }
}

static void
pawnMoves(Game *game, BitBoard kingThreats, MoveSlice *slice)
{
  int k;
  BitBoard pushSources, captureSources, pushTargets, captureTargets, fromBoard, toBoard;
  Move move;
  MoveType promotions[] = {PromoteKnight, PromoteBishop, PromoteRook, PromoteQueen};
  Position from, to;
  Rank promotionRank = game->WhosTurn == White ? Rank8 : Rank1;

  pushSources = AllPawnPushSources(&game->ChessSet, game->WhosTurn);
  captureSources = AllPawnCaptureSources(&game->ChessSet, game->WhosTurn);

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
          if(!ExposesCheck(game, kingThreats, &move)) {
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

  if(game->EnPassantSquare == EmptyPosition) {
    return;
  }

  toBoard = POSBOARD(game->EnPassantSquare);

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

    if(!ExposesCheck(game, kingThreats, &move)) {
      AppendMove(slice, move);
    }
  }
}

static void
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
