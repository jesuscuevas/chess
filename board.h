#pragma once

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <regex>
#include <stack>
#include <string>
#include <vector>

#include "types.h"

// resolves to the (absolute) rank number of `color`'s `n`th rank (1-8)
#define RANK(color, n) ((Rank) (color ? (9 - n) : n))

// returns an evaluation number that encodes the number of moves required for the fastest checkmate in the position
#define MATE_IN(color, n) (color ? (INT_MIN + n) : (INT_MAX - n))

// extracts [n] from a "mate in [n]" evalation number
#define MATE(evaluation) ((evaluation > 0) ? (INT_MAX - evaluation) : (evaluation - INT_MIN))

// returns whether or not the evaluation number `evaluation` is intended to represent a checkmate
#define IS_MATE(evaluation) (EVAL_COLOR(evaluation) ^ (bool) (evaluation & 0x40000000))

// returns the color that's favored in a position given an evaluation number (white is returned by default if the evaluation is even)
#define EVAL_COLOR(evaluation) ((evaluation < 0) ? PieceColor::BLACK : PieceColor::WHITE)

// returns whether evaluation `eval1` is better for `color`than `eval2`
#define BETTER(color, eval1, eval2) (color ? (eval1 < eval2) : (eval1 > eval2))

// returns the opposite color of `color`
#define OPPOSITE(color) ((PieceColor) (color ^ 1))

/* Checkmate evaluation system:
    INT_MAX = Mate in 0 (for white)
    INT_MAX - 1 = Mate in 1 (for white)
    INT_MAX - 2 = Mate in 2 (for white)
    ...
    INT_MIN + 2 = Mate in 2 (for black)
    INT_MIN + 1 = Mate in 1 (for black)
    INT_MIN = Mate in 0 (for black)
*/

// list of ways each piece can move (todo: a better system that's less redundant)
const std::map<PieceType, std::list<CoordOffset>> PIECE_OFFSETS[2] = {
    {
        { PieceType::PAWN, {{0, 1}, {0, 2}, {1, 1}, {-1, 1}}},
        { PieceType::KNIGHT, {{1, 2}, {1, -2}, {2, 1}, {2, -1}, {-1, 2}, {-1, -2}, {-2, 1}, {-2, -1}}},
        { PieceType::KING, {{-2, 0}, {2, 0}, {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, {0, -1}, {1, 0}, {1, 1}, {1, -1}}}
    }, {
        { PieceType::PAWN, {{0, -1}, {0, -2}, {1, -1}, {-1, -1}}},
        { PieceType::KNIGHT, {{1, 2}, {1, -2}, {2, 1}, {2, -1}, {-1, 2}, {-1, -2}, {-2, 1}, {-2, -1}}},
        { PieceType::KING, {{-2, 0}, {2, 0}, {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, {0, -1}, {1, 0}, {1, 1}, {1, -1}}}
    }
};

// Unicode representation of pieces
const char PIECES[2][7][4] = {
    {"\u2659", "\u2658", "\u2657", "\u2656", "\u2655", "\u2654"},
    {"\u265f", "\u265e", "\u265d", "\u265c", "\u265b", "\u265a"}
};

// piece values (measured in centipawns)
const int PIECE_VALUES[6] = { 100, 300, 300, 500, 900, 99999 };

// unit vectors for rank and file offsets
const CoordOffset DELTA_RANK = {0, 1};
const CoordOffset DELTA_FILE = {1, 0};

class Board {
public:
    GameResult result = GameResult::IN_PROGRESS;
    PieceColor toPlay = PieceColor::WHITE;
    int moveNumber = 1; // move number (currently unused)
    int halfMoveNumber = 1; // number of half moves (plies) - to be used eventually for the fifty-move rule
private:
    enum Side { QUEEN, KING };

    SquareColor colors[9][9]; // square colors used for display() calls
    Square board[9][9] = {NULL}; // board representation
    std::stack<GameState> states; // stack of board state information
    std::list<Move> moves; // list of moves made this game
    std::list<Piece> pieces[2]; // list of all pieces
    std::map<PieceType, std::list<Piece *>> remaining[2]; // remaining pieces (pieces still on the board)
    std::map<PieceType, std::list<Piece *>> captured[2]; // captured pieces (pieces no longer on the board)
    std::string fg[2] = {"\x1b[38:5:255m", "\x1b[38:5:232m"}; // foreground terminal color
    std::string bg[2] = {"\x1b[48:5:248m", "\x1b[48:5:240m"}; // background terminal color

public:
    Square& operator[](Coord& coord) {
        return board[coord.rank][coord.file];
    }

    inline Square& operator[](const Coord& coord) {
        return operator[]((Coord&)coord);
    }

    std::map<PieceType, std::vector<Range>> PIECE_RANGES = {
        {
            PieceType::ROOK, {
                Range(DELTA_FILE,  DELTA_FILE), Range(-DELTA_FILE, -DELTA_FILE),
                Range(DELTA_RANK,  DELTA_RANK), Range(-DELTA_RANK, -DELTA_RANK)
            }
        }, {
            PieceType::BISHOP, {
                Range(DELTA_RANK + DELTA_FILE, DELTA_RANK + DELTA_FILE), Range(DELTA_RANK - DELTA_FILE, DELTA_RANK - DELTA_FILE),
                Range(-DELTA_RANK + DELTA_FILE, -DELTA_RANK + DELTA_FILE), Range(-DELTA_RANK - DELTA_FILE, -DELTA_RANK - DELTA_FILE)
            }
        }, { PieceType::QUEEN, {
                Range(DELTA_FILE,  DELTA_FILE), Range(-DELTA_FILE, -DELTA_FILE),
                Range(DELTA_RANK,  DELTA_RANK), Range(-DELTA_RANK, -DELTA_RANK),
                Range(DELTA_RANK + DELTA_FILE, DELTA_RANK + DELTA_FILE), Range(DELTA_RANK - DELTA_FILE, DELTA_RANK - DELTA_FILE),
                Range(-DELTA_RANK + DELTA_FILE, -DELTA_RANK + DELTA_FILE), Range(-DELTA_RANK - DELTA_FILE, -DELTA_RANK - DELTA_FILE)
            }
        }
    };

    // whether coordinate is on board
    static bool onBoard(Coord coord) {
        return (1 <= coord.rank) && (coord.rank <= 8) && (File::A <= coord.file) && (coord.file <= File::H);
    };

    // classical starting position (default)
    Board() {
        // initialize board state
        GameState state;
        state.canCastle[PieceColor::WHITE][Side::QUEEN] = true;
        state.canCastle[PieceColor::BLACK][Side::QUEEN] = true;
        state.canCastle[PieceColor::WHITE][Side::KING] = true;
        state.canCastle[PieceColor::BLACK][Side::KING] = true;
        state.passant = (Piece *) NULL;
        states.push(state);

        // place pieces on the board
        for (PieceColor color : {PieceColor::WHITE, PieceColor::BLACK}) {
            /* place front rank */
            Rank rank = RANK(color, 2);

            // place pawns
            for (File file = File::A; file <= File::H; file++) {
                Piece pawn = {color, PieceType::PAWN, {file, rank}};
                pieces[color].push_back(pawn);
                board[rank][file] = &pieces[color].back();
            }

            /* place back rank */
            rank = RANK(color, 1);

            // place rooks
            Piece queensRook = { color, PieceType::ROOK, {File::A, rank} };
            pieces[color].push_back(queensRook);
            board[rank][A] = &pieces[color].back();

            Piece kingsRook = { color, PieceType::ROOK, {File::H, rank} };
            pieces[color].push_back(kingsRook);
            board[rank][H] = &pieces[color].back();

            // place knights
            Piece queensKnight = { color, PieceType::KNIGHT, {File::B, rank} };
            pieces[color].push_back(queensKnight);
            board[rank][B] = &pieces[color].back();

            Piece kingsKnight  = { color, PieceType::KNIGHT, {File::G, rank} };
            pieces[color].push_back(kingsKnight);
            board[rank][G] = &pieces[color].back();

            // place bishops
            Piece queensBishop = { color, PieceType::BISHOP, {File::C, rank} };
            pieces[color].push_back(queensBishop);
            board[rank][C] = &pieces[color].back();

            Piece kingsBishop  = { color, PieceType::BISHOP, {File::F, rank} };
            pieces[color].push_back(kingsBishop);
            board[rank][F] = &pieces[color].back();

            // place queen
            Piece queen = { color, PieceType::QUEEN, {File::D, rank} };
            pieces[color].push_back(queen);
            board[rank][D] = &pieces[color].back();

            // place king
            Piece king = { color, PieceType::KING, {File::E, rank} };
            pieces[color].push_back(king);
            board[rank][E] = &pieces[color].back();
        }

        // load list of references to remaining pieces (all pieces)
        for (uint8_t color = PieceColor::WHITE; color <= PieceColor::BLACK; color++)
            for (Piece& piece : pieces[color])
                remaining[color][piece.type].push_back(&piece);

        // cache square colors for faster display() calls
        for (Rank rank = 1; rank <= 8; rank++)
            for (File file = File::A; file <= File::H; file++)
                colors[rank][file] = (SquareColor) ((rank + file + 1) % 2);
    }

    // load board state from FEN string
    Board(std::string fen) {
        const std::regex FEN("^([PNBRQKpnbrqk1-8]+)(\\/[PNBRQKpnbrqk1-8]+){7} [wb] (-|(K?Q?k?q?)) (-|([a-h][1-8])) [0-9]+ [0-9]+$");

        // invalid format
        if(!std::regex_match(fen, FEN)) {
            std::cerr << "invalid FEN text\n";
            exit(EXIT_FAILURE);
        }

        // piece placement
        size_t curr = 0;
        size_t next = fen.find(" ");
        std::string token = fen.substr(curr, next - curr);

        size_t c = 0;
        size_t n;
        for(uint8_t rank = 8; rank >= 1; rank--) {
            n = token.find("/", c);
            if(n == std::string::npos) n = token.size();

            std::string rankString = token.substr(c, n - c);
            File file = File::A;

            for(char c : rankString) {
                int piece = (PieceType) std::string("PNBRQKpnbrqk").find(c);
                if(piece == std::string::npos) file = file + (c - '1');
                else {
                    PieceColor color = (PieceColor) (piece / 6);
                    PieceType type = (PieceType) (piece % 6);
                    pieces[color].push_back((Piece) {
                        .color = color,
                        .type = type,
                        .location = {file, rank}
                    });
                    board[rank][file] = &pieces[color].back();
                }
                file++;
            }

            c = n + 1;
        }

        // side to play
        curr = next + 1;
        next = fen.find(" ", curr);
        token = fen.substr(curr, next - curr);
        toPlay = (PieceColor) std::string("wb").find(token[0]);

        // castling rights
        GameState state;
        curr = next + 1;
        next = fen.find(" ", curr);
        token = fen.substr(curr, next - curr);
        state.canCastle[0][0] = false;
        state.canCastle[0][1] = false;
        state.canCastle[1][0] = false;
        state.canCastle[1][1] = false;
        if(token != "-") for(char c : token) {
            int i = std::string("KQkq").find(c);
            state.canCastle[i / 2][i % 2] = true;
        }

        // passant candidate
        curr = next + 1;
        next = fen.find(" ", curr);
        token = fen.substr(curr, next - curr);
        state.passant = NULL;
        if(token != "-") {
            Piece * piece = board[token[0] - '`'][token[1] - '0'];
            state.passant = piece;
        }

        // half moves
        curr = next + 1;
        next = fen.find(" ", curr);
        token = fen.substr(curr, next - curr);
        halfMoveNumber = std::stoi(token);

        // full moves
        curr = next + 1;
        next = fen.length();
        token = fen.substr(curr, next - curr);
        moveNumber = std::stoi(token);

        // load list of references to remaining pieces (all pieces)
        for (uint8_t color = PieceColor::WHITE; color <= PieceColor::BLACK; color++)
            for (Piece& piece : pieces[color])
                remaining[color][piece.type].push_back(&piece);

        // cache square colors for faster display() calls
        for (Rank rank = 1; rank <= 8; rank++)
            for (File file = File::A; file <= File::H; file++)
                colors[rank][file] = (SquareColor) ((rank + file + 1) % 2);

        // update state
        states.push(state);
    }

    // returns a superset of all legal moves for color `color`
    std::list<Move> getCandidateMoves(PieceColor color) {
        std::list<Move> moves;

        Board& board = *this;

        // non-ranged pieces
        for (PieceType pieceType : {PieceType::PAWN, PieceType::KNIGHT, PieceType::KING}) {
            for (Piece* piece : remaining[color][pieceType]) {
                for (const CoordOffset& offset : PIECE_OFFSETS[color].at(pieceType)) {
                    Move move;
                    move.from = piece->location;
                    move.to = piece->location + offset;

                    if (!onBoard(move.to)) continue;
                    move.piece = piece;
                    move.capture = board[move.to];

                    // add move and all move variants (i.e. alternative promotions) to candidate list
                    if(piece->type == PieceType::PAWN && move.to.rank == RANK(color, 8)) {
                        for(uint8_t promoteTo = PieceType::KNIGHT; promoteTo <= PieceType::QUEEN; promoteTo++) {
                            move.promoteTo = (PieceType) promoteTo;
                            moves.push_back(move);
                        }
                    } else moves.push_back(move);
                }
            }
        }

        // ranged pieces (variable range)
        for (PieceType pieceType : {PieceType::ROOK, PieceType::BISHOP, PieceType::QUEEN}) {
            for (Piece* piece : remaining[color][pieceType]) {
                for (Range& range : PIECE_RANGES[pieceType]) {
                    while (range.hasNext()) {
                        Move move;
                        move.from = piece->location;
                        move.to = piece->location + range.next();

                        if (!onBoard(move.to)) break;
                        move.piece = piece;
                        move.capture = board[move.to];

                        moves.push_back(move);
                        if (board[move.to]) break;
                    }
                    range.reset();
                }
            }
        }

        return moves;
    }

    // returns a list of every legal move that `color` has in the current position
    std::list<Move> getMoves(PieceColor color) {
        std::list<Move> moves;
        for (Move& move : getCandidateMoves(color)) if (validate(color, move)) moves.push_back(move);
        return moves;
    }

    // returns a list of every legal move in the position with algebraic notation move descriptions
    // (theoretically slows down the evaluation but makes debugging easier)
    std::list<Move> getAlgebraicMoves(PieceColor color) {
        std::list<Move> moves = getMoves(color);
        for (Move& move : moves) {
            std::string algebraic = toAlgebraic(move);
            move.algebraic = (char *) malloc(algebraic.length() + 1);
            strcpy(move.algebraic, algebraic.c_str());
        }
        return moves;
    }

    // Returns whether or not a square is being attacked by a piece owned by `color` (Note: an 'attack' as defined by FIDE
    // does not depend on the ability for the attacking piece to capture a piece on that square. For example, a piece pinned
    // to its king is still said to be 'attacking' the squares it would otherwise be able to capture on had it not been pinned 
    // (FIDE Handbook E. 3.1.2).
    bool isAttacked(PieceColor color, Coord location) const {
        Board& board = (Board&)*this;

        // king
        for(const CoordOffset offset : PIECE_OFFSETS[color].at(PieceType::KING)) {
            if(abs(offset.dfile) > 1) continue; // king can't castle into a capture
            Coord coord = location + offset;
            if (!onBoard(coord) || !board[coord]) continue;

            const Piece& piece = *board[coord];
            if (piece.color == color && piece.type == PieceType::KING) return true;
        }

        // pawns
        if (color ? (location.rank < 7) : (location.rank > 2)) {
            Rank rank = color ? (location.rank + 1) : (location.rank - 1);
            File file = location.file;
            if (file > A) {
                const Square& square = this->board[rank][file - 1];
                if (square && square->color == color && square->type == PAWN) return true;
            }
            if (file < H) {
                const Square& square = this->board[rank][file + 1];
                if (square && square->color == color && square->type == PAWN) return true;
            }
        }

        // knights
        for (const CoordOffset offset : PIECE_OFFSETS[color].at(PieceType::KNIGHT)) {
            Coord coord = location + offset;
            if (!onBoard(coord) || !board[coord]) continue;

            const Piece& piece = *board[coord];
            if (piece.color == color && piece.type == PieceType::KNIGHT) return true;
        }

        // ranged pieces
        for (int i = 0; i < 8; i++) {
            Range range = ((std::map<PieceType, std::vector<Range>>) PIECE_RANGES)[PieceType::QUEEN][i];
            while (range.hasNext()) {
                Coord coord = location + range.next();
                if (!onBoard(coord)) break;
                Square& square = board[coord];
                if (square) {
                    PieceType type = (i / 4) ? PieceType::BISHOP : PieceType::ROOK;
                    Piece& piece = *square;
                    if (piece.color != color) break;
                    if (piece.type == PieceType::QUEEN || square->type == type) return true;
                    break;
                }
            }
            range.reset();
        }

        return false;
    }

    // returns whether or not the king of color `color` is in check
    bool inCheck(PieceColor color) const {
        const Piece * king = remaining[color].at(PieceType::KING).front();

        return isAttacked(OPPOSITE(color), king->location);
    }

    // execute a move (assumes valid input)
    void move(PieceColor color, Move& move) {
        GameState state = states.top();
        const Rank rank = move.from.rank;
        const File file = move.from.file;
        const Rank rankPrime = move.to.rank;
        const File filePrime = move.to.file;
        Square& source = board[rank][file];
        Square& target = board[rankPrime][filePrime];

        // check for game-ending conditions first
        if (move.mate) {
            if (move.check) result = color ? GameResult::BLACK_WINS : GameResult::WHITE_WINS;
            else result = GameResult::DRAW; // i.e. stalemate (other draw conditions are not yet implemented)
        }

        // handle castling logic
        if (state.canCastle[color][Side::QUEEN] || state.canCastle[color][Side::KING]) {
            switch (source->type) {
            case PieceType::KING: {
                // the king has lost castling rights
                state.canCastle[color][Side::QUEEN] = false;
                state.canCastle[color][Side::KING] = false;

                // if castling
                if (move.moveType == MoveType::CASTLE) {
                    // also move rook
                    int8_t dfile = filePrime - file;
                    int8_t df = dfile / abs(dfile);
                    Side side = (Side) ((filePrime - 1) / 4); // A - D => queenside, E - H => kingside
                    Square& rookSource = board[rank][side ? H : A];
                    Square& rookTarget = board[rank][side ? File::F : File::D];
                    rookSource->location.file = side ? File::F : File::D;
                    rookTarget = rookSource;
                    rookSource = NULL;
                }
                break;
            }
            case PieceType::ROOK:
                // the king can no longer castle on the side of this rook
                if (rank == RANK(color, 1)) {
                    if (file == File::A) state.canCastle[color][Side::QUEEN] = false;
                    else if (file == File::H) state.canCastle[color][Side::KING] = false;
                }
                break;
            }
        }

        // remove previous en passant candidate
        state.passant = (Piece *) NULL;

        // handle special pawn moves (pawn promotion and moving foward two squares)
        if (source->type == PieceType::PAWN) {
            if (move.moveType == MoveType::PROMOTION) {
                source->type = move.promoteTo; // promote the piece copy
                remaining[color][PAWN].remove(source); // remove piece from remaining pawns list
                remaining[color][source->type].push_back(source); // add promoted piece to corresponding remaining list
            } else if (color ? (rank == rankPrime + 2) : (rank + 2 == rankPrime))
                state.passant = source;
        }

        // execute capture(s)
        if (move.captureType == CaptureType::EN_PASSANT) {
            Square& passantSquare = board[rank][filePrime];
            Piece * piece = (Piece *) passantSquare;
            remaining[piece->color][piece->type].remove(piece); // remove piece from piece list
            captured[piece->color][piece->type].push_back(piece); // add piece copy to list of captured pieces
            passantSquare = (Piece*) NULL; // clear passant square
        } else if (move.captureType == CaptureType::NORMAL) {
            Piece * piece = target;
            if(piece->type == ROOK && piece->location.rank == RANK(piece->color, 1)) {
                if(piece->location.file == A) state.canCastle[piece->color][Side::QUEEN] = false;
                else if(piece->location.file == H) state.canCastle[piece->color][Side::KING] = false;
            }
            remaining[piece->color][piece->type].remove(piece);
            captured[piece->color][piece->type].push_back(piece);
        }

        // move source piece to target square
        source->location = move.to;
        target = source;
        source = NULL;

        // push move info and board state to their respective data structures
        moves.push_back(move);
        states.push(state);
    }

    // undo a move (temporarily assumes that `move` is on the top of the `moves` stack)
    void unmove(Move& move) {
        Piece * piece = move.piece;
        PieceColor color = OPPOSITE(piece->color);

        // locate captured piece (if there is one)
        Piece * capturedPiece = (move.captureType == CaptureType::EN_PASSANT) ? captured[color][PAWN].back() : move.capture;

        // undo game-ending changes
        result = GameResult::IN_PROGRESS;

        // undo piece capture(s)
        if (move.captureType != CaptureType::NONE) {
            const PieceType type = capturedPiece->type;
            captured[color][type].remove(capturedPiece);
            remaining[color][type].push_back(capturedPiece);
            (*this)[capturedPiece->location] = capturedPiece;
        }

        switch(move.moveType) {
        // undo a pawn promotion
        case MoveType::PROMOTION: {
            // remove the promoted piece from remaining list
            remaining[piece->color][piece->type].remove(piece);

            // undo promotion
            piece->type = PieceType::PAWN;
            
            // reinsert the pawn into the remaining pawns lists
            remaining[piece->color][PieceType::PAWN].push_back(piece);
            break;
        }
        
        // if move is a castling move, then also move the rook back to its original square
        case MoveType::CASTLE:
            switch (move.to.file) {
                case File::C:
                board[move.from.rank][File::A] = board[move.from.rank][File::D];
                board[move.from.rank][File::D] = NULL;
                break;
                case File::G:
                board[move.from.rank][File::H] = board[move.from.rank][File::F];
                board[move.from.rank][File::F] = NULL;
            }
        }

        // move piece back to its original square
        (*this)[move.from] = piece;
        piece->location = move.from;
        if(move.captureType != CaptureType::NORMAL) (*this)[move.to] = NULL;

        // remove the move from move list
        moves.pop_back();
        states.pop();
    }

    // fills move struct and returns whether move is pseudo-legal
    bool pseudoLegal(PieceColor color, Move& move) {
        GameState& state = states.top();
        const Rank rank = move.from.rank;
        const File file = move.from.file;
        const Rank rankPrime = move.to.rank;
        const File filePrime = move.to.file;
        Square& source = board[rank][file];
        Square& target = board[rankPrime][filePrime];
        Piece * piece = source;

        // fill move struct
        move.piece = piece;
        move.capture = target;

        // whether the moving piece is a pawn moving two squares
        bool moveTwo = false;

        // in bounds
        if (!onBoard(move.from) || !onBoard(move.to)) return false;

        // ensure a piece is on the selected square
        if (!piece) return false;

        // ensure player owns piece
        if (piece->color != color) return false;

        // if target square has a piece on it, then ensure that it's the oponnent's piece and that it's not a king
        // (note: checking for a king shouldn't be strictly necessary but it's a temporary solution to a bug that allowed king captures)
        if (target) {
            if(color == target->color || target->type == PieceType::KING) return false;
            move.captureType = CaptureType::NORMAL;
        }

        int8_t drank = rankPrime - rank; // change in rank
        int8_t dfile = filePrime - file; // change in file
        int8_t dr = drank ? drank / abs(drank) : 0; // normalized change in rank
        int8_t df = dfile ? dfile / abs(dfile) : 0; // normalized change in file
        
        // piece-type-dependent rules
        switch (piece->type) {
        case PieceType::PAWN:
            if (abs(dfile) > 1) return false; // moving by > 1 file
            switch (color) {
            case PieceColor::WHITE:
                if (drank <= 0 || drank > 2) return false;

                // stays on the same file
                if (!dfile) {
                    if (target) return false;
                    if (drank == 2) {
                        if (rank != 2 || board[rank + 1][file]) return false;
                        moveTwo = true;
                    }
                }
                else {
                    if (drank == 2) return false;

                    // check for en passant
                    if(!target) {
                        Square& passantTarget = board[rank][filePrime];
                        if (!passantTarget || passantTarget->color == color || !state.passant || (state.passant != passantTarget)) return false;
                        move.captureType = CaptureType::EN_PASSANT;
                        move.capture = passantTarget;
                    }
                }
                break;
            case PieceColor::BLACK:
                if (drank >= 0 || drank < -2) return false;

                if (!dfile) { // moving fowards
                    if (target) return false;
                    if (drank == -2) {
                        if (rank != 7 || board[rank - 1][file]) return false;
                        moveTwo = true;
                    }
                }
                else { // i.e. a capture
                    if (drank == -2) return false;

                    // check for en passant
                    if (!target) {
                        Square& passantTarget = board[rank][filePrime];
                        if (!passantTarget || passantTarget->color == color || !state.passant || (state.passant != passantTarget)) return false;
                        move.captureType = CaptureType::EN_PASSANT;
                        move.capture = passantTarget;
                    }
                }
                break;
            }
            if(move.to.rank == RANK(color, 8)) {
                move.moveType = MoveType::PROMOTION;
                if(!move.promoteTo) move.promoteTo = PieceType::QUEEN;
            }

            break;
        case PieceType::KNIGHT:
            if (abs(drank * dfile) != 2) return false;
            break;
        case PieceType::BISHOP:
            if (abs(drank) != abs(dfile)) return false;
            for (int i = 1; i < abs(drank); i++) if (board[rank + i * dr][file + i * df]) return false;
            break;
        case PieceType::ROOK:
            if (dr && df) return false;
            if (df) { for (int i = 1; i < abs(dfile); i++) if (board[rank][file + i * df]) return false; }
            else { for (int i = 1; i < abs(drank); i++) if (board[rank + i * dr][file]) return false; }
            break;
        case PieceType::QUEEN:
            if (dr && df) {
                if (abs(drank) != abs(dfile)) return false;
                for (int i = 1; i < abs(dfile); i++) if (board[rank + i * dr][file + i * df]) return false;
            }
            else if (df) { for (int i = 1; i < abs(dfile); i++) if (board[rank][file + i * df]) return false; }
            else { for (int i = 1; i < abs(drank); i++) if (board[rank + i * dr][file]) return false; }
            break;
        case PieceType::KING:
            if (abs(drank) > 1 || abs(dfile) > 2) return false;
            if (abs(dfile) == 2) { // castling
                Side side = (Side) ((filePrime - 1) / 4);
                if (drank || rank != RANK(color, 1) || file != File::E || !state.canCastle[color][side] || inCheck(color)) { return false; }
                switch (filePrime) {
                case File::C: // queenside
                    if (board[rank][File::B]) return false;
                    for (File f = File::C; f <= File::D; f++)
                        if (board[rank][f] || isAttacked(OPPOSITE(color), { f, rank })) return false;
                    break;
                case File::G: // kingside
                    for (File f = File::F; f <= File::G; f++)
                        if (board[rank][f] || isAttacked(OPPOSITE(color), { f, rank })) return false;
                    break;
                default:
                    return false;
                }
                move.moveType = MoveType::CASTLE;
            }
            break;
        }

        return true;
    }

    // returns whether a pseudolegal move `move` is legal
    bool legal(PieceColor color, Move& move) {
        // do move
        this->move(color, move);

        // check for checks
        bool check = inCheck(color);
        move.check = inCheck(OPPOSITE(color));

        // undo move
        this->unmove(move);

        return !check;
    }

    // checks whether a move is legal and whether or not it is a game-ending move
    bool validate(PieceColor color, Move& move) {
        if (!pseudoLegal(color, move) || !legal(color, move)) return false;

        // simulate move
        this->move(color, move);

        // check for checkmate or stalemate (absence of legal responses to move)
        move.mate = true;
        PieceColor enemyColor = OPPOSITE(color);
        for (Move& candidate : getCandidateMoves(enemyColor)) {
            if (pseudoLegal(enemyColor, candidate) && legal(enemyColor, candidate)) {
                move.mate = false;
                break;
            }
        }

        // undo move
        this->unmove(move);

        return true;
    }

    // try to execute a move - returns true upon success
    bool tryMove(PieceColor color, Move& move) {
        // check for validity
        if (!validate(color, move)) return false;
        
        // perform move
        this->move(color, move);
        return true;
    }

    // evaluate terminal node
    int evaluate() {
        // evaluate end of game conditions
        switch(result) {
            case GameResult::DRAW: return 0;
            case GameResult::WHITE_WINS: return INT_MAX; // +M0
            case GameResult::BLACK_WINS: return INT_MIN; // -M0
            case GameResult::IN_PROGRESS: break;
        }
        
        int evaluation = 0;

        // material evaluation
        for(PieceType type : PIECE_TYPES) {
            if(remaining[WHITE].contains(type)) evaluation += remaining[WHITE].at(type).size() * PIECE_VALUES[type];
            if(remaining[BLACK].contains(type)) evaluation -= remaining[BLACK].at(type).size() * PIECE_VALUES[type];
        }

        // positional evaluation
        int pawns_on_file[2][10] = {0}; // number of pawns on each file (padded on both sides)
        int doubled_pawns = 0; // difference in # of doubled pawns
        int isolated_pawns = 0; // difference in # of isolated pawns

        // count number of pawns on each file
        for(PieceColor color : {WHITE, BLACK})
            for(Piece * pawn : remaining[color].at(PAWN))
                pawns_on_file[color][pawn->location.file]++;

        for(File file = A; file <= H; file++) {
            // doubled pawns (doubled pawns 0.5, tripled pawns 1.0, etc.)
            if(pawns_on_file[WHITE][file] > 1) doubled_pawns += pawns_on_file[WHITE][file] - 1;
            if(pawns_on_file[BLACK][file] > 1) doubled_pawns -= pawns_on_file[BLACK][file] - 1;
            
            // isolated pawns
            if(pawns_on_file[WHITE][file] && !pawns_on_file[WHITE][file - 1] && !pawns_on_file[WHITE][file + 1]) isolated_pawns += pawns_on_file[WHITE][file];
            if(pawns_on_file[BLACK][file] && !pawns_on_file[BLACK][file - 1] && !pawns_on_file[BLACK][file + 1]) isolated_pawns -= pawns_on_file[BLACK][file];
        }
        
        // evaluate pawn structures
        evaluation -= 50 * (doubled_pawns + isolated_pawns);

        // evaluate mobility
        evaluation += 10 * (getMoves(WHITE).size() - getMoves(BLACK).size());

        return evaluation;
    }

    // evaluate a position using minimax to depth `depth`
    int evaluatePosition(PieceColor color, unsigned int depth) {
        // evaluate heuristic node
        if(depth == 0 || result != GameResult::IN_PROGRESS) return evaluate();

        // evaluate moves in the current position
        int evaluation = 0;
        switch(color) {
            case WHITE: // maximizing player
            evaluation = INT_MIN;
            for(Move& move : getAlgebraicMoves(color)) {
                int moveEvaluation = evaluateMove(move, depth - 1);
                evaluation = std::max(evaluation, moveEvaluation);
            }
            break;
            case BLACK: // minimizing player
            evaluation = INT_MAX;
            for(Move& move : getAlgebraicMoves(color)) {
                int moveEvaluation = evaluateMove(move, depth - 1);
                evaluation = std::min(evaluation, moveEvaluation);
            }
            break;
        }

        return evaluation;
    }
    
    // evaluate a move using a minimax approach
    int evaluateMove(Move& move, unsigned int depth) {
        // do move
        this->move(move.piece->color, move);

        // evaluate resulting position
        const PieceColor color = move.piece->color;
        move.evaluation = evaluatePosition(OPPOSITE(color), depth);
        if(IS_MATE(move.evaluation) && EVAL_COLOR(move.evaluation) == color) color ? move.evaluation++ : move.evaluation--; // if results in checkmate, increment mate counter

        // undo move
        this->unmove(move);

        // return the evaluation of this move
        return move.evaluation;
    }

    // returns a list of the 'best' moves in the position for `color` by performing a search to depth `depth`
    std::vector<Move> bestMoves(PieceColor color, unsigned int depth) {
        std::vector<Move> bestMoves;

        int bestEvaluation = color ? INT_MAX : INT_MIN;
        for(Move& move : getAlgebraicMoves(color)) {
            int evaluation = this->evaluateMove(move, depth - 1);
            if(BETTER(color, evaluation, bestEvaluation)) {
                bestMoves.clear();
                bestMoves.push_back(move);
                bestEvaluation = evaluation;
            } else if(evaluation == bestEvaluation) bestMoves.push_back(move);
        }

        return bestMoves;
    }

    // returns a randomly selected move from the list of best moves
    Move bestMove(PieceColor color, unsigned int depth) {
        const std::vector<Move> bestMoves = this->bestMoves(color, depth);
        return bestMoves.at(std::rand() % bestMoves.size());
    }

    // generate a reduced algebraic notation string from `move`
    std::string toAlgebraic(Move &move) {
        std::string moveStr = "";

        if (move.moveType == MoveType::CASTLE) moveStr = (move.to.file == File::G) ? "O-O" : "O-O-O";
        else {
            if (move.piece->type != PAWN) moveStr += " NBRQK"[move.piece->type];

            // explicitly state ranks and files iff necessary
            if (move.piece->type != PAWN || move.captureType != CaptureType::NONE) {
                std::list<Move> candidateMoves;
                for (Move& candidate : getMoves(move.piece->color))
                    if (move.to == candidate.to && move.from != candidate.from && move.piece->type == candidate.piece->type) candidateMoves.push_back(candidate);

                bool rank = false; // whether or not there is rank ambiguity
                bool file = move.piece->type == PAWN; // whether or not there is file ambiguity
                for (Move& candidate : candidateMoves) {
                    if (candidate.from.rank == move.from.rank) file = true;
                    if (candidate.from.file == move.from.file) rank = true;
                }
                if(file) moveStr += ('`' + move.from.file);
                if(rank) moveStr += ('0' + move.from.rank);
            }

            if (move.captureType != CaptureType::NONE) moveStr += "x";
            moveStr += ('`' + move.to.file);
            moveStr += ('0' + move.to.rank);
            if (move.moveType == MoveType::PROMOTION) {
                moveStr += "=";
                moveStr += (" NBRQ")[move.promoteTo];
            }
        }

        if (move.check) moveStr += "+#"[move.mate];

        return moveStr;
    }

    // parse algebraic notation string and fill `move`
    bool parseAlgebraic(PieceColor color, Move& move, std::string moveStr) {
        const std::regex MOVE("^[KQRBN]?[a-h]?[1-8]?x?[a-h][1-8](=[QRBN])?[#+]?");

        PieceType pieceType = PieceType::PAWN;

        move.from = {(File) -1, (Rank) -1}; // -1 denotes an indefinite square (e.g. the source square of "Nf3" is unspecified)
        move.to = {(File) 0, (Rank) 0};

        // parse move string
        if (std::regex_match(moveStr, MOVE)) {
            // remove check(mate) modifiers
            if (moveStr.ends_with("+") || moveStr.ends_with("#")) moveStr.pop_back();
            
            // extract the last two characters
            std::string token = moveStr.substr(moveStr.length() - 2, 2);
            moveStr.pop_back();
            moveStr.pop_back();

            // check for promotion clause (=[QRBN])
            if (token[0] == '=') {
                move.moveType = MoveType::PROMOTION;
                move.promoteTo = (PieceType) std::string(" NBRQ").find(token[1]);
                if(move.promoteTo == (PieceType) std::string::npos) move.promoteTo = PieceType::PAWN;

                // consume next token
                token = moveStr.substr(moveStr.length() - 2, 2);
                moveStr.pop_back();
                moveStr.pop_back();
            }

            // parse target square token ([a-h][1-8])
            move.to.rank = token[1] - '0';
            move.to.file = (File) (token[0] - '`');

            if (!moveStr.empty()) {
                // remove capture clause (x)
                if (moveStr.ends_with("x")) moveStr.pop_back();
                if (!moveStr.empty()) {

                    // extract piece type (if applicable)
                    pieceType = (PieceType) std::string(" NBRQK").find(moveStr[0]);
                    if (pieceType != -1) moveStr = moveStr.substr(1);
                    else pieceType = PieceType::PAWN;

                    // extract starting square info
                    switch (moveStr.length()) {
                    case 2:
                        move.from.rank = moveStr[1] - '0';
                        move.from.file = (File) (moveStr[0] - '`');
                        break;
                    case 1:
                        if ('a' <= moveStr[0] && moveStr[0] <= 'h') move.from.file = (File) (moveStr[0] - '`');
                        if ('1' <= moveStr[0] && moveStr[0] <= '8') move.from.rank = (Rank) (moveStr[0] - '0');
                        break;
                    }
                }
            }
        } else if (moveStr == "O-O") {
            pieceType = PieceType::KING;
            move.from.rank = RANK(color, 1);
            move.from.file = File::E;
            move.to.rank = move.from.rank;
            move.to.file = File::G;
            move.moveType = MoveType::CASTLE;
        } else if (moveStr == "O-O-O") {
            pieceType = PieceType::KING;
            move.from.rank = RANK(color, 1);
            move.from.file = File::E;
            move.to.rank = move.from.rank;
            move.to.file = File::C;
            move.moveType = MoveType::CASTLE;
        }

        // check whether move is legal by searching for all legal moves (temporary solution)
        std::list<Move> candidates;
        for (Move candidate : getMoves(color)) {
            if (move.from.rank != (Rank) -1 && move.from.rank != candidate.from.rank) continue;
            if (move.from.file != (File) -1 && move.from.file != candidate.from.file) continue;
            if (move.promoteTo != candidate.promoteTo) continue;
            if (move.to == candidate.to && candidate.piece->type == pieceType) candidates.push_back(candidate);
        }

        // make a deep copy of the move
        if (candidates.size() == 1) {
            move = candidates.front();
            move.algebraic = (char *) malloc(moveStr.length() + 1);
            strcpy(move.algebraic, toAlgebraic(move).c_str());
            return true;
        }

        return false;
    }

    // Parse and execute move in algebraic notation
    bool parseMove(PieceColor color, std::string moveStr, bool debug) {
        Move move;

        if(parseAlgebraic(color, move, moveStr)) {
            this->move(color, move);
            return true;
        }
        return false;
    }

    // displays a list of moves played this game
    void displayMoves() const {
        int n = 1;

        std::list<Move>::const_iterator begin = moves.begin();
        if(!moves.empty() && moves.front().piece->color == PieceColor::BLACK) {
            std::cout << "1... " << moves.front().algebraic << " ";
            begin++;
            n = 3;
        }

        for (std::list<Move>::const_iterator move = begin; move != moves.end(); move++) {
            bool color = n % 2;
            if (color) std::cout << n / 2 + 1 << ". ";
            std::cout << (*move).algebraic << " ";
            n++;
        }

        std::cout << std::endl;
    }

    // display board (debug: blue = passant candidate, red = castling rights)
    void display(bool debug = false) {
        const GameState& state = states.top();

        // clear screen
        std::cout << "\x1b[0;0H" << "\x1b[0J";

        // print board
        std::cout << "   a  b  c  d  e  f  g  h\n";
        for (uint16_t rank = 8; rank >= 1; rank--) {
            std::cout << rank << " ";
            for (uint16_t file = 1; file <= 8; file++) {
                const Coord coord = {(File) file, (Rank) rank};
                const Square &square = board[rank][file];
                std::string tile_color = bg[colors[rank][file]];
                std::string piece_color = square ? fg[square->color] : "";
                const char * piece = square ? PIECES[square->color][square->type] : " ";
                if(!moves.empty() && moves.back().from == coord) tile_color = "\x1b[46m";
                if(!moves.empty() && moves.back().to == coord) tile_color = "\x1b[106m";
                if(debug) {
                    if ((rank == 1 || rank == 8) && (file == A || file == H) && state.canCastle[rank == 8][file == H]) tile_color = "\x1b[41m";
                    if (square && state.passant == square) tile_color = "\x1b[44m";
                }
                std::cout << tile_color << " " << piece_color << piece << " \x1b[0m";
            }
            std::cout << " " << rank << "\n";
        }
        std::cout << "   a  b  c  d  e  f  g  h\n\n";
        
        // display moves
        displayMoves();

        // display check message if in check
        if (result == GameResult::IN_PROGRESS && inCheck(OPPOSITE(toPlay))) std::cout << "Check!\n";
    }
};