CC=clang

CFLAGS=-g -pedantic -Wall -Werror -std=c99 -m64 -ltcmalloc -lprofiler -O2

all: weak

weak: $(wildcard *.c *.h)
	$(CC) $(CFLAGS) $(filter-out %.h, $^) -o $@

clean:
	rm -rf weak *.dSYM
