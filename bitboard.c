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

// Used in BitScanForward.
const Position deBruijnLookup[64] = {
  63, 0, 58, 1, 59, 47, 53, 2,
  60, 39, 48, 27, 54, 33, 42, 3,
  61, 51, 37, 40, 49, 18, 28, 20,
  55, 30, 34, 11, 43, 14, 22, 4,
  62, 57, 46, 52, 38, 26, 32, 41,
  50, 36, 17, 19, 29, 10, 13, 21,
  56, 45, 25, 31, 35, 16, 9, 12,
  44, 24, 15, 8, 23, 7, 6, 5,
};

// Used in BitScanBackward.
const int bitBackward8[256] = {
  -1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
};

// Are all the positions along the same line?
bool
Aligned(Position pos1, Position pos2, Position pos3)
{
  return ((Between[pos1][pos2] | Between[pos1][pos3] | Between[pos2][pos3]) &
          (POSBOARD(pos1) | POSBOARD(pos2) | POSBOARD(pos3)));
}

#ifndef USE_BITSCAN_ASM
Position
BitScanBackward(BitBoard bitBoard)
{
  int offset = 0;

  // See [10].

  if(bitBoard == EmptyBoard) {
    panic("BitScanBackward attempted on empty BitBoard.");
  }

  if(bitBoard > C64(0xffffffff)) {
    bitBoard >>= 32;
    offset = 32;
  }
  if(bitBoard > C64(0xffff)) {
    bitBoard >>= 16;
    offset += 16;
  }
  if(bitBoard > C64(0xff)) {
    bitBoard >>= 8;
    offset += 8;
  }

  return offset + bitBackward8[bitBoard];
}

// Determine the position of the first non-zero least significant bit.
Position
BitScanForward(BitBoard bitBoard)
{
  // Uses De Bruijn multiplication.
  // See [9].

  const BitBoard debruijn64 = C64(0x07EDD5E59A4E28C2);
  BitBoard isolated, multiple;
  int index;

  if(bitBoard == EmptyBoard) {
    panic("BitScanForward attempted on empty BitBoard.");
  }

  isolated = bitBoard & -bitBoard;
  multiple = isolated * debruijn64;
  index = multiple >> 58;

  return deBruijnLookup[index];
}
#endif

// Flip bitboard vertically (about the horizontal axis).
BitBoard
FlipVertical(BitBoard bitBoard)
{
  // Flipping about the horizontal rank [4].

  return (bitBoard << 56) |
    ((bitBoard << 40) & Rank7Mask) |
    ((bitBoard << 24) & Rank6Mask) |
    ((bitBoard << 8) & Rank5Mask) |
    ((bitBoard >> 8) & Rank4Mask) |
    ((bitBoard >> 24) & Rank3Mask) |
    ((bitBoard >> 40) & Rank2Mask) |
    (bitBoard >> 56);
}

// Flip bitboard diagonally about the A1-H8 diagonal.
BitBoard
FlipDiagA1H8(BitBoard bitBoard)
{
  // Flipping across A1-H8 [5].

  BitBoard temp;
  const BitBoard
    k1 = C64(0x5500550055005500),
    k2 = C64(0x3333000033330000),
    k4 = C64(0x0f0f0f0f00000000);

  temp = k4 & (bitBoard ^ (bitBoard << 28));
  bitBoard ^= temp ^ (temp >> 28);
  temp = k2 & (bitBoard ^ (bitBoard << 14));
  bitBoard ^= temp ^ (temp >> 14);
  temp = k1 & (bitBoard ^ (bitBoard << 7));
  bitBoard ^= temp ^ (temp >> 7);
  return bitBoard;
}

void
InitRays(void)
{
  BitBoard nortRay, soutRay, eastRay, aEastRay, westRay, hWestRay, noeaRay1, noeaRay, soweRay8,
    soweRay, noweRay1, noweRay, soeaRay8, soeaRay;

  // Define as ints so they are signed, and we don't fall off the end when subtracting...!
  int file, pos, rank;

  nortRay = C64(0x0101010101010100);
  for(pos = A1; pos <= H8; pos++) {
    nortRays[pos] = nortRay;
    nortRay <<= 1;
  }

  soutRay = C64(0x0080808080808080);
  for(pos = H8; pos >= A1; pos--) {
    soutRays[pos] = soutRay;
    soutRay >>= 1;
  }

  aEastRay = C64(0xfe);
  for(rank = Rank1; rank <= Rank8; rank++) {
    eastRay = aEastRay;
    for(file = FileA; file <= FileH; file++) {
      pos = POSITION(rank, file);
      eastRays[pos] = eastRay & aEastRay;
      eastRay <<= 1;
    }
    aEastRay <<= 8;
  }

  hWestRay = C64(0x7f00000000000000);
  for(rank = Rank8; rank >= Rank1; rank--) {
    westRay = hWestRay;
    for(file = FileH; file >= FileA; file--) {
      pos = POSITION(rank, file);
      westRays[pos] = westRay & hWestRay;
      westRay >>= 1;
    }
    hWestRay >>= 8;
  }

  noeaRay1 = C64(0x8040201008040200);
  for(file = FileA; file <= FileH; file++) {
    noeaRay = noeaRay1;
    for(rank = Rank1; rank <= Rank8; rank++) {
      pos = POSITION(rank, file);
      noeaRays[pos] = noeaRay;
      noeaRay <<= 8;
    }
    // Use EastOne() for the wrapping.
    noeaRay1 = EastOne(noeaRay1);
  }

  soweRay8 = C64(0x0040201008040201);
  for(file = FileH; file >= FileA; file--) {
    soweRay = soweRay8;
    for(rank = Rank8; rank >= Rank1; rank--) {
      pos = POSITION(rank, file);
      soweRays[pos] = soweRay;
      soweRay >>= 8;
    }
    // Use WestOne() for the wrapping.
    soweRay8 = WestOne(soweRay8);
  }

  noweRay1 = C64(0x0102040810204000);
  for(file = FileH; file >= FileA; file--) {
    noweRay = noweRay1;
    for(rank = Rank1; rank <= Rank8; rank++) {
      pos = POSITION(rank, file);
      noweRays[pos] = noweRay;
      noweRay <<= 8;
    }
    // Use WestOne() for the wrapping.
    noweRay1 = WestOne(noweRay1);
  }

  soeaRay8 = C64(0x0002040810204080);
  for(file = FileA; file <= FileH; file++) {
    soeaRay = soeaRay8;
    for(rank = Rank8; rank >= Rank1; rank--) {
      pos = POSITION(rank, file);
      soeaRays[pos] = soeaRay;
      soeaRay >>= 8;
    }
    // Use EastOne() for the wrapping.
    soeaRay8 = EastOne(soeaRay8);
  }
}

// Count the number of bits in the specified BitBoard.
int
PopCount(BitBoard x)
{
  // 'SWAR' population count.

 const BitBoard
   k1 = C64(0x5555555555555555),
   k2 = C64(0x3333333333333333),
   k4 = C64(0x0f0f0f0f0f0f0f0f),
   kf = C64(0x0101010101010101);

  // Put count of each 2 bits into those 2 bits.
  x = x - ((x >> 1) & k1);
  // Put count of each 4 bits into those 4 bits.
  x = (x & k2) + ((x >> 2) & k2);
  // Pu count of each 8 bits into those 8 bits.
  x = (x + (x >> 4)) & k4;
  // Returns 8 most significant bits of x + (x<<8) + (x<<16) + (x<<24) + ...
  x = (x * kf) >> 56;

  return (int)x;
}

// Is the specified position occupied in the specified BitBoard?
bool
PositionOccupied(BitBoard bitBoard, Position pos)
{
  return (bitBoard & POSBOARD(pos)) != EmptyBoard;
}

// Rotate bitboard 90 degrees anticlockwise.
BitBoard
Rotate90AntiClockwise(BitBoard bitBoard)
{
  // Flip vertically, then across A1-H8 diagonal [6].

  bitBoard = FlipVertical(bitBoard);
  return FlipDiagA1H8(bitBoard);
}

// Rotate bitboard 90 degrees clockwise.
BitBoard
Rotate90Clockwise(BitBoard bitBoard)
{
  // Rotate 90 degrees clockwise [7].

  // Flip across A1-H8 diagonal, flip vertical.
  bitBoard = FlipDiagA1H8(bitBoard);
  return FlipVertical(bitBoard);
}
