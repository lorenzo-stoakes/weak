CC=gcc

CFLAGS=-g -pedantic -Wall -Werror -std=c99 -m64 -ltcmalloc -lprofiler -O3 -fomit-frame-pointer

all: weak

weak: $(wildcard *.c *.h)
	$(CC) $(CFLAGS) $(filter-out %.h, $^) -o $@

clean:
	rm -rf weak *.dSYM
