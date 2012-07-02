#include <strings.h>
#include "weak.h"

// See http://chessprogramming.wikispaces.com/BitScan#bsfbsr
#ifdef _MSC_VER
    #include <intrin.h>
    #ifdef _WIN64
        #pragma intrinsic(_BitScanForward64)
        #pragma intrinsic(_BitScanReverse64)
        #define USING_INTRINSICS
    #endif
#elif defined(__GNUC__) && defined(__LP64__)
    static inline unsigned char _BitScanForward64(unsigned int* const Index,
                                                  const BitBoard Mask)
    {
        BitBoard Ret;
        __asm__
        (
            "bsfq %[Mask], %[Ret]"
            :[Ret] "=r" (Ret)
            :[Mask] "mr" (Mask)
        );
        *Index = (unsigned int)Ret;
        return Mask?1:0;
    }
    static inline unsigned char _BitScanReverse64(unsigned int* const Index,
                                                  const BitBoard Mask)
    {
        BitBoard Ret;
        __asm__
        (
            "bsrq %[Mask], %[Ret]"
            :[Ret] "=r" (Ret)
            :[Mask] "mr" (Mask)
        );
        *Index = (unsigned int)Ret;
        return Mask?1:0;
    }
    #define USING_INTRINSICS
#endif

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

Position
BitScanBackward(BitBoard bitBoard)
{
  // See [10].

  if(bitBoard == EmptyBoard) {
    panic("BitScanBackward attempted on empty BitBoard.");
  }

#if defined(__LP64__)
  Position ret;

  _BitScanReverse64(&ret, bitBoard);

  return ret;
#else

  int offset = 0;

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
#endif
}

// Determine the position of the first non-zero least significant bit.
Position
BitScanForward(BitBoard bitBoard)
{
  // See [9].

#if defined(__LP64__)
  Position ret;
  _BitScanForward64(&ret, bitBoard);
  return ret;
#else
  const BitBoard debruijn64 = C64(0x07EDD5E59A4E28C2);
  BitBoard isolated, multiple;
  int index;

	// Uses De Bruijn multiplication.

  if(bitBoard == EmptyBoard) {
    panic("BitScanForward attempted on empty BitBoard.");
  }

  isolated = bitBoard & -bitBoard;
  multiple = isolated * debruijn64;
  index = multiple >> 58;

  return deBruijnLookup[index];
#endif
}

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
  return (bitBoard & POSBOARD(pos)) != 0;
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

// northwest    north   northeast
//  NoWe         Nort         NoEa
//          +7    +8    +9
//              \  |  /
//  West    -1 <-  0 -> +1    East
//              /  |  \.
//          -9    -8    -7
//  SoWe         Sout         SoEa
//  southwest    south   southeast

BitBoard
SoutOne(BitBoard bitBoard)
{
  return bitBoard >> 8;
}

BitBoard
NortOne(BitBoard bitBoard)
{
  return bitBoard << 8;
}

BitBoard
WestOne(BitBoard bitBoard)
{
  return (bitBoard & NotFileAMask) >> 1;
}

BitBoard
EastOne(BitBoard bitBoard)
{
  return (bitBoard & NotFileHMask) << 1;
}

BitBoard
NoEaOne(BitBoard bitBoard)
{
  return (bitBoard & NotFileHMask) << 9;
}

BitBoard
SoEaOne(BitBoard bitBoard)
{
  return (bitBoard & NotFileHMask) >> 7;
}

BitBoard
SoWeOne(BitBoard bitBoard)
{
  return (bitBoard & NotFileAMask) >> 9;
}

BitBoard
NoWeOne(BitBoard bitBoard)
{
  return (bitBoard & NotFileAMask) << 7;
}
