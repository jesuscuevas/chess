# chess
A simple chess engine written in C++ that conforms to the rules of chess as defined by FIDE (with the exception of a few unimplemented rules related to draws - see [Planned Features](#planned-features) for more information). The engine supports two game modes (player vs. player and player vs. engine - although currently this must be configured manually in the source code) and games can be played from the classical starting position or from a custom position specified in an FEN file. The engine currently uses alpha-beta pruning and does a material count and pawn structure evaluation to compute a node heuristic. It reads and displays move descriptions in algebraic notation.

## Planned Features
1. Implementing draw conditions other than stalemate (the 50 move rule, draw by repetition, and draw by insufficient material).
2. Improved move/position evaluation methods (better alpha-beta pruning heuristics and a better evaluation function).
3. Separate play and evaluation/analysis modes.
4. Improved text formatting (`libncurses`).

## To Run
```sh
./chess [options]
```
