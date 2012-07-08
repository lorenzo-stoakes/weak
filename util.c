#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "weak.h"

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

// Wrapper for free. We might change the implementation later.
void
release(void *ptr)
{
  // For now this doesn't want to work properly. TODO: Revisit.
  //  free(ptr);
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
