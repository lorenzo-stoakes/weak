// perft_test.c
char* TestPerft(void);

#define TEST_COUNT 1

char* (*TestFunctions[TEST_COUNT])(void) = { &TestPerft };

char *TestNames[TEST_COUNT] = { "Perft Test" };
