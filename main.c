#include <stdio.h>
#include <sys/time.h>
#include "weak.h"

// Initialise lookup tables, etc.
static void
init()
{
  InitKnight();
  InitRays();
}

int
main(int argc, char **argv)
{
  double elapsed;
  Game game;
  PerftStats stats;
  struct timeval start, end;

  // Use unbuffered output.
  setbuf(stdout, NULL);

  puts("WeakC v0.0.dev.\n");

  printf("Initialising... ");
  init();
  puts("done.\n");

  game = NewGame(true, White);

  gettimeofday(&start, NULL);
  stats = Perft(&game, 4);
  gettimeofday(&end, NULL);

  elapsed = (end.tv_sec - start.tv_sec) * 1000.0;
  elapsed += (end.tv_usec - start.tv_usec) / 1000.0;

  puts(StringPerft(&stats));

  printf("%f ms elapsed.\n", elapsed);

  return 0;
}
