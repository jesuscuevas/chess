# chess
A simple chess engine written in C++ that conforms to the rules of chess as defined by FIDE (with the exception of a few unimplemented rules revolving around draws - see [Planned Features](#planned-features) for more details). The engine supports two game modes (player vs. player and player vs. engine) and games can be played from the classical starting position or from a custom position specified by an FEN file. The engine currently uses a minimax algorithm (at low depth for practical purposes) and does a basic material count to evaluate a simple node heuristic. It reads and displays move descriptions in algebraic notation.

## Planned Features
1. Implementing draw conditions other than stalemate (the 50 move rule and draw by repetition).
2. Improved move/position evaluation methods (alpha-beta pruning and a better heuristic node value function).
3. Separate play and evaluation/analysis modes.
4. Improved text formatting (`libncurses`).

## To Run
```sh
g++ -o chess chess.cpp
./chess [FEN file]
```
