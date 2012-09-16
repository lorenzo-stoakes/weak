/*
  Weak, a chess engine derived from Stockfish.

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

// Derived from Stockfish transposition table.

#include "weak.h"

// TODO: Parameterise.
#define DEFAULT_SIZE_MB 512

//static uint8_t       generation = 0;
static TransCluster* clusters   = NULL;
static uint64_t      transSize  = 0;
static uint8_t       generation = 0;

static FORCE_INLINE TransEntry* firstEntry(uint64_t);
static void         saveEntry(TransEntry*, uint16_t, uint8_t, uint32_t, QuickMove, int);

void
InitTrans()
{
  ResizeTrans(DEFAULT_SIZE_MB);
}

TransEntry*
LookupPosition(uint64_t key)
{
  int i;
  TransEntry *entry = firstEntry(key);
  uint32_t innerKey = key >> 32;

  for(i = 0; i < TRANS_CLUSTER_SIZE; i++, entry++) {
    if(entry->Key32 == innerKey) {
      return entry;
    }
  }

  return NULL;
}

void
NextSearchTrans()
{
  // We use generation to determine which entries relate to our current search or not.
  generation++;
}

void
ResizeTrans(uint64_t sizeMb)
{
  uint64_t size;

  // We want size to be a multiple of 2, minimum 1kb.
  size = 1024;
  while(C64(2) * size * sizeof(TransCluster) <= (sizeMb * C64(1024) * C64(1024))) {
    size *= 2;
  }

  // Nothing to do if resizing to same size.
  if(size == transSize) {
    return;
  }

  if(clusters != NULL) {
    release(clusters);
  }

  transSize = size;
  clusters = allocateZero(sizeof(TransCluster), transSize);
}

void
SavePosition(uint64_t key, int value, QuickMove quickMove, uint16_t depth)
{
  int i, weight;
  TransEntry *entry, *replaceee;

  entry = replaceee = firstEntry(key);

  // Use high 32 bits of key to key inside the cluster.
  uint32_t innerKey = key >> 32;

  for(i = 0; i < TRANS_CLUSTER_SIZE; i++, entry++) {
    if(!entry->Key32 || entry->Key32 == innerKey) {
      saveEntry(entry, depth, generation, innerKey, quickMove, value);

      return;
    }

    // This entry is occupied, determine whether it ought to be replaced over current
    // replacement candidate.

    // Again we are mimicking Stockfish's approach here :-)

    // We give major weighting to entries which are from the same search.
    weight =  replaceee->Generation == generation ? 2 : 0;
    weight += entry->Generation == generation ? -2 : 0;
    // Otherwise, our main differentiator is depth.
    weight += entry->Depth < replaceee->Depth ? 1 : 0;

    if(weight > 0) {
      replaceee = entry;
    }
  }

  // If we get here, then we're going to have to replace.
  saveEntry(replaceee, depth, generation, innerKey, quickMove, value);
}

void
UpdateGeneration(TransEntry *entry)
{
  entry->Generation = generation;
}

static FORCE_INLINE
TransEntry* firstEntry(uint64_t key)
{
  // Use low 32 bits of key to determine the cluster.
  return clusters[((uint32_t)key) & (transSize-1)].Data;
}

static void
saveEntry(TransEntry *entry, uint16_t depth, uint8_t gen, uint32_t key32,
          QuickMove quickMove, int value)
{
  entry->Depth = depth;
  entry->Generation = gen;
  entry->Key32 = key32;
  entry->QuickMove = quickMove;
  entry->Value = value;
}
