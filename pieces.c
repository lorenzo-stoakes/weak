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

#include "weak.h"
#include "magic.h"

static BitBoard kingSquares[64];
static BitBoard knightSquares[64];
static BitBoard pawnSquares[2][64];

static FORCE_INLINE BitBoard bishopMagicSquareThreats(Position, BitBoard);
static FORCE_INLINE BitBoard rookMagicSquareThreats(Position, BitBoard);

static FORCE_INLINE BitBoard bishopQueenAttackersTo(ChessSet*, Position, BitBoard);
static FORCE_INLINE BitBoard kingAttackersTo(ChessSet*, Position);
static FORCE_INLINE BitBoard knightAttackersTo(ChessSet *chessSet, Position to);
static FORCE_INLINE BitBoard pawnAttackersTo(ChessSet*, Position);
static FORCE_INLINE BitBoard rookQueenAttackersTo(ChessSet*, Position, BitBoard);

BitBoard
AllAttackersTo(ChessSet *chessSet, Position pos, BitBoard occupancy)
{
  return pawnAttackersTo(chessSet, pos) | knightAttackersTo(chessSet, pos) |
    bishopQueenAttackersTo(chessSet, pos, occupancy) |
    rookQueenAttackersTo(chessSet, pos, occupancy) |
    kingAttackersTo(chessSet, pos);
}

BitBoard
BishopAttacksFrom(Position bishop, BitBoard occupancy)
{
  return bishopMagicSquareThreats(bishop, occupancy);
}

BitBoard
CalcBishopSquareThreats(Position bishop, BitBoard occupancy)
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
KingAttacksFrom(Position king)
{
  return kingSquares[king];
}

BitBoard
KnightAttacksFrom(Position knight)
{
  return knightSquares[knight];
}

BitBoard
PawnAttacksFrom(Position pawn, Side side)
{
  return pawnSquares[side][pawn];
}

// Get rook threats from the specified square and occupancy.
BitBoard
CalcRookSquareThreats(Position rook, BitBoard occupancy)
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

static FORCE_INLINE BitBoard
bishopQueenAttackersTo(ChessSet *chessSet, Position to, BitBoard occupancy)
{
  return (chessSet->PieceOccupancy[Bishop] | chessSet->PieceOccupancy[Queen]) &
    bishopMagicSquareThreats(to, occupancy);
}

static FORCE_INLINE BitBoard
kingAttackersTo(ChessSet *chessSet, Position to)
{
  return chessSet->PieceOccupancy[King] & kingSquares[to];
}

static FORCE_INLINE BitBoard
knightAttackersTo(ChessSet *chessSet, Position to)
{
  return chessSet->PieceOccupancy[Knight] & knightSquares[to];
}

static FORCE_INLINE BitBoard
pawnAttackersTo(ChessSet *chessSet, Position to)
{
  return (chessSet->Sets[White].Boards[Pawn] & pawnSquares[Black][to]) |
    (chessSet->Sets[Black].Boards[Pawn] & pawnSquares[White][to]);
}

static FORCE_INLINE BitBoard
rookQueenAttackersTo(ChessSet *chessSet, Position to, BitBoard occupancy)
{
  return (chessSet->PieceOccupancy[Rook] | chessSet->PieceOccupancy[Queen]) &
    rookMagicSquareThreats(to, occupancy);
}
