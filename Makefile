CFLAGS=-pedantic -Wall -g -std=c99

all: weak

weak: bitboard.c\
      main.c\
      util.c\
      weak.h
	gcc $(CFLAGS) main.c bitboard.c util.c -o weak