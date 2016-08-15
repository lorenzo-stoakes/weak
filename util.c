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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "weak.h"

// We violate naming convention here for familiarity-with-go's sake. :-) TODO: Fix.

#define INIT_BUILDER_SIZE 10

static void        expandBuilder(StringBuilder*);
static ListNode*   newListNode(List*, ListNode*, ListNode*, void*);
static PackedMoves newPackedMoves(void);

// Wrapper for malloc. We might change the implementation later.
void*
allocate(size_t size, size_t num)
{
  return malloc(size*num);
}

void*
allocateZero(size_t size, size_t num)
{
  return calloc(num, size);
}

void
panic(char *msg, ...)
{
  int len;
  char *msgNl;
  va_list args;

  va_start(args, msg);

  len = strlen(msg);
  msgNl = (char*)allocate(sizeof(char), len+1);
  strcpy(msgNl, msg);
  msgNl[len] = '\n';
  vfprintf(stderr, msgNl, args);

  va_end(args);

  abort();
}

// Wrapper for free. We might change the implementation later.
void
release(void *ptr)
{
  free(ptr);
}

void
AppendString(StringBuilder *builder, char *str, ...)
{
  char buffer[APPEND_STRING_BUFFER_LENGTH];
  int strLen;
  va_list args;

  if(builder->cap < builder->len) {
    panic("Impossible.");
  }
  if(builder->cap == builder->len) {
    expandBuilder(builder);
  }

  va_start(args, str);

  vsprintf(buffer, str, args);

  va_end(args);

  strLen = strlen(buffer);

  builder->strings[builder->len] = strdup(buffer);
  builder->len++;

  builder->Length += strLen;
}

char*
BuildString(StringBuilder *builder, bool releaseBuilder)
{
  char *ret;
  int i, j, k;

  char *str;

  if(builder->Length <= 0) {
    return NULL;
  }

  ret = (char*)allocate(sizeof(char), builder->Length + 1);

  k = 0;
  for(i = 0; i < builder->len && k < builder->Length; i++) {
    str = builder->strings[i];
    for(j = 0; str[j] != '\0'; j++) {
      ret[k] = str[j];
      k++;
    }
  }

  ret[k] = '\0';

  if(releaseBuilder) {
    ReleaseStringBuilder(builder);
  }

  return ret;
}

int
Max(int a, int b)
{
  return a >= b ? a : b;
}

List*
NewList()
{
  List *ret = allocate(sizeof(List), 1);

  ret->Count = 0;
  ret->Head = NULL;
  ret->Tail = NULL;

  return ret;
}

StringBuilder
NewStringBuilder()
{
  StringBuilder ret;

  ret.Length = 0;

  ret.cap = INIT_BUILDER_SIZE;
  ret.len = 0;
  ret.strings = (char**)allocate(sizeof(char*), INIT_BUILDER_SIZE);

  return ret;
}

PackedMoves
PackMoveHistory(MemorySlice *history, int offset)
{
  Memory *curr;
  Move move;
  PackedMoves ret = newPackedMoves();

  int i;
  int count = history->Curr - history->Vals - offset;
  int index;
  int packSize = count/4;
  int target;

  uint64_t packed;

  if(packSize == 0) {
    packSize = 1;
  }

  ret.Count = count;
  ret.Moves = allocate(sizeof(uint64_t), packSize);

  for(curr = history->Vals + offset, index = 0; curr < history->Curr; curr += 4, index++) {
    packed = 0;
    target = count < 4 ? count : 4;

    for(i = 0; i < target; i++) {
      move = curr[i].Move;
      packed |= ((uint64_t)move<<(i*16));
    }

    ret.Moves[index] = packed;
    count -= 4;
  }

  return ret;
}

void*
PopBack(List *list)
{
  ListNode *prev;
  void *ret;

  assert(list != NULL);
  assert(list->Count > 0);
  assert(list->Head);
  assert(list->Tail);

  ret = list->Tail->Item;
  prev = list->Tail->Prev;
  release(list->Tail);
  list->Tail = prev;

  if(list->Count == 1) {
    list->Head = NULL;
  }

  list->Count--;

  return ret;
}

void*
PopFront(List *list)
{
  ListNode *next;
  void *ret;

  assert(list != NULL);
  assert(list->Count > 0);
  assert(list->Head);
  assert(list->Tail);

  ret = list->Head->Item;
  next = list->Head->Next;
  release(list->Head);
  list->Head = next;

  if(list->Count == 1) {
    list->Tail = NULL;
  }

  list->Count--;

  return ret;
}

void
PushFront(List *list, void *item)
{
  ListNode *node = newListNode(list, NULL, list->Head, item);

  assert(list != NULL);

  if(list->Count == 0) {
    list->Tail = node;
  } else {
    list->Head->Prev = node;
  }

  list->Head = node;

  list->Count++;
}

void
PushBack(List *list, void *item)
{
  ListNode *node = newListNode(list, list->Tail, NULL, item);

  assert(list != NULL);

  if(list->Count == 0) {
    list->Head = node;
  } else {
    list->Tail->Next = node;
  }

  list->Tail = node;

  list->Count++;
}

void
ReleaseStringBuilder(StringBuilder *builder)
{
  int i;

  for(i = 0; i < builder->len; i++) {
    release(builder->strings[i]);
  }
}

// Set stdout output unbuffered.
void
SetUnbufferedOutput()
{
  // Use unbuffered output.
  setbuf(stdout, NULL);
}

Move*
UnpackMoveHistory(PackedMoves* packed, bool releasePackedMoves)
{
  int i, index, j, target;
  uint64_t packedVal;

  // +1 to zero-terminate.
  Move *ret = allocateZero(sizeof(Move), packed->Count+1);

  index = 0;
  for(i = 0; i < packed->Count; i++) {
    packedVal = packed->Moves[i];
    target = packed->Count - i*4 < 4 ? packed->Count - i*4 : 4;

    for(j = 0; j < target; index++, j++) {
      ret[index] = packedVal&((1<<16)-1);
      packedVal >>= 16;
    }
  }

  if(releasePackedMoves) {
    release(packed->Moves);
  }

  return ret;
}

static void
expandBuilder(StringBuilder *builder)
{
  char **buffer;

  builder->cap *= 2;
  buffer = (char**)allocate(sizeof(char*), builder->cap);
  memcpy(buffer, builder->strings, builder->len*sizeof(char*));
  release(builder->strings);
  builder->strings = buffer;
}

static ListNode*
newListNode(List* list, ListNode* prev, ListNode* next, void* item)
{
  ListNode *ret = allocate(sizeof(ListNode), 1);

  ret->List = list;
  ret->Prev = prev;
  ret->Next = next;
  ret->Item = item;

  return ret;
}

static PackedMoves
newPackedMoves()
{
  PackedMoves ret;

  ret.Count = 0;
  ret.Moves = NULL;

  return ret;
}
