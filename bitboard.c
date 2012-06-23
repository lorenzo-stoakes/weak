#include "weak.h"

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

// northwest    north   northeast
//  NoWe         Nort         NoEa
//          +7    +8    +9
//              \  |  /
//  West    -1 <-  0 -> +1    East
//              /  |  \
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
}
