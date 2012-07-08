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
      threats = BishopSquareThreats(from, bitBoard);
      currThreats = BishopThreatBase[from][index];

      if(currThreats != EmptyBoard && currThreats != threats) {
        panic("Invalid magic for bishops in position %s, index %llu. Have threats:-\n"
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

      threats = RookSquareThreats(from, bitBoard);
      currThreats = RookThreatBase[from][index];

      if(currThreats != EmptyBoard && currThreats != threats) {
        panic("Invalid magic for rooks in position %s, index %llu. Have threats:-\n"
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
