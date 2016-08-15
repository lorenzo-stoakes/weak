COMMON_FLAGS=-g -D_GNU_SOURCE -pedantic -Wall -Werror -Wextra -Wpacked -Wshadow -std=c99 -m64 -O3 -lpthread

CFLAGS=$(COMMON_FLAGS) -DNDEBUG -fomit-frame-pointer
DEBUG_FLAGS=$(COMMON_FLAGS)

CODE_FILES=$(wildcard *.c *.h Makefile)

BENCH_FILES=$(CODE_FILES) $(wildcard benches/*.c benches/*.h)
TEST_FILES=$(CODE_FILES) $(wildcard tests/*.c tests/*.h)

FILTER_FILES=Makefile %.h

all: weak

weak: $(CODE_FILES)
	./genver.sh
	$(CC) $(CFLAGS) $(filter-out $(FILTER_FILES), $^) -o $@

# Typically, we don't want to run long-running benchmarks. Default to QUICK_BENCH.
bench: $(BENCH_FILES)
	$(CC) $(CFLAGS) -DQUICK_BENCH $(filter-out $(FILTER_FILES) main.c, $^) -o benches/bench
	./benches/bench

benchfull: $(BENCH_FILES)
	$(CC) $(CFLAGS) $(filter-out $(FILTER_FILES) main.c, $^) -o benches/bench
	./benches/bench

clean:
	rm -rf weak *.dSYM benches/bench benches/*.dSYM tests/test tests/*.dSYM

debug: $(CODE_FILES)
	./genver.sh
	$(CC) $(DEBUG_FLAGS) $(filter-out $(FILTER_FILES), $^) -o weak

# Typically, we don't want to run long-running tests. Default to QUICK_TEST.
test: $(TEST_FILES)
	$(CC) $(DEBUG_FLAGS) -DQUICK_TEST $(filter-out $(FILTER_FILES) main.c, $^) -o tests/test
	./tests/test

# However, if pedantry is required, we have this :-)
# Don't use debug flags here, as full tests become impractically slow.
testfull: $(TEST_FILES)
	$(CC) $(CFLAGS) $(filter-out $(FILTER_FILES) main.c, $^) -o tests/test
	./tests/test

trail:
    # Remove trailing whitespace in all .c, .h files.
	find . -name '*.h' -or -name '*.c' -or -name 'Makefile' | xargs -I _ sed -i '' 's/[ \\t]+$$//' _

.PHONY: all bench benchfull clean debug test testfull trail
