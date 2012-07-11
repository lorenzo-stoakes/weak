#include <stdio.h>
#include <strings.h>
#include "../weak.h"
#include "test.h"

#if defined(QUICK_TEST)
#define MAX_DEPTH 4
#else
#define MAX_DEPTH 7
#endif

#define POSITION_COUNT 5
static char *fens[POSITION_COUNT] = {
  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
  "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -",
  "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -",
  "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
  "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1"
};

static int expectedDepthCounts[POSITION_COUNT] = { 6, 5, 7, 6, 6 };
static PerftStats expecteds[POSITION_COUNT][7] = {
  {
    {20, 0, 0, 0, 0, 0, 0 },
    {400, 0, 0, 0, 0, 0, 0},
    {8902, 34, 0, 0, 0, 12, 0},
    {197281, 1576, 0, 0, 0, 469, 8},
    {4865609, 82719, 258, 0, 0, 27351, 347},
    {119060324, 2812008, 5248, 0, 0, 809099, 10828}
  },
  {
    {48, 8, 0, 2, 0, 0, 0},
    {2039, 351, 1, 91, 0, 3, 0},
    {97862, 17102, 45, 3162, 0, 993, 1},
    {4085603, 757163, 1929, 128013, 15172, 25523, 43},
    {193690690, 35043416, 73365, 4993637, 8392, 3309887, 30171},
  },
  {
    {14, 1, 0, 0, 0, 2, 0},
    {191, 14, 0, 0, 0, 10, 0},
    {2812, 209, 2, 0, 0, 267, 0},
    {43238, 3348, 123, 0, 0, 1680, 17},
    {674624, 52051, 1165, 0, 0, 52950, 0},
    {11030083, 940350, 33325, 0, 7552, 452473, 2733},
    {178633661, 14519036, 294874, 0, 140024, 12797406, 87},
  },
  {
    {6, 0, 0, 0, 0, 0, 0},
    {264, 87, 0, 6, 48, 10, 0},
    {9467, 1021, 4, 0, 120, 38, 22},
    {422333, 131393, 0, 7795, 60032, 15492, 5},
    {15833292, 2046173, 6512, 0, 329464, 200568, 0},
    {706045033, 210369132, 212, 10882006, 81102984, 26973664, 0},
  },
  {
    {6, 0, 0, 0, 0, 0, 0},
    {264, 87, 0, 6, 48, 10, 0},
    {9467, 1021, 4, 0, 120, 38, 22},
    {422333, 131393, 0, 7795, 60032, 15492, 5},
    {15833292, 2046173, 6512, 0, 329464, 200568, 0},
    {706045033, 210369132, 212, 10882006, 81102984, 26973664, 0},
  }
};

char*
TestPerft()
{
  char tmp[200];
  Game game;
  int i, j;
  PerftStats actual, expected;
  StringBuilder builder = NewStringBuilder();

  // We want an initial newline so perft test results appear on separate lines.
  AppendString(&builder, "\n");

  for(i = 0; i < POSITION_COUNT; i++) {
    game = ParseFen(fens[i]);
    for(j = 0; j < expectedDepthCounts[i] && j < MAX_DEPTH; j++) {
      expected = expecteds[i][j];
      actual = Perft(&game, j+1);

      if(actual.Count != expected.Count) {
        sprintf(tmp, "Perft Position %d: Expected Count of %llu, got %llu.\n",
                i+1, expected.Count, actual.Count);
        AppendString(&builder, tmp);
      }

      if(actual.Captures != expected.Captures) {
        sprintf(tmp, "Perft Position %d: Expected Captures of %llu, got %llu.\n",
                i+1, expected.Captures, actual.Captures);
        AppendString(&builder, tmp);
      }

      if(actual.EnPassants != expected.EnPassants) {
        sprintf(tmp, "Perft Position %d: Expected EnPassants of %llu, got %llu.\n",
                i+1, expected.EnPassants, actual.EnPassants);
        AppendString(&builder, tmp);
      }

    }
  }

  return builder.Length == 1 ? NULL : BuildString(&builder, true);
}
