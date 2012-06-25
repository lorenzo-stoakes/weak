# Weak #

## Introduction ##

Weak is an open-source chess engine under active development, currently at a very early stage + thus
not even capable of actually playing a game yet :-)

Weak is written in C. Note it uses the latest development version of go, so it may not compile
against release or weekly versions.

## The Plan ##

The plan for development is to move in steps, starting with the absolute basics and slowly moving
towards a fully-featured engine. We are currently at stage 1.

0:  Not yet capable of playing a game.

1: Capable of playing a game (badly), does not implement [50 move rule][1] or
[threefold repetition][2].

1.5: Improve code quality and unit tests.

1.75: Add [xboard][0] support.

2:  Introduce the [50 move rule][2] and [threefold repetition][3].

3a: Introduce actual analysis + strategy, gradually improving it against a yet-to-be-determined
means of assessment of ability.

3b: Optimise. Improve code where flakiness detected.

4:  ???

5:  Profit!

## Eventual Aims of the Project ##

* Strength - The name is intended to eventually become ironic rather than descriptive :) the aim is
  for weak to be strong enough to be a credible chess opponent both against humans and other chess
  programs.

* Performance - One of the primary aims of the weak project is to exhibit excellent performance.

[0]:http://www.gnu.org/software/xboard/
[1]:http://en.wikipedia.org/wiki/50_move_rule
[2]:http://en.wikipedia.org/wiki/Three-fold_repetition
