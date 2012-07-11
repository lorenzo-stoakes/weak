#include <stdio.h>
#include <time.h>
#include "bench.h"

void
OutputBenchResults(char *name, double elapsed, long iters, int64_t nodes)
{
  printf("%s: %ld %f ms/op", name, iters, elapsed/iters);

  // Not all benchmarks will necessarily return node count, if not they each return -1.
  if(nodes > 0) {
    printf(" %f n/s", 1000*nodes/elapsed);
  }

  printf("\n");
}
