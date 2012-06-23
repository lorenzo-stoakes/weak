#include <stdio.h>
#include <strings.h>
#include "weak.h"

char*
StringPosition(Position pos)
{
  char *ret;

  // Unsigned so we don't need to check < 0.
  if(pos > 63) {
    return strdup("#invalid position");
  }

  ret = (char*)allocate(2);

  if(sprintf(ret, "%c%d", 'a'+FILE(pos), RANK(pos)) != 2) {
    panic("Couldn't string position");
  }

  return ret;
}

