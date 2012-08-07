#include "weak.h"

static FORCE_INLINE Move* bishopMoves(Position*, Move*, BitBoard, BitBoard);
static Move* castleMoves(Game*, Move*);
static Move* evasions(Move*, Game*);
static FORCE_INLINE Move* kingMoves(Position, Move*, BitBoard);
static FORCE_INLINE Move* knightMoves(Position*, Move*, BitBoard);
static Move* nonEvasions(Move*, Game*);
static Move* pawnMovesBlack(ChessSet*, Position, Move*, BitBoard, bool);
static Move* pawnMovesWhite(Game*, Move*, BitBoard, bool);
static FORCE_INLINE Move* queenMoves(Position*, Move*, BitBoard, BitBoard);
static FORCE_INLINE Move* rookMoves(Position*, Move*, BitBoard, BitBoard);

Move*
AllMoves(Move *start, Game *game)
{
  Move *curr = start, *end = start;

  end = game->CheckStats.CheckSources ? evasions(start, game) : nonEvasions(start, game);

  // Filter out illegal moves.
  while(curr != end) {
    if(!PseudoLegal(game, *curr, game->CheckStats.Pinned)) {
      // Switch last move with the one we are rejecting.
      end--;
      *curr = *end;
    } else {
      curr++;
    }
  }

  return end;
}

static Move*
castleMoves(Game *game, Move *end)
{
  BitBoard attackMask, occupancy, opposition;
  bool good;
  CastleSide castleSide;
  Position king, pos;
  Side side = game->WhosTurn;
  Side opposite;

  king = E1 + side*8*7;

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
        if(castleSide == QueenSide) {
          *end++ = MAKE_MOVE(king, king-2, CastleQueenSide);
        } else {
          *end++ = MAKE_MOVE(king, king-2, CastleKingSide);
        }
      }
    }
  }

  return end;
}

static Move*
evasions(Move *end, Game *game)
{
  BitBoard attacks, moves, targets;
  BitBoard checks = game->CheckStats.CheckSources;
  BitBoard occupancy = game->ChessSet.Occupancy;
  BitBoard slideAttacks = EmptyBoard;
  ChessSet *chessSet = &game->ChessSet;
  int checkCount = 0;
  Piece piece;
  Position check;
  Position king = game->CheckStats.DefendedKing;
  Side side = game->WhosTurn;

  while(checks != EmptyBoard) {
    check = PopForward(&checks);

    checkCount++;

    piece = PieceAt(chessSet, check);

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

  // King evasion moves.

  moves = attacks & ~chessSet->Sets[side].Occupancy;
  while(moves) {
    *end++ = MAKE_MOVE_QUICK(king, PopForward(&moves));
  }

  // If there is more than 1 check, blocking won't achieve anything.
  if(checkCount > 1) {
    return end;
  }

  // Blocking/capturing the checking piece.
  // We use check from the loop above, since we have only 1 check this will
  // be the sole checker.
  targets = Between[check][king] | game->CheckStats.CheckSources;

  if(side == White) {
    end = pawnMovesWhite(game, end, targets, true);
  } else {
    end = pawnMovesBlack(chessSet, game->EnPassantSquare, end, targets, true);
  }

  // King already handled.
  end = knightMoves(chessSet->PiecePositions[side][Knight], end, targets);
  end = bishopMoves(chessSet->PiecePositions[side][Bishop], end, occupancy, targets);
  end = rookMoves  (chessSet->PiecePositions[side][Rook],   end, occupancy, targets);
  end = queenMoves (chessSet->PiecePositions[side][Queen],  end, occupancy, targets);

  return end;
}

static FORCE_INLINE Move*
bishopMoves(Position *positions, Move *end, BitBoard occupancy, BitBoard mask)
{
  BitBoard attacks;
  Position from, to;

  while((from = *positions++) != EmptyPosition) {
    attacks = BishopAttacksFrom(from, occupancy) & mask;
    while(attacks != EmptyBoard) {
      to = PopForward(&attacks);

      *end++ = MAKE_MOVE_QUICK(from, to);
    }
  }

  return end;
}

static FORCE_INLINE Move*
kingMoves(Position from, Move *end, BitBoard mask)
{
  BitBoard attacks;
  Position to;

  attacks = KingAttacksFrom(from) & mask;
  while(attacks != EmptyBoard) {
    to = PopForward(&attacks);

    *end++ = MAKE_MOVE_QUICK(from, to);
  }

  return end;
}

static FORCE_INLINE Move*
knightMoves(Position *positions, Move *end, BitBoard mask)
{
  BitBoard attacks;
  Position from, to;

  if(*positions != EmptyPosition) {
    do {
      from = *positions;

      attacks = KnightAttacksFrom(from) & mask;
      while(attacks != EmptyBoard) {
        to = PopForward(&attacks);

        *end++ = MAKE_MOVE_QUICK(from, to);
      }
    } while(*++positions != EmptyPosition);
  }

  return end;
}

static Move*
nonEvasions(Move *end, Game *game)
{
  Side side = game->WhosTurn;
  ChessSet *chessSet = &game->ChessSet;
  BitBoard occupancy =  chessSet->Occupancy;
  BitBoard attackable = ~(chessSet->Sets[side].Occupancy);

  if(side == White) {
    end = pawnMovesWhite(game, end, attackable, false);
  } else {
    end = pawnMovesBlack(chessSet, game->EnPassantSquare, end, attackable, false);
  }

  end = knightMoves(chessSet->PiecePositions[side][Knight], end, attackable);
  end = bishopMoves(chessSet->PiecePositions[side][Bishop], end, occupancy, attackable);
  end =   rookMoves(chessSet->PiecePositions[side][Rook], end, occupancy, attackable);
  end =  queenMoves(chessSet->PiecePositions[side][Queen], end, occupancy, attackable);
  end =   kingMoves(game->CheckStats.DefendedKing, end, attackable);

  end = castleMoves(game, end);

  return end;
}

static Move*
pawnMovesBlack(ChessSet *chessSet, Position enPassant, Move *curr, BitBoard mask, bool evasion)
{
  BitBoard empty = chessSet->EmptySquares;
  BitBoard opposition = chessSet->Sets[White].Occupancy & mask;

  BitBoard bitBoard1, bitBoard2;

  BitBoard pawns = chessSet->Sets[Black].Boards[Pawn];

  BitBoard pawnsOn2 = pawns&Rank2Mask;
  BitBoard pawnsNotOn2 = pawns&NotRank2Mask;

  Position to;

  // Moves.

  bitBoard1 = SoutOne(pawnsNotOn2) & empty;
  bitBoard2 = SoutOne(bitBoard1 & Rank6Mask) & empty & mask;

  // Do this separately, as for double pushes we don't care about target squares in the region
  // we're pushing *through*.
  bitBoard1 &= mask;

  while(bitBoard1) {
    to = PopForward(&bitBoard1);

    *curr++ = MAKE_MOVE_QUICK(to+8, to);
  }

  while(bitBoard2) {
    to = PopForward(&bitBoard2);

    *curr++ = MAKE_MOVE_QUICK(to+16, to);
  }

  // Promotions.
  if(pawnsOn2 != EmptyBoard && (mask & Rank1Mask) != EmptyBoard) {
    bitBoard1 = SoutOne(pawnsOn2) & empty & mask;

    while(bitBoard1) {
      to = PopForward(&bitBoard1);

      *curr++ = MAKE_MOVE(to+8, to, PromoteBishop);
      *curr++ = MAKE_MOVE(to+8, to, PromoteKnight);
      *curr++ = MAKE_MOVE(to+8, to, PromoteRook);
      *curr++ = MAKE_MOVE(to+8, to, PromoteQueen);
    }

    bitBoard1 = SoWeOne(pawnsOn2) & opposition;
    while(bitBoard1) {
      to = PopForward(&bitBoard1);

      *curr++ = MAKE_MOVE(to+9, to, PromoteBishop);
      *curr++ = MAKE_MOVE(to+9, to, PromoteKnight);
      *curr++ = MAKE_MOVE(to+9, to, PromoteRook);
      *curr++ = MAKE_MOVE(to+9, to, PromoteQueen);
    }

    bitBoard2 = SoEaOne(pawnsOn2) & opposition;
    while(bitBoard2) {
      to = PopForward(&bitBoard2);

      *curr++ = MAKE_MOVE(to+7, to, PromoteBishop);
      *curr++ = MAKE_MOVE(to+7, to, PromoteKnight);
      *curr++ = MAKE_MOVE(to+7, to, PromoteRook);
      *curr++ = MAKE_MOVE(to+7, to, PromoteQueen);
    }
  }

  // Captures.

  bitBoard1 = SoWeOne(pawnsNotOn2) & opposition;
  while(bitBoard1) {
    to = PopForward(&bitBoard1);

    *curr++ = MAKE_MOVE_QUICK(to+9, to);
  }

  bitBoard2 = SoEaOne(pawnsNotOn2) & opposition;
  while(bitBoard2) {
    to = PopForward(&bitBoard2);

    *curr++ = MAKE_MOVE_QUICK(to+7, to);
  }

  // En passant.
  if(enPassant != EmptyPosition) {
    if(evasion && (mask & NortOne(POSBOARD(enPassant))) == EmptyBoard) {
      return curr;
    }

    bitBoard1 = pawnsNotOn2 & PawnAttacksFrom(enPassant, White);

    while(bitBoard1) {
      *curr++ = MAKE_MOVE(PopForward(&bitBoard1), enPassant, EnPassant);
    }
  }

  return curr;
}

static Move*
pawnMovesWhite(Game *game, Move *curr, BitBoard mask, bool evasion)
{
  ChessSet *chessSet = &game->ChessSet;

  BitBoard empty = chessSet->EmptySquares;
  BitBoard opposition = chessSet->Sets[Black].Occupancy & mask;

  BitBoard bitBoard1, bitBoard2;

  BitBoard pawns = chessSet->Sets[White].Boards[Pawn];

  BitBoard pawnsOn7 = pawns&Rank7Mask;
  BitBoard pawnsNotOn7 = pawns&NotRank7Mask;

  Position enPassant = game->EnPassantSquare;
  Position to;

  // Moves.

  bitBoard1 = NortOne(pawnsNotOn7) & empty;
  bitBoard2 = NortOne(bitBoard1 & Rank3Mask) & empty & mask;

  // Do this separately, as for double pushes we don't care about target squares in the region
  // we're pushing *through*.  
  bitBoard1 &= mask;

  while(bitBoard1) {
    to = PopForward(&bitBoard1);

    *curr++ = MAKE_MOVE_QUICK(to-8, to);
  }

  while(bitBoard2) {
    to = PopForward(&bitBoard2);

    *curr++ = MAKE_MOVE_QUICK(to-16, to);
  }

  // Promotions.
  if(pawnsOn7 != EmptyBoard && (mask & Rank8Mask) != EmptyBoard) {
    bitBoard1 = NortOne(pawnsOn7) & empty & mask;

    while(bitBoard1) {
      to = PopForward(&bitBoard1);

      *curr++ = MAKE_MOVE(to-8, to, PromoteBishop);
      *curr++ = MAKE_MOVE(to-8, to, PromoteKnight);
      *curr++ = MAKE_MOVE(to-8, to, PromoteRook);
      *curr++ = MAKE_MOVE(to-8, to, PromoteQueen);
    }

    bitBoard1 = NoWeOne(pawnsOn7) & opposition;
    while(bitBoard1) {
      to = PopForward(&bitBoard1);

      *curr++ = MAKE_MOVE(to-7, to, PromoteBishop);
      *curr++ = MAKE_MOVE(to-7, to, PromoteKnight);
      *curr++ = MAKE_MOVE(to-7, to, PromoteRook);
      *curr++ = MAKE_MOVE(to-7, to, PromoteQueen);
    }

    bitBoard2 = NoEaOne(pawnsOn7) & opposition;
    while(bitBoard2) {
      to = PopForward(&bitBoard2);

      *curr++ = MAKE_MOVE(to-9, to, PromoteBishop);
      *curr++ = MAKE_MOVE(to-9, to, PromoteKnight);
      *curr++ = MAKE_MOVE(to-9, to, PromoteRook);
      *curr++ = MAKE_MOVE(to-9, to, PromoteQueen);
    }
  }

  // Captures.

  bitBoard1 = NoWeOne(pawnsNotOn7) & opposition;
  while(bitBoard1) {
    to = PopForward(&bitBoard1);

    *curr++ = MAKE_MOVE_QUICK(to-7, to);
  }

  bitBoard2 = NoEaOne(pawnsNotOn7) & opposition;
  while(bitBoard2) {
    to = PopForward(&bitBoard2);

    *curr++ = MAKE_MOVE_QUICK(to-9, to);
  }

  // En passant.
  if(enPassant != EmptyPosition) {
    if(evasion && (mask & SoutOne(POSBOARD(enPassant))) == EmptyBoard) {
      return curr;
    }

    bitBoard1 = pawnsNotOn7 & PawnAttacksFrom(enPassant, Black);

    while(bitBoard1) {
      *curr++ = MAKE_MOVE(PopForward(&bitBoard1), enPassant, EnPassant);
    }
  }

  return curr;
}

static FORCE_INLINE Move*
queenMoves(Position *positions, Move *end, BitBoard occupancy, BitBoard mask)
{
  BitBoard attacks;
  Position from, to;

  if(*positions != EmptyPosition) {
    do {
      from = *positions;
      attacks = (RookAttacksFrom(from, occupancy) |
                 BishopAttacksFrom(from, occupancy)) & mask;
      while(attacks != EmptyBoard) {
        to = PopForward(&attacks);

        *end++ = MAKE_MOVE_QUICK(from, to);
      }
    } while(*++positions != EmptyPosition);
  }

  return end;
}

static FORCE_INLINE Move*
rookMoves(Position *positions, Move *end, BitBoard occupancy, BitBoard mask)
{
  BitBoard attacks;
  Position from, to;

  while((from = *positions++) != EmptyPosition) {
    attacks = RookAttacksFrom(from, occupancy) & mask;
    while(attacks != EmptyBoard) {
      to = PopForward(&attacks);

      *end++ = MAKE_MOVE_QUICK(from, to);
    }
  }

  return end;
}
