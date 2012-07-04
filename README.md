# Weak #

## Introduction ##

Weak is an open-source chess engine under active development, currently at a very early stage + thus
not even capable of actually playing a game yet :-)

Weak is written in C. This has been hastily ported from the [go version][0] so the code is
somewhat messy. I'm working on it:-).

Right now the code simply generates moves and runs a [perft][1] test. I am working on
refactoring and optimising this code before porting the rest of the engine, adding tests for it
and expanding the code to a fully implemented, effective engine.

[0]:https://github.com/lorenzo-stoakes/weak-go
[1]:http://chessprogramming.wikispaces.com/perft
