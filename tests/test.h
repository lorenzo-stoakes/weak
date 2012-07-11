#ifndef TEST_HEADER
#define TEST_HEADER

#include "../weak.h"

#define TEST_COUNT 1

// perft_test.c
char* TestPerft(void);

char* (*TestFunctions[TEST_COUNT])(void);
char *TestNames[TEST_COUNT];

#endif
