CC=clang

COMMON_FLAGS=-g -pedantic -Wall -Werror -Wshadow -std=c99 -m64 -ltcmalloc
CFLAGS=$(COMMON_FLAGS) -O3 -fomit-frame-pointer
CODE_FILES=$(wildcard *.c *.h)

all: weak

weak: $(CODE_FILES)
	$(CC) $(CFLAGS) $(filter-out %.h, $^) -o $@

clean:
	rm -rf weak *.dSYM
