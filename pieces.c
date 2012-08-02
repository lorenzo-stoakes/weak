#include "weak.h"
#include "magic.h"

static BitBoard kingSquares[64];
static BitBoard knightSquares[64];
static BitBoard pawnSquares[2][64];

static FORCE_INLINE BitBoard bishopMagicSquareThreats(Position, BitBoard);
static FORCE_INLINE BitBoard rookMagicSquareThreats(Position, BitBoard);

BitBoard
BishopAttacksFrom(Position bishop, BitBoard occupancy)
{
  return bishopMagicSquareThreats(bishop, occupancy);
}

BitBoard
BishopQueenAttackersTo(ChessSet *chessSet, Position to, BitBoard occupancy)
{
  return (chessSet->PieceOccupancy[Bishop] | chessSet->PieceOccupancy[Queen]) &
    bishopMagicSquareThreats(to, occupancy);
}

BitBoard
BishopSquareThreats(Position bishop, BitBoard occupancy)
{
  BitBoard blockers, noea, nowe, soea, sowe;
  BitBoard ret = EmptyBoard;
  Position blocker;

  noea = NoEaRay(bishop);
  blockers = noea & occupancy;
  if(blockers != EmptyBoard) {
    blocker = BitScanForward(blockers);
    noea &= ~NoEaRay(blocker);
  }
  ret |= noea;

  nowe = NoWeRay(bishop);
  blockers = nowe & occupancy;
  if(blockers != EmptyBoard) {
    blocker = BitScanForward(blockers);
    nowe &= ~NoWeRay(blocker);
  }
  ret |= nowe;

  soea = SoEaRay(bishop);
  blockers = soea & occupancy;
  if(blockers != EmptyBoard) {
    blocker = BitScanBackward(blockers);
    soea &= ~SoEaRay(blocker);
  }
  ret |= soea;

  sowe = SoWeRay(bishop);
  blockers = sowe & occupancy;
  if(blockers != EmptyBoard) {
    blocker = BitScanBackward(blockers);
    sowe &= ~SoWeRay(blocker);
  }
  ret |= sowe;

  return ret;
}

void
InitKing()
{
  BitBoard bitBoard, king;
  Position pos;

  for(pos = A1; pos <= H8; pos++) {
    king = POSBOARD(pos);

    bitBoard = EastOne(king) | WestOne(king);
    king |= bitBoard;
    bitBoard |= NortOne(king) | SoutOne(king);

    kingSquares[pos] = bitBoard;
  }
}

void
InitKnight()
{
  BitBoard attacks, bitBoard, east, west;
  Position pos;

  for(pos = A1; pos <= H8; pos++) {
    bitBoard = POSBOARD(pos);

    // ..x.x..
    // .x...x.
    // ...N...
    // .x...x.
    // ..x.x..

    east = EastOne(bitBoard);
    west = WestOne(bitBoard);

    // ..x.x..
    // .x...x.
    // ..*N*..
    // .x...x.
    // ..x.x..

    attacks = (east | west) << 16;
    attacks |= (east | west) >> 16;

    // ..*.*..
    // .x...x.
    // ...N...
    // .x...x.
    // ..*.*..

    east = EastOne(east);
    west = WestOne(west);

    // ..*.*..
    // .x...x.
    // .*.N.*.
    // .x...x.
    // ..*.*..

    attacks |= (east | west) << 8;
    attacks |= (east | west) >> 8;

    // ..*.*..
    // .*...*.
    // ...N...
    // .*...*.
    // ..*.*..

    knightSquares[pos] = attacks;
  }
}

void
InitPawn()
{
  BitBoard bitBoard;
  Position pos;

  for(pos = A1; pos <= H8; pos++) {
    bitBoard = POSBOARD(pos);

    pawnSquares[White][pos] = NoWeOne(bitBoard) | NoEaOne(bitBoard);
    pawnSquares[Black][pos] = SoWeOne(bitBoard) | SoEaOne(bitBoard);
  }
}

BitBoard
KingAttackersTo(ChessSet *chessSet, Position to)
{
  return chessSet->PieceOccupancy[King] & kingSquares[to];
}

BitBoard
KingAttacksFrom(Position king)
{
  return kingSquares[king];
}

BitBoard
KnightAttackersTo(ChessSet *chessSet, Position to)
{
  return chessSet->PieceOccupancy[Knight] & knightSquares[to];
}

BitBoard
KnightAttacksFrom(Position knight)
{
  return knightSquares[knight];
}

BitBoard
PawnAttackersTo(ChessSet *chessSet, Position to)
{
  return (chessSet->Sets[White].Boards[Pawn] & pawnSquares[Black][to]) |
    (chessSet->Sets[Black].Boards[Pawn] & pawnSquares[White][to]);
}

BitBoard
PawnAttacksFrom(Position pawn, Side side)
{
  return pawnSquares[side][pawn];
}

BitBoard
RookAttacksFrom(Position rook, BitBoard occupancy)
{
  return rookMagicSquareThreats(rook, occupancy);
}

BitBoard
RookQueenAttackersTo(ChessSet *chessSet, Position to, BitBoard occupancy)
{
  return (chessSet->PieceOccupancy[Rook] | chessSet->PieceOccupancy[Queen]) &
    rookMagicSquareThreats(to, occupancy);
}

// Get rook threats from the specified square and occupancy.
BitBoard
RookSquareThreats(Position rook, BitBoard occupancy)
{
  BitBoard blockers, east, nort, sout, west;
  BitBoard ret = EmptyBoard;
  Position blocker;

  nort = NortRay(rook);
  blockers = nort & occupancy;
  if(blockers != EmptyBoard) {
    blocker = BitScanForward(blockers);
    nort &= ~NortRay(blocker);
  }
  ret |= nort;

  east = EastRay(rook);
  blockers = east & occupancy;
  if(blockers != EmptyBoard) {
    blocker = BitScanForward(blockers);
    east &= ~EastRay(blocker);
  }
  ret |= east;

  sout = SoutRay(rook);
  blockers = sout & occupancy;
  if(blockers != EmptyBoard) {
    blocker = BitScanBackward(blockers);
    sout &= ~SoutRay(blocker);
  }
  ret |= sout;

  west = WestRay(rook);
  blockers = west & occupancy;
  if(blockers != EmptyBoard) {
    blocker = BitScanBackward(blockers);
    west &= ~WestRay(blocker);
  }
  ret |= west;

  return ret;
}

static FORCE_INLINE BitBoard
bishopMagicSquareThreats(Position bishop, BitBoard occupancy) {
  BitBoard magic = magicBoard[MAGIC_BISHOP][bishop];
  BitBoard mask = magicMask[MAGIC_BISHOP][bishop];
  int shift = magicShift[MAGIC_BISHOP][bishop];
  BitBoard index = (magic*(occupancy&mask))>>shift;

  return BishopThreatBase[bishop][index];
}

static FORCE_INLINE BitBoard
rookMagicSquareThreats(Position rook, BitBoard occupancy) {
  BitBoard magic = magicBoard[MAGIC_ROOK][rook];
  BitBoard mask = magicMask[MAGIC_ROOK][rook];
  int shift = magicShift[MAGIC_ROOK][rook];
  BitBoard index = (magic*(occupancy&mask))>>shift;

  return RookThreatBase[rook][index];
}
