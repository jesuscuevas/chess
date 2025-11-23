#pragma once

#include <iomanip>
#include <random>
#include <sstream>

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
        Move move = board.bestMove(color, depth);

        board.tryMove(color, move);
    }
};

class HumanPlayer : public Player {
private:
    static std::string evaluationString(int evaluation) {
        if(IS_MATE(evaluation)) return std::string((evaluation > 0) ? "+M" : "-M") + std::to_string(MATE(evaluation));

        std::stringstream stream;
        if(evaluation > 0) stream << "+";
        stream << std::fixed << std::setprecision(2) << evaluation / 100.0f;
        return stream.str();
    }

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

            if(debug && move == "evaluate")
                std::cout << "Evaluation: " << evaluationString(board.evaluatePosition(board.toPlay, INT_MIN, INT_MAX, depth)) << std::endl;
            if (move == "moves") {
                // list all legal moves in the current position
                // if debug mode is enabled, evaluations will also be displayed
                std::list<Move> moves = board.getAlgebraicMoves(color);
                
                std::cout << "Legal moves:\n";

                // in debug mode, add move evaluations and sort moves by numerical evaluation
                if(debug) {
                    PieceColor sideToPlay = board.toPlay;
                    for (Move& move : moves) board.evaluateMove(move, INT_MIN, INT_MAX, depth);
                    moves.sort([sideToPlay](const Move& a, const Move& b) {
                        return BETTER(sideToPlay, a.evaluation, b.evaluation);
                    });

                    for(Move& move : moves) std::cout << move.algebraic << " (" << evaluationString(move.evaluation) << ")\n";
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
