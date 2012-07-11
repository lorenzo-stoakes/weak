CC=clang

COMMON_FLAGS=-g -pedantic -Wall -Werror -Wshadow -std=c99 -m64 -ltcmalloc
CFLAGS=$(COMMON_FLAGS) -O3 -fomit-frame-pointer
DEBUG_FLAGS=$(COMMON_FLAGS) -O0

CODE_FILES=$(wildcard *.c *.h)

all: weak

weak: $(CODE_FILES)
	$(CC) $(CFLAGS) $(filter-out %.h, $^) -o $@

clean:
	rm -rf weak *.dSYM
debug: $(CODE_FILES)
	$(CC) $(DEBUG_FLAGS) $(filter-out %.h, $^) -o weak

