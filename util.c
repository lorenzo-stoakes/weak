#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "weak.h"

// We violate naming convention here for familiarity-with-go's sake. :-) TODO: Fix.

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

// Set stdout output unbuffered.
void
SetUnbufferedOutput()
{
  // Use unbuffered output.
  setbuf(stdout, NULL);
}
