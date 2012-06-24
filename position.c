#include "weak.h"

static BitBoard nortRays[64], eastRays[64], soutRays[64], westRays[64],
  noeaRays[64], soweRays[64], noweRays[64], soeaRays[64];

void
InitRays(void)
{
  BitBoard nortRay, soutRay, eastRay, aEastRay, westRay, hWestRay, noeaRay1, noeaRay, soweRay8,
    soweRay, noweRay1, noweRay, soeaRay8, soeaRay;

  // Define as ints so they are signed.
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
    for(file = FileH; file >= FileA; file--) {
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

BitBoard
NortRay(Position pos)
{
  return nortRays[pos];
}

BitBoard EastRay(Position pos)
{
  return eastRays[pos];
}

BitBoard SoutRay(Position pos)
{
  return soutRays[pos];
}

BitBoard WestRay(Position pos)
{
  return westRays[pos];
}

BitBoard NoEaRay(Position pos)
{
  return noeaRays[pos];
}

BitBoard NoWeRay(Position pos)
{
  return noweRays[pos]; 
}

BitBoard SoEaRay(Position pos)
{
  return soeaRays[pos];
}

BitBoard SoWeRay(Position pos)
{
  return soweRays[pos];  
}
