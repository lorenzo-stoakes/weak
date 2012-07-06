#include <stdio.h>
#include <sys/time.h>
#include "weak.h"

//#define GET_FULL_PERFT

// Initialise lookup tables, etc.
static void
init()
{
  InitKnight();
  InitRays();

  // Relies on above.
  InitMagics();
}

int
main(int argc, char **argv)
{
  double elapsed;
  Game game;
#if defined(GET_FULL_PERFT)
  PerftStats stats;
#else
  uint64_t count;
#endif
  struct timeval start, end;

  // Use unbuffered output.
  setbuf(stdout, NULL);

  puts("WeakC v0.0.dev.\n");

  printf("Initialising... ");
  init();
  puts("done.\n");

  game = NewGame(false, White);

  gettimeofday(&start, NULL);
#if defined(GET_FULL_PERFT)
  stats = Perft(&game, 4);
#else
  count = QuickPerft(&game, 4);
#endif
  gettimeofday(&end, NULL);

  elapsed = (end.tv_sec - start.tv_sec) * 1000.0;
  elapsed += (end.tv_usec - start.tv_usec) / 1000.0;

#if defined(GET_FULL_PERFT)
  puts(StringPerft(&stats));
#else
  printf("%lld moves.\n\n", count);
#endif

  printf("%f ms elapsed.\n", elapsed);

  return 0;
}
