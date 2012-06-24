#include <stdio.h>
#include "weak.h"

// Initialise lookup tables, etc.
static void
init()
{
  InitKnight();
}

int
main(int argc, char **argv)
{
  Game game;
  PerftStats stats;

  init();

  puts("WeakC v0.0.dev.\n");

  game = NewGame(true, White);

  stats = Perft(&game, 4);
  puts(StringPerft(&stats));
  
  return 0;
}
