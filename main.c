#include <stdio.h>
#include "weak.h"

int
main(int argc, char **argv)
{
  int i;
  Game game;
  Move move;
  MoveSlice moves;

  puts("WeakC v0.0.dev.\n");

  game = NewGame(true, White);
  moves = AllMoves(&game);

  for(i = 0; i < moves.Len; i++) {
    move = moves.Vals[i];
    puts(StringMove(&move));
  }

  return 0;
}
