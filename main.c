#include <stdio.h>
#include <time.h>
#include "weak.h"

//#define GET_FULL_PERFT

int
main(int argc, char **argv)
{
  Game game;
  int plies;
#if defined(GET_FULL_PERFT)
  PerftStats stats;
#else
  uint64_t count;
#endif
  clock_t ticks;
  double elapsed;

  // Use unbuffered output.
  setbuf(stdout, NULL);

  if(argc <= 1) {
    printf("Usage: %s [perft plies]\n", argv[0]);
    return 1;
  }

  plies = atoi(argv[1]);

  if(plies < 1) {
    printf("Invalid integer %s.\n", argv[1]);
  }

  puts("WeakC v0.0.dev.\n");

  printf("Initialising... ");
  InitEngine();
  game = ParseFen(FEN1);
  puts("done.\n");

  puts(StringChessSet(&game.ChessSet));

  ticks = clock();
#if defined(GET_FULL_PERFT)
  stats = Perft(&game, plies);
#else
  count = QuickPerft(&game, plies);
#endif
  ticks = clock() - ticks;
  elapsed = ((double)ticks)/CLOCKS_PER_SEC;

  printf("%d plies.\n", plies);

#if defined(GET_FULL_PERFT)
  puts(StringPerft(&stats));
#else
  printf("%llu moves.\n", count);
  printf("%llu nps.\n\n", (uint64_t)(count/elapsed));
#endif

  printf("%f ms elapsed.\n", 1000*elapsed);

  return 0;
}
