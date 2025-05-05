# chess
A simple chess engine written in C++ that conforms to the rules of chess as defined by FIDE (with the exception of a few unimplemented rules related to draws - see [Planned Features](#planned-features) for more information). The engine supports two game modes (player vs. player and player vs. engine - although currently this must be configured manually in the source code) and games can be played from the classical starting position or from a custom position specified in an FEN file. The engine currently uses a minimax algorithm (at low depth for practical purposes) and does a basic material count to evaluate a simple node heuristic. It reads and displays move descriptions in algebraic notation.

## Planned Features
1. Implementing draw conditions other than stalemate (the 50 move rule, draw by repetition, and draw by insufficient material).
2. Improved move/position evaluation methods (alpha-beta pruning and a better heuristic node value function).
3. Separate play and evaluation/analysis modes.
4. Improved text formatting (`libncurses`).

## To Run
```sh
g++ -o chess chess.cpp
./chess [-d engine-depth] [-f FEN-file]
```
