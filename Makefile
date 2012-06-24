CC=clang
CFLAGS=-pedantic -Wall -Werror -g -std=c99 -m64

all: weak

weak: $(wildcard *.c *.h)
	$(CC) $(CFLAGS) $(filter-out %.h, $^) -o $@

clean:
	rm -f weak
