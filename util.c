#include <stdio.h>
#include <stdlib.h>
#include "weak.h"

// Wrapper for malloc. We might change the implementation later.
void*
allocate(size_t size)
{
  return malloc(size);
}

// Wrapper for free. We might change the implementation later.
void
release(void *ptr)
{
  free(ptr);
}

void
panic(char *msg)
{
  fprintf(stderr, "%s\n", msg);
  abort();
}
