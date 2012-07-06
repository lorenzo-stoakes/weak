#include <stdio.h>
#include "weak.h"
#include "magic.h"

void
InitMagics()
{
  long len, shift;
  long bishopCount = 0, rookCount = 0;
  BitBoard bitBoard, index, magic, mask;
  Position from, pos;

  for(pos = A1; pos <= H8; pos++) {
    len = 1<<(64 - magicShift[MAGIC_BISHOP][pos]);
    BishopThreatBase[pos] = (BitBoard*)allocate(sizeof(BitBoard)*len);

    bishopCount += len;

    len = 1<<(64 - magicShift[MAGIC_ROOK][pos]);
    RookThreatBase[pos] = (BitBoard*)allocate(sizeof(BitBoard)*len);

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

      BishopThreatBase[from][index] = BishopSquareThreats(from, bitBoard) & mask;

      bishopCount--;

      bitBoard = (bitBoard - mask) & mask;
    } while(bitBoard != EmptyBoard);

    magic = magicBoard[MAGIC_ROOK][from];
    mask = magicMask[MAGIC_ROOK][from];
    shift = magicShift[MAGIC_ROOK][from];

    bitBoard = EmptyBoard;
    do {
      index = (bitBoard * magic) >> shift;

      RookThreatBase[from][index] = RookSquareThreats(from, bitBoard) & mask;

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
