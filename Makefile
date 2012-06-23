CFLAGS=-pedantic -Wall -g -std=c99 -m64

all: weak

weak: bitboard.c\
      main.c\
      pawn.c\
      stringer.c\
      util.c\
      weak.h
	clang $(CFLAGS) main.c bitboard.c pawn.c stringer.c util.c -o weak