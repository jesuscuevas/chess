# chess
A simple chess engine written in C++ that conforms to the rules of chess as defined by FIDE (with the exception of a few unimplemented rules related to draws - see [Planned Features](#planned-features) for more information). The engine supports two game modes (player vs. player and player vs. engine - although currently this must be configured manually in the source code) and games can be played from the classical starting position or from a custom position specified in an FEN file. The engine uses alpha-beta pruning with captures-first move ordering and does a material count and pawn structure evaluation to evaluate heuristic nodes. It reads and displays move descriptions in algebraic notation.

## Planned Features
1. Draw by insufficient material.
1. Transposition table and iterative deepening.
1. Support for PGN file imports.
1. Support for FEN and PGN file exports.
1. Separate play and evaluation/analysis modes.
1. Improved text formatting (`libncurses`).

## Usage
```sh
make
./chess [options]
```
