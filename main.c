#include <stdio.h>
#include <time.h>
#include "weak.h"

int
main(int argc, char **argv)
{
  clock_t ticks;
  double elapsed;
  Game game;
  uint64_t ret;

  SetUnbufferedOutput();

  puts("WeakC v0.0.dev.\n");

  printf("Initialising... ");
  InitEngine();
  puts("done.\n");

  game = ParseFen(FEN1);

  ticks = clock();
  ret = QuickPerft(&game, 4);
  ticks = clock() - ticks;
  // In ms.
  elapsed = 1000*((double)ticks)/CLOCKS_PER_SEC;

  printf("%llu - %f elapsed.\n", ret, elapsed);

  return 0;
}
