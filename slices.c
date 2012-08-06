#include "weak.h"

static const int INIT_MEMORY_COUNT = 100;

// TODO: Avoid duplication throughout.

MemorySlice
NewMemorySlice()
{
  MemorySlice ret;

  ret.Vals = (Memory*)allocate(sizeof(Memory), INIT_MEMORY_COUNT);
  ret.Curr = ret.Vals;

  return ret;
}


MoveSlice
NewMoveSlice(Move *buffer)
{
  MoveSlice ret;

  ret.Vals = buffer;
  ret.Curr = buffer;

  return ret;
}
