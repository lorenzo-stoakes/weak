#include <stdio.h>
#include "weak.h"

int
main(int argc, char **argv)
{
  Game game;
  PerftStats stats;

  puts("WeakC v0.0.dev.\n");

  game = NewGame(true, White);

  stats = Perft(&game, 4);
  puts(StringPerft(&stats));
  
  return 0;
}
