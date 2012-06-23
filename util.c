#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
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
panic(char *msg, ...)
{
  int len;
  char *msgNl;
  va_list args;

  va_start(args, msg);  

  len = strlen(msg);
  msgNl = (char*)allocate(len+1);
  strcpy(msgNl, msg);
  msgNl[len] = '\n';
  vfprintf(stderr, msgNl, args);
  
  va_end(args);

  abort();
}
