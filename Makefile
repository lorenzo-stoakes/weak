CFLAGS=-pedantic -Wall -Werror -g -std=c99 -m64

all: weak

weak: bitboard.c\
      chessset.c\
      main.c\
      pawn.c\
      set.c\
      stringer.c\
      util.c\
      weak.h
	clang $(CFLAGS) main.c bitboard.c chessset.c pawn.c set.c stringer.c util.c -o weak