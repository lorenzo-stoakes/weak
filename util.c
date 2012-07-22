#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "weak.h"

// We violate naming convention here for familiarity-with-go's sake. :-) TODO: Fix.

#define INIT_BUILDER_SIZE 10

static void expandBuilder(StringBuilder*);

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
AppendString(StringBuilder *builder, char *str)
{
  int strLen = strlen(str);

  if(strLen <= 0) {
    return;
  }

  if(builder->cap < builder->len) {
    panic("Impossible.");
  }
  if(builder->cap == builder->len) {
    expandBuilder
(builder);
  }

  builder->strings[builder->len] = strdup(str);
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
