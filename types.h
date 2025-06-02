#pragma once

#include <memory>

/* user-defined types */

typedef uint8_t Rank;

enum File { NONE, A, B, C, D, E, F, G, H };
enum SquareColor { LIGHT, DARK };
enum PieceColor { WHITE, BLACK };
enum PieceType { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };
enum class MoveType { NORMAL, CASTLE, PROMOTION };
enum class CaptureType { NONE, NORMAL, EN_PASSANT };
enum GameResult { IN_PROGRESS, DRAW, WHITE_WINS, BLACK_WINS };

// represents a coordinate
struct Coord {
    File file;
    Rank rank;
};

// represents an offset from a coordinate
struct CoordOffset {
    int8_t dfile;
    int8_t drank;
};

// represents a piece
struct Piece {
    PieceColor color;
    PieceType type;
    Coord location;
};

// alias for piece pointers
typedef Piece * Square;

// represents info about the current game state
struct GameState {
    bool canCastle[2][2]; // on which side(s) of the board each color has castling rights
    Piece* passant; // candidate for being captured en passant
};

// represents a move in memory
struct Move {
    char * algebraic; // algebraic notation move description
    Piece * piece; // the moving piece
    Piece * capture; // the piece captured by the move (if there is one)
    Coord from, to; // starting and ending positions
    MoveType moveType; // type of move (normal, castling, promotion)
    CaptureType captureType; // type of capture (none, normal, en passant)
    PieceType promoteTo; // type of piece to promote to (PAWN i.e. NULL implies no promotion)
    bool check; // whether the move causes check (to the opponent)
    bool mate; // whether the move causes either checkmate or stalemate
    int evaluation; // numerical evaluation of move (not always calculated)

    Move() {
        algebraic = NULL;
        piece = NULL;
        capture = NULL;
        from = {(File) -1, (Rank) -1};
        to = {(File) 0, (Rank) 0};
        moveType = MoveType::NORMAL;
        captureType = CaptureType::NONE;
        promoteTo = PieceType::PAWN;
        check = false;
        mate = false;
        evaluation = 0;
    }

    Move(const Move& move) {
        memcpy(this, &move, sizeof(Move));

        // if move has an algebraic notation string, make a deep copy of it to avoid double frees
        if(move.algebraic) {
            size_t size = strlen(move.algebraic) + 1;
            algebraic = (char *) malloc(size);
            strcpy(algebraic, move.algebraic);
        }
    }

    ~Move() {
        if(algebraic) {
            free(algebraic);
            algebraic = NULL;
        }
    }
};

/* operator overloads */
PieceColor operator!(PieceColor color) { return (PieceColor) (!(bool) color); }

File operator+(File file, int8_t offset) { return (File) ((uint8_t) file + offset); }
File operator+(File file, int offset) { return file + (int8_t) offset; }
File operator++(File& file, int) { return (file = (File) (file + (int8_t) 1)); }

bool operator==(const Coord& coord1, const Coord& coord2) {
    return (coord1.rank == coord2.rank) && (coord1.file == coord2.file);
}

std::ostream& operator<<(std::ostream& stream, Coord coord) {
    return stream << (char)(0x60 + coord.file) << coord.rank;
}

Coord operator+(Coord coord, CoordOffset offset) {
    return { (File) ((int8_t) coord.file + offset.dfile), (Rank) ((int8_t) coord.rank + offset.drank) };
}

Coord operator-(Coord coord, CoordOffset offset) {
    return { (File) ((int8_t) coord.file - offset.dfile), (Rank) ((int8_t) coord.rank - offset.drank) };
}

CoordOffset operator-(CoordOffset offset) {
    return { (int8_t)-offset.dfile, (int8_t)-offset.drank };
}

CoordOffset operator+(CoordOffset offset1, CoordOffset offset2) {
    return { (int8_t)(offset1.dfile + offset2.dfile), (int8_t)(offset1.drank + offset2.drank) };
}

CoordOffset operator-(CoordOffset offset1, CoordOffset offset2) {
    return { (int8_t)(offset1.dfile - offset2.dfile), (int8_t)(offset1.drank - offset2.drank) };
}

bool operator==(const Piece& p1, const Piece& p2) {
    return (p1.color == p2.color) && (p1.location == p2.location) && (p1.type == p2.type);
}

/* constants */
const PieceType PIECE_TYPES[6] = {
    PieceType::PAWN,
    PieceType::KNIGHT,
    PieceType::BISHOP,
    PieceType::ROOK,
    PieceType::QUEEN,
    PieceType::KING
};

// represents a linear range of coordinate offsets (used for bishops, rooks, and queens)
class Range {
private:
    CoordOffset start, step, _next;
    int i = 0;

public:
    Range(CoordOffset start, CoordOffset step) : start(start), _next(start), step(step) {}

    CoordOffset next() {
        CoordOffset next = _next;
        _next = _next + step;
        i++;
        return next;
    }

    bool hasNext() const {
        return (i < 8);
    }
    
    void reset() {
        _next = start;
        i = 0;
    }
};
