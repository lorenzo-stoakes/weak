# Weak #

## Introduction ##

Weak is an open-source chess engine under active development, currently at a very early stage +
thus not even capable of actually playing a game yet :-)

Weak is written in C. The engine was originally written in [go][0], but I faced performance
problems I couldn't easy overcome, so have switched to C.

The actual engine code is not complete yet, but the move generation passes all [perft][1]
tests. I am working on improving move generation performance for the time being, so at any
stage I might break the tests while introducing some new optimisations. As soon as I have
performance to a level I'm happy with, that will stabilise.

Once the move generation performance exercise is complete, I will add search, evaluation and a
front-end, probably a [UCI][2]-compatible interface.

There is no engine yet to speak of, but it is coming, off in the distance...

I am very influenced and inspired by the [Chess Programming Wiki][3] and [Stockfish][4], the
best open-source chess engine out there.

[0]:https://github.com/lorenzo-stoakes/weak-go
[1]:http://chessprogramming.wikispaces.com/perft
[2]:http://en.wikipedia.org/wiki/Universal_Chess_Interface
[3]:http://chessprogramming.wikispaces.com/
[4]:http://www.stockfishchess.com/
