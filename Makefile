CC=clang

COMMON_FLAGS=-g -pedantic -Wall -Werror -Wshadow -std=c99 -m64 -ltcmalloc

CFLAGS=$(COMMON_FLAGS) -O3 -fomit-frame-pointer
DEBUG_FLAGS=$(COMMON_FLAGS) -O0

CODE_FILES=$(wildcard *.c *.h)

BENCH_FILES=$(CODE_FILES) $(wildcard benches/*.c benches/*.h)
TEST_FILES=$(CODE_FILES) $(wildcard tests/*.c tests/*.h)

all: weak

weak: $(CODE_FILES)
	$(CC) $(CFLAGS) $(filter-out %.h, $^) -o $@

# Typically, we don't want to run long-running benchmarks. Default to QUICK_BENCH.
bench: $(BENCH_FILES)
	$(CC) $(CFLAGS) -DQUICK_BENCH $(filter-out main.c %.h, $^) -o benches/bench
	./benches/bench

benchfull: $(BENCH_FILES)
	$(CC) $(CFLAGS) $(filter-out %.h, $^) -o benches/bench
	./benches/bench

clean:
	rm -rf weak *.dSYM benches/bench benches/*.dSYM tests/test tests/*.dSYM

debug: $(CODE_FILES)
	$(CC) $(DEBUG_FLAGS) $(filter-out %.h, $^) -o weak

# Typically, we don't want to run long-running tests. Default to QUICK_TEST.
test: $(TEST_FILES)
	$(CC) $(CFLAGS) -DQUICK_TEST $(filter-out main.c %.h, $^) -o tests/test
	./tests/test

# However, if pedantry is required, we have this :-)
testfull: $(TEST_FILES)
	$(CC) $(CFLAGS) $(filter-out main.c %.h, $^) -o tests/test
	./tests/test

.PHONY: all bench benchfull clean debug test testfull
