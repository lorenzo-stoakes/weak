#include "weak.h"

const long HashSize = 2097152;

// Assume no collisions.
BoardPositionPair *hashTable;
static int hashCount;

void
InitHash()
{
  hashTable = (BoardPositionPair*)allocate(sizeof(BoardPositionPair)*HashSize);
}

BitBoard
Hash(BitBoard bitBoard)
{
  // See http://stackoverflow.com/questions/5085915/

  bitBoard ^= bitBoard >> 33;
  bitBoard *= 0xff51afd7ed558ccd;
  bitBoard ^= bitBoard >> 33;
  bitBoard *= 0xc4ceb9fe1a85ec53;
  bitBoard ^= bitBoard >> 33;

  return bitBoard % HashSize;
}

void
HashAdd(BitBoard bitBoard, Positions positions)
{
  BoardPositionPair pair;

  BitBoard hash = Hash(bitBoard);

  pair.Board = bitBoard;
  pair.Positions = positions;

  hashTable[hash] = pair;
  hashCount++;
}

BoardPositionPair
HashGet(BitBoard bitBoard)
{
  return hashTable[Hash(bitBoard)];
  /*
  ret = hashTable[hash];

  if(ret.Board != bitBoard

  if(.Board == bitBoard) {
    return hashTable[hash];
  }

  empty.Board = EmptyBoard;
  return empty;*/
}
