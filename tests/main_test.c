/*
  Weak, a chess engine derived from Stockfish.

  Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
  Copyright (C) 2008-2012 Marco Costalba, Joona Kiiski, Tord Romstad (Stockfish authors)
  Copyright (C) 2011-2012 Lorenzo Stoakes

  Weak is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Weak is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <strings.h>
#include "test.h"

char* (*TestFunctions[TEST_COUNT])(void);
char *TestNames[TEST_COUNT];

static void
init()
{
  TestFunctions[0] = &TestPerft;
  TestNames[0] = strdup("Perft Test");
}

int main()
{
  int failed = 0;
  int i;
  char *msg;

  init();

  InitEngine();

  for(i = 0; i < TEST_COUNT; i++) {
    // HACK. TODO: Fix!
    msg = TestPerft();    
    //msg = testFunctions[i]();

    if(msg != NULL) {
      failed++;
      printf("%s FAILED: %s\n", "Perft Test", msg);
    }
  }

  if(failed > 0) {
    printf("%d/%d tests passed, %d failed.\n", TEST_COUNT-failed, TEST_COUNT, failed);

    return 1;
  }

  return 0;
}
