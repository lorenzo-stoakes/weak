#include <stdio.h>
#include <time.h>
#include "weak.h"

//#define GET_FULL_PERFT

// Perft positions, see http://chessprogramming.wikispaces.com/Perft+Results.
#define FEN1 "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define FEN2 "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"
#define FEN3 "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -"
#define FEN4a "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"
#define FEN4b "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1"

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
