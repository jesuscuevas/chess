#pragma once

#include <string>

#include "board.h"
#include "player.h"

class Game {
private:
    HumanPlayer player1;
    CPUPlayer player2;
    Board board;
public:
    Game(int depth) : player1(PieceColor::WHITE, depth), player2(PieceColor::BLACK, depth) {}
    Game(std::string fen, unsigned int depth) : Game(depth) { board = Board(fen); }

    void run(bool debug = false)  {
        board.display(debug);

        // main loop
        while(board.result == GameResult::IN_PROGRESS) {
            board.toPlay ? player2.move(board, debug) : player1.move(board, debug);
            board.display(debug);
        }

        // display game outcome - stalemate is currently the only draw condition and checkmate the only win condition
        switch (board.result) {
        case GameResult::DRAW_BY_STALEMATE:
            std::cout << "Drawn by stalemate!\n";
            break;
        case GameResult::DRAW_BY_REPETITION:
            std::cout << "Drawn by threefold repetition!\n";
            break;
        case GameResult::DRAW_BY_50_MOVE_RULE:
            std::cout << "Drawn by the 50 move rule\n";
            break;
        case GameResult::DRAW_BY_INSUFFICIENT_MATERIAL:
            std::cout << "Drawn by insufficient material\n";
            break;
        case GameResult::WHITE_WINS:
            std::cout << "Checkmate! White wins!\n";
            break;
        case GameResult::BLACK_WINS:
            std::cout << "Checkmate! Black wins!\n";
            break;
        default:
            std::cerr << "Game ended unexpectedly\n";
            exit(EXIT_FAILURE);
        }
    }
};