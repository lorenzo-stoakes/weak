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

void
InitMagics()
{
  long len, shift;
  long bishopCount = 0, rookCount = 0;
  BitBoard bitBoard, currThreats, index, magic, mask, threats;
  Position from, pos;

  for(pos = A1; pos <= H8; pos++) {
    len = 1<<(64 - magicShift[MAGIC_BISHOP][pos]);
    BishopThreatBase[pos] = (BitBoard*)allocateZero(sizeof(BitBoard), len);

    bishopCount += len;

    len = 1<<(64 - magicShift[MAGIC_ROOK][pos]);
    RookThreatBase[pos] = (BitBoard*)allocateZero(sizeof(BitBoard), len);

    rookCount += len;
  }

  for(from = A1; from <= H8; from++) {
    magic = magicBoard[MAGIC_BISHOP][from];
    mask = magicMask[MAGIC_BISHOP][from];
    shift = magicShift[MAGIC_BISHOP][from];

    // Carry-Rippler. See http://chessprogramming.wikispaces.com/Traversing+Subsets+of+a+Set
    bitBoard = EmptyBoard;
    do {
      index = (bitBoard * magic) >> shift;
      threats = CalcBishopSquareThreats(from, bitBoard);
      currThreats = BishopThreatBase[from][index];

      if(currThreats != EmptyBoard && currThreats != threats) {
        panic("Invalid magic for bishops in position %s, index %lu. Have threats:-\n"
              "%s\n"
              "But already have:-\n"
              "%s\n"
              "When looking at permutation:-\n"
              "%s\n",
              StringPosition(from),
              index,
              StringBitBoard(threats),
              StringBitBoard(currThreats),
              StringBitBoard(bitBoard));
      }

      BishopThreatBase[from][index] = threats;

      bishopCount--;

      bitBoard = (bitBoard - mask) & mask;
    } while(bitBoard != EmptyBoard);

    magic = magicBoard[MAGIC_ROOK][from];
    mask = magicMask[MAGIC_ROOK][from];
    shift = magicShift[MAGIC_ROOK][from];

    bitBoard = EmptyBoard;
    do {
      index = (bitBoard * magic) >> shift;

      threats = CalcRookSquareThreats(from, bitBoard);
      currThreats = RookThreatBase[from][index];

      if(currThreats != EmptyBoard && currThreats != threats) {
        panic("Invalid magic for rooks in position %s, index %lu. Have threats:-\n"
              "%s\n"
              "But already have:-\n"
              "%s\n"
              "When looking at permutation:-\n"
              "%s\n",
              StringPosition(from),
              index,
              StringBitBoard(threats),
              StringBitBoard(currThreats),
              StringBitBoard(bitBoard));
      }

      RookThreatBase[from][index] = threats;

      rookCount--;

      bitBoard = (bitBoard - mask) & mask;
    } while(bitBoard != EmptyBoard);
  }

  if(rookCount != 0) {
    panic("Generating magics for rooks out by %d.", rookCount);
  }
  if(bishopCount != 0) {
    panic("Generating magics for bishops out by %d.", bishopCount);
  }
}
