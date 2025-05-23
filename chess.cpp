#include <cstdio>
#include <iostream>
#include <getopt.h>

#include "game.h"

int main(int argc, char * argv[]) {
    // default parameters
    unsigned int depth = DEFAULT_DEPTH;
    std::string fenString = "";
    bool debug = false;

    Game game(DEFAULT_DEPTH);

    // parse command line arguments
    int opt;
    while((opt = getopt(argc, argv, "dDf")) != -1) {
        switch(opt) {
            case 'd':
                depth = std::stoi(argv[optind]);
                break;
            case 'D':
                debug = true;
                break;
            case 'f':
            {
                FILE * fp = fopen(argv[optind], "r");
                if(!fp) {
                    std::cerr << "Could not open FEN file '" << argv[optind] << "'\n";
                    return EXIT_FAILURE;
                }
                
                char fen[1024] = {0};
                if(!fread(fen, sizeof(char), sizeof(fen), fp)) return EXIT_FAILURE;
                
                fclose(fp);

                fenString = std::string(fen);

                break;
            }
            default:
                std::cerr << "Usage: chess [options]\n";
                std::cerr << "-d depth : engine recursion depth\n";
                std::cerr << "-f file  : starts game from position in FEN file <file>\n";
                std::cerr << "-D       : start in debug mode" << std::endl;
                return EXIT_FAILURE;
        }
    }

    // initialize game with FEN string if provided
    if(fenString.empty()) game = Game(depth);
    else game = Game(fenString, depth);

    // run game
    game.run(debug);

    return 0;
}