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
