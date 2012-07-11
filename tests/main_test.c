#include <stdio.h>
#include <strings.h>
#include "test.h"

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
    msg = TestFunctions[i]();

    if(msg != NULL) {
      failed++;
      printf("%s FAILED: %s\n", TestNames[i], msg);
    }
  }

  if(failed > 0) {
    printf("%d/%d tests passed, %d failed.\n", TEST_COUNT-failed, TEST_COUNT, failed);

    return 1;
  }
    
  return 0;
}
