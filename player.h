#pragma once

#include <random>

#include "board.h"

#define DEFAULT_DEPTH 2 // default engine recursion depth

class Player {
public:
	PieceColor color;
	unsigned int depth;

	Player(PieceColor color = PieceColor::WHITE, unsigned int depth = DEFAULT_DEPTH) : color(color), depth(depth) {}
	virtual void move(Board& board, bool debug = false) = 0;
};

class CPUPlayer : public Player {
public:
	CPUPlayer(PieceColor color) : Player(color) {}
	CPUPlayer(PieceColor color, unsigned int depth) : Player(color, depth) {}

	void move(Board& board, bool debug = false) {
		Move move = board.evaluate(color, depth);

		board.tryMove(color, move);
	}
};

class HumanPlayer : public Player {
public:
	HumanPlayer(PieceColor color) : Player(color) {}
	HumanPlayer(PieceColor color, unsigned int depth) : Player(color, depth) {}

	void move(Board& board, bool debug = false) {
		std::string move;

		bool valid = false;

		// poll user
		while (!valid) {
			std::cout << "Move (" << (color ? "Black" : "White") << "): ";
			std::cin >> move;

			if (move == "moves") {
				// list all legal moves in the current position
				// if debug mode is enabled, evaluations will also be displayed
				std::list<Move> moves = board.getAlgebraicMoves(color);
				
				std::cout << "Legal moves:\n";

				// in debug mode add move evaluations
				if(debug) {
					for (Move& move : moves) board.evaluateMove(move, depth);
					moves.sort([board](const Move& a, const Move& b) {
						return board.toPlay ? a.evaluation < b.evaluation : a.evaluation > b.evaluation;
					});

					for(Move& move : moves) {
						std::cout << move.algebraic << " (";
						if(IS_MATE(move.evaluation)) std::cout << (move.evaluation > 0 ? "+" : "-") << "M" << MATE(move.evaluation);
						else std::cout << (move.evaluation > 0 ? "+" : "") << move.evaluation;
						std::cout << ")\n";
					}
				} else for(Move& move : moves) std::cout << move.algebraic << "\n";

				std::cout << std::endl;
			} else if (move == "resign") {
				board.result = board.toPlay ? GameResult::WHITE_WINS : GameResult::BLACK_WINS;
				break;
			} else if(move == "exit" || move == "quit") exit(EXIT_SUCCESS);

			if (valid = board.parseMove(color, move, debug)) board.display(debug);
		}
	}
};
