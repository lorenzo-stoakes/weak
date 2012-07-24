#include "weak.h"

static void castleMoves(Game*, MoveSlice*);
static void pawnMoves(Game*, MoveSlice*, BitBoard);
static void pieceMoves(Piece, Game*, MoveSlice*, BitBoard);

// Get all valid moves for the current player.
void
AllMoves(MoveSlice *slice, Game *game)
{
  BitBoard kingThreats;
  Move *curr;
  Piece piece;

  kingThreats = KingThreats(&game->ChessSet, OPPOSITE(game->WhosTurn));  

  // Get all psuedolegal moves.  

  pawnMoves(game, slice);
  for(piece = Knight; piece <= King; piece++) {
    pieceMoves(piece, game, slice);
  }
  castleMoves(game, kingThreats, slice);

  // Filter out illegal moves.
  for(curr = slice->Vals; curr < slice->Curr;) {
    if(ExposesCheck(game, kingThreats, curr)) {
      slice->Curr--;      
      *curr = *slice->Curr;
    } else {
      curr++;
    }
  }
}

static void
castleMoves(Game *game, BitBoard kingThreats, MoveSlice *ret)
{
  CastleSide castleSide;
  Move move;
  Position king;
  Side side = game->WhosTurn;

  king = E1 + side*8*7;

  // Add unneeded fields to prevent garbage in unassigned fields. TODO: Review
  move.Capture = false;
  move.From = king;
  move.Piece = King;
  move.To = king - 2;

  for(castleSide = KingSide; castleSide <= QueenSide; castleSide++) {
    if(game->CastlingRights[side][castleSide] &&
       (game->ChessSet.Occupancy&CastlingMasks[side][castleSide]) == EmptyBoard &&
       (kingThreats&(POSBOARD(king)|CastlingAttackMasks[side][castleSide])) == EmptyBoard) {       
      move.Type = castleSide == KingSide ? CastleKingSide : CastleQueenSide;
      AppendMove(ret, move);
    }
  }
}

static void
pawnMoves(Game *game, MoveSlice *slice)
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
pieceMoves(Piece piece, Game *game, MoveSlice *ret)
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

      AppendMove(ret, move);
    }

    // Captures.
    move.Capture = true;
    while(captureTargets) {
      to = PopForward(&captureTargets);

      move.To = to;
      AppendMove(ret, move);
    }
  }
}
