#include <stdio.h>
#include "weak.h"

//#define GET_FULL_PERFT

int
main(int argc, char **argv)
{
  SetUnbufferedOutput();

  puts("WeakC v0.0.dev.\n");

  printf("Initialising... ");
  InitEngine();
  puts("done.\n");

  puts("Work in progress :-) Try make bench for some output.");

  return 0;
}
