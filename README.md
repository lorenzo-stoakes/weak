# Weak #

## Introduction ##

Weak is an open-source chess engine under active development, currently at a very early stage + thus
not even capable of actually playing a game yet :-)

Weak is written in C. This has been hastily ported from the [go version][0] so the code is
somewhat messy. I'm working on it:-).

The actual engine code is not complete yet, but the move generation passes all [perft][1] tests
and has relatively acceptable performance. The code still needs refactoring, but the next stage is to add a simple search + interface to the engine so it can actually be used :-) working on that now.

[0]:https://github.com/lorenzo-stoakes/weak-go
[1]:http://chessprogramming.wikispaces.com/perft
