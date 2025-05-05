#include <cstdio>
#include <iostream>
#include <getopt.h>

#include "game.h"

int main(int argc, char * argv[]) {
    // default parameters
    unsigned int depth = DEFAULT_DEPTH;
    std::string fenString = "";
    Game game(DEFAULT_DEPTH);

    // parse command line arguments
    int opt;
    while((opt = getopt(argc, argv, "df")) != -1) {
        switch(opt) {
            case 'd':
                depth = std::stoi(argv[optind]);
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
                std::cerr << "Usage: chess [-d depth] [-f FEN file]\n";
                return EXIT_FAILURE;
        }
    }

    // initialize game with FEN string if provided
    if(fenString.empty()) game = Game(depth);
    else game = Game(fenString, depth);

    // run game
    game.run(true);

    return 0;
}