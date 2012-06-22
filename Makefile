CFLAGS=-pedantic -Wall -g -std=c99

all: weak

weak: main.c
	gcc $(CFLAGS) main.c -o weak