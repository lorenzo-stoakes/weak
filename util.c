#include <stdlib.h>
#include "weak.h"

// Wrapper for malloc. We might change the implementation later.
void*
mal(size_t size) {
  return malloc(size);
}

// Wrapper for free. We might change the implementation later.
void
free(void *ptr) {
  free(ptr);
}