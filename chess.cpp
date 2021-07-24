/*
This is a complete remake of niklasf's python-chess in C++
The original version can be found here: https://github.com/niklasf/python-chess
*/

#include <string>
#include <unordered_map>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <vector>
#include <iostream>

using namespace std;

typedef string _EnPassantSpec;

typedef bool Color;
const Color COLORS[] = {true, false}, WHITE = true, BLACK = false;
const string COLOR_NAMES[] = {"black", "white"};

typedef int PieceType;
const PieceType PIECE_TYPES[] = {1, 2, 3, 4, 5, 6}, PAWN = 1, KNIGHT = 2, BISHOP = 3, ROOK = 4, QUEEN = 5, KING = 6;
char PIECE_SYMBOLS[] = {'\0', 'p', 'n', 'b', 'r', 'q', 'k'};
string PIECE_NAMES[] = {"", "pawn", "knight", "bishop", "rook", "queen", "king"};

char piece_symbol(PieceType piece_type)
{
    return PIECE_SYMBOLS[piece_type];
}

string piece_name(PieceType piece_type)
{
    return PIECE_NAMES[piece_type];
}

const unordered_map<char, string> UNICODE_PIECE_SYMBOLS = {
    {'R', "♖"},
    {'r', "♜"},
    {'N', "♘"},
    {'n', "♞"},
    {'B', "♗"},
    {'b', "♝"},
    {'Q', "♕"},
    {'q', "♛"},
    {'K', "♔"},
    {'k', "♚"},
    {'P', "♙"},
    {'p', "♟"},
};

const char FILE_NAMES[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};

const char RANK_NAMES[] = {'1', '2', '3', '4', '5', '6', '7', '8'};

const string STARTING_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
// The FEN for the standard chess starting position.

const string STARTING_BOARD_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";
// The board part of the FEN for the standard chess starting position.

class Status
{
public:
    const static int VALID = 0;
    const static int NO_WHITE_KING = 1 << 0;
    const static int NO_BLACK_KING = 1 << 1;
    const static int TOO_MANY_KINGS = 1 << 2;
    const static int TOO_MANY_WHITE_PAWNS = 1 << 3;
    const static int TOO_MANY_BLACK_PAWNS = 1 << 4;
    const static int PAWNS_ON_BACKRANK = 1 << 5;
    const static int TOO_MANY_WHITE_PIECES = 1 << 6;
    const static int TOO_MANY_BLACK_PIECES = 1 << 7;
    const static int BAD_CASTLING_RIGHTS = 1 << 8;
    const static int INVALID_EP_SQUARE = 1 << 9;
    const static int OPPOSITE_CHECK = 1 << 10;
    const static int EMPTY = 1 << 11;
    const static int RACE_CHECK = 1 << 12;
    const static int RACE_OVER = 1 << 13;
    const static int RACE_MATERIAL = 1 << 14;
    const static int TOO_MANY_CHECKERS = 1 << 15;
    const static int IMPOSSIBLE_CHECK = 1 << 16;
};

const int STATUS_VALID = Status::VALID;
const int STATUS_NO_WHITE_KING = Status::NO_WHITE_KING;
const int STATUS_NO_BLACK_KING = Status::NO_BLACK_KING;
const int STATUS_TOO_MANY_KINGS = Status::TOO_MANY_KINGS;
const int STATUS_TOO_MANY_WHITE_PAWNS = Status::TOO_MANY_WHITE_PAWNS;
const int STATUS_TOO_MANY_BLACK_PAWNS = Status::TOO_MANY_BLACK_PAWNS;
const int STATUS_PAWNS_ON_BACKRANK = Status::PAWNS_ON_BACKRANK;
const int STATUS_TOO_MANY_WHITE_PIECES = Status::TOO_MANY_WHITE_PIECES;
const int STATUS_TOO_MANY_BLACK_PIECES = Status::TOO_MANY_BLACK_PIECES;
const int STATUS_BAD_CASTLING_RIGHTS = Status::BAD_CASTLING_RIGHTS;
const int STATUS_INVALID_EP_SQUARE = Status::INVALID_EP_SQUARE;
const int STATUS_OPPOSITE_CHECK = Status::OPPOSITE_CHECK;
const int STATUS_EMPTY = Status::EMPTY;
const int STATUS_RACE_CHECK = Status::RACE_CHECK;
const int STATUS_RACE_OVER = Status::RACE_OVER;
const int STATUS_RACE_MATERIAL = Status::RACE_MATERIAL;
const int STATUS_TOO_MANY_CHECKERS = Status::TOO_MANY_CHECKERS;
const int STATUS_IMPOSSIBLE_CHECK = Status::IMPOSSIBLE_CHECK;

class Termination
{
    // Enum with reasons for a game to be over.

    const static int CHECKMATE = 1;
    // See :func:`chess.Board.is_checkmate()`.
    const static int CSTALEMATE = 2;
    // See :func:`chess.Board.is_stalemate()`.
    const static int CINSUFFICIENT_MATERIAL = 3;
    // See :func:`chess.Board.is_insufficient_material()`.
    const static int CSEVENTYFIVE_MOVES = 4;
    // See :func:`chess.Board.is_seventyfive_moves()`.
    const static int CFIVEFOLD_REPETITION = 5;
    // See :func:`chess.Board.is_fivefold_repetition()`.
    const static int CFIFTY_MOVES = 6;
    // See :func:`chess.Board.can_claim_fifty_moves()`.
    const static int CTHREEFOLD_REPETITION = 7;
    // See :func:`chess.Board.can_claim_threefold_repetition()`.
    const static int CVARIANT_WIN = 8;
    // See :func:`chess.Board.is_variant_win()`.
    const static int CVARIANT_LOSS = 9;
    // See :func:`chess.Board.is_variant_loss()`.
    const static int CVARIANT_DRAW = 10;
    // See :func:`chess.Board.is_variant_draw()`.
};

class Outcome
{
    /*
    Information about the outcome of an ended game, usually obtained from
    :func:`chess.Board.outcome()`.
    */

public:
    Termination termination;
    int winner;

    Outcome(Termination termination, int winner)
    {
        termination = termination;
        // The reason for the game to have ended.
        winner = winner;
        // The winning color or ``-1`` if drawn.
    }

    string result()
    {
        // Returns ``1-0``, ``0-1`` or ``1/2-1/2``.
        return winner == -1 ? "1/2-1/2" : (winner ? "1-0" : "0-1");
    }
};

typedef int Square;
const Square SQUARES[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63}, A1 = 0, B1 = 1, C1 = 2, D1 = 3, E1 = 4, F1 = 5, G1 = 6, H1 = 7, A2 = 8, B2 = 9, C2 = 10, D2 = 11, E2 = 12, F2 = 13, G2 = 14, H2 = 15, A3 = 16, B3 = 17, C3 = 18, D3 = 19, E3 = 20, F3 = 21, G3 = 22, H3 = 23, A4 = 24, B4 = 25, C4 = 26, D4 = 27, E4 = 28, F4 = 29, G4 = 30, H4 = 31, A5 = 32, B5 = 33, C5 = 34, D5 = 35, E5 = 36, F5 = 37, G5 = 38, H5 = 39, A6 = 40, B6 = 41, C6 = 42, D6 = 43, E6 = 44, F6 = 45, G6 = 46, H6 = 47, A7 = 48, B7 = 49, C7 = 50, D7 = 51, E7 = 52, F7 = 53, G7 = 54, H7 = 55, A8 = 56, B8 = 57, C8 = 58, D8 = 59, E8 = 60, F8 = 61, G8 = 62, H8 = 63;

const string SQUARE_NAMES[] = {"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2", "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3", "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4", "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5", "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6", "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7", "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"};

Square parse_square(string name)
{
    /*
    Gets the square index for the given square *name*
    (e.g., ``a1`` returns ``0``).

    :raises: :exc:`invalid_argument` if the square name is invalid.
    */
    const auto it = find(SQUARE_NAMES, SQUARE_NAMES + sizeof(SQUARE_NAMES) / sizeof(SQUARE_NAMES[0]), name);

    if (it == end(SQUARE_NAMES))
        throw invalid_argument("");

    return distance(SQUARE_NAMES, it);
}

string square_name(Square square)
{
    // Gets the name of the square, like ``a3``.
    return SQUARE_NAMES[square];
}

Square square(int file_index, int rank_index)
{
    // Gets a square number by file and rank index.
    return rank_index * 8 + file_index;
}

int square_file(Square square)
{
    // Gets the file index of the square where ``0`` is the a-file.
    return square & 7;
}

int square_rank(Square square)
{
    // Gets the rank index of the square where ``0`` is the first rank.
    return square >> 3;
}

int square_distance(Square a, Square b)
{
    /*
    Gets the distance (i.e., the number of king steps) from square *a* to *b*.
    */
    return max(abs(square_file(a) - square_file(b)), abs(square_rank(a) - square_rank(b)));
}

Square square_mirror(Square square)
{
    // Mirrors the square vertically.
    return square ^ 0x38;
}

const Square SQUARES_180[] = {square_mirror(0), square_mirror(1), square_mirror(2), square_mirror(3), square_mirror(4), square_mirror(5), square_mirror(6), square_mirror(7), square_mirror(8), square_mirror(9), square_mirror(10), square_mirror(11), square_mirror(12), square_mirror(13), square_mirror(14), square_mirror(15), square_mirror(16), square_mirror(17), square_mirror(18), square_mirror(19), square_mirror(20), square_mirror(21), square_mirror(22), square_mirror(23), square_mirror(24), square_mirror(25), square_mirror(26), square_mirror(27), square_mirror(28), square_mirror(29), square_mirror(30), square_mirror(31), square_mirror(32), square_mirror(33), square_mirror(34), square_mirror(35), square_mirror(36), square_mirror(37), square_mirror(38), square_mirror(39), square_mirror(40), square_mirror(41), square_mirror(42), square_mirror(43), square_mirror(44), square_mirror(45), square_mirror(46), square_mirror(47), square_mirror(48), square_mirror(49), square_mirror(50), square_mirror(51), square_mirror(52), square_mirror(53), square_mirror(54), square_mirror(55), square_mirror(56), square_mirror(57), square_mirror(58), square_mirror(59), square_mirror(60), square_mirror(61), square_mirror(62), square_mirror(63)};

typedef long Bitboard;
const Bitboard BB_EMPTY = 0;
const Bitboard BB_ALL = 0xffffffffffffffff;

const Bitboard BB_SQUARES[] = {1L << 0, 1L << 1, 1L << 2, 1L << 3, 1L << 4, 1L << 5, 1L << 6, 1L << 7, 1L << 8, 1L << 9, 1L << 10, 1L << 11, 1L << 12, 1L << 13, 1L << 14, 1L << 15, 1L << 16, 1L << 17, 1L << 18, 1L << 19, 1L << 20, 1L << 21, 1L << 22, 1L << 23, 1L << 24, 1L << 25, 1L << 26, 1L << 27, 1L << 28, 1L << 29, 1L << 30, 1L << 31, 1L << 32, 1L << 33, 1L << 34, 1L << 35, 1L << 36, 1L << 37, 1L << 38, 1L << 39, 1L << 40, 1L << 41, 1L << 42, 1L << 43, 1L << 44, 1L << 45, 1L << 46, 1L << 47, 1L << 48, 1L << 49, 1L << 50, 1L << 51, 1L << 52, 1L << 53, 1L << 54, 1L << 55, 1L << 56, 1L << 57, 1L << 58, 1L << 59, 1L << 60, 1L << 61, 1L << 62, 1L << 63}, BB_A1 = 1L << 0, BB_B1 = 1L << 1, BB_C1 = 1L << 2, BB_D1 = 1L << 3, BB_E1 = 1L << 4, BB_F1 = 1L << 5, BB_G1 = 1L << 6, BB_H1 = 1L << 7, BB_A2 = 1L << 8, BB_B2 = 1L << 9, BB_C2 = 1L << 10, BB_D2 = 1L << 11, BB_E2 = 1L << 12, BB_F2 = 1L << 13, BB_G2 = 1L << 14, BB_H2 = 1L << 15, BB_A3 = 1L << 16, BB_B3 = 1L << 17, BB_C3 = 1L << 18, BB_D3 = 1L << 19, BB_E3 = 1L << 20, BB_F3 = 1L << 21, BB_G3 = 1L << 22, BB_H3 = 1L << 23, BB_A4 = 1L << 24, BB_B4 = 1L << 25, BB_C4 = 1L << 26, BB_D4 = 1L << 27, BB_E4 = 1L << 28, BB_F4 = 1L << 29, BB_G4 = 1L << 30, BB_H4 = 1L << 31, BB_A5 = 1L << 32, BB_B5 = 1L << 33, BB_C5 = 1L << 34, BB_D5 = 1L << 35, BB_E5 = 1L << 36, BB_F5 = 1L << 37, BB_G5 = 1L << 38, BB_H5 = 1L << 39, BB_A6 = 1L << 40, BB_B6 = 1L << 41, BB_C6 = 1L << 42, BB_D6 = 1L << 43, BB_E6 = 1L << 44, BB_F6 = 1L << 45, BB_G6 = 1L << 46, BB_H6 = 1L << 47, BB_A7 = 1L << 48, BB_B7 = 1L << 49, BB_C7 = 1L << 50, BB_D7 = 1L << 51, BB_E7 = 1L << 52, BB_F7 = 1L << 53, BB_G7 = 1L << 54, BB_H7 = 1L << 55, BB_A8 = 1L << 56, BB_B8 = 1L << 57, BB_C8 = 1L << 58, BB_D8 = 1L << 59, BB_E8 = 1L << 60, BB_F8 = 1L << 61, BB_G8 = 1L << 62, BB_H8 = 1L << 63;

const Bitboard BB_CORNERS = BB_A1 | BB_H1 | BB_A8 | BB_H8;
const Bitboard BB_CENTER = BB_D4 | BB_E4 | BB_D5 | BB_E5;

const Bitboard BB_LIGHT_SQUARES = 0x55aa55aa55aa55aa;
const Bitboard BB_DARK_SQUARES = 0xaa55aa55aa55aa55;

const Bitboard BB_FILES[] = {0x0101010101010101 << 0, 0x0101010101010101 << 1, 0x0101010101010101 << 2, 0x0101010101010101 << 3, 0x0101010101010101 << 4, 0x0101010101010101 << 5, 0x0101010101010101 << 6, 0x0101010101010101 << 7}, BB_FILE_A = 0x0101010101010101 << 0, BB_FILE_B = 0x0101010101010101 << 1, BB_FILE_C = 0x0101010101010101 << 2, BB_FILE_D = 0x0101010101010101 << 3, BB_FILE_E = 0x0101010101010101 << 4, BB_FILE_F = 0x0101010101010101 << 5, BB_FILE_G = 0x0101010101010101 << 6, BB_FILE_H = 0x0101010101010101 << 7;

const Bitboard BB_RANKS[] = {0xffL << (8 * 0), 0xffL << (8 * 1), 0xffL << (8 * 2), 0xffL << (8 * 3), 0xffL << (8 * 4), 0xffL << (8 * 5), 0xffL << (8 * 6), 0xffL << (8 * 7)}, BB_RANK_1 = 0xffL << (8 * 0), BB_RANK_2 = 0xffL << (8 * 1), BB_RANK_3 = 0xffL << (8 * 2), BB_RANK_4 = 0xffL << (8 * 3), BB_RANK_5 = 0xffL << (8 * 4), BB_RANK_6 = 0xffL << (8 * 5), BB_RANK_7 = 0xffL << (8 * 6), BB_RANK_8 = 0xffL << (8 * 7);

const Bitboard BB_BACKRANKS = BB_RANK_1 | BB_RANK_8;

int lsb(Bitboard bb)
{
    return log2(bb & -bb);
}

function<Square(Bitboard)> scan_forward = [](bb)
{
    if (!bb)
        return [=]() mutable
        {
            return -1;
        };

    Bitboard r = bb & -bb;
    bb ^= r;

    return [=]() mutable
    {
        return log2(r);
    };
}

int
msb(Bitboard bb)
{
    return log2(bb);
}

function<Square(Bitboard)> scan_reversed = [](bb)
{
    if (!bb)
        return [=]() mutable
        {
            return -1;
        };

    Bitboard r = log2(bb);
    bb ^= BB_SQUARES[r];

    return [=]() mutable
    {
        return r;
    };
}

// popcount: Callable[[Bitboard], int] = getattr(int, "bit_count", lambda bb: bin(bb).count("1"))

Bitboard
flip_vertical(Bitboard bb)
{
    // https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating#FlipVertically
    bb = ((bb >> 8) & 0x00ff00ff00ff00ff) | ((bb & 0x00ff00ff00ff00ff) << 8);
    bb = ((bb >> 16) & 0x0000ffff0000ffff) | ((bb & 0x0000ffff0000ffff) << 16);
    bb = (bb >> 32) | ((bb & 0x00000000ffffffff) << 32);
    return bb;
}

Bitboard flip_horizontal(Bitboard bb)
{
    // https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating#MirrorHorizontally
    bb = ((bb >> 1) & 0x5555555555555555) | ((bb & 0x5555555555555555) << 1);
    bb = ((bb >> 2) & 0x3333333333333333) | ((bb & 0x3333333333333333) << 2);
    bb = ((bb >> 4) & 0x0f0f0f0f0f0f0f0f) | ((bb & 0x0f0f0f0f0f0f0f0f) << 4);
    return bb;
}

Bitboard flip_diagonal(Bitboard bb)
{
    // https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating#FlipabouttheDiagonal
    Bitboard t = (bb ^ (bb << 28)) & 0x0f0f0f0f00000000;
    bb = bb ^ (t ^ (t >> 28));
    t = (bb ^ (bb << 14)) & 0x3333000033330000;
    bb = bb ^ (t ^ (t >> 14));
    t = (bb ^ (bb << 7)) & 0x5500550055005500;
    bb = bb ^ (t ^ (t >> 7));
    return bb;
}

Bitboard flip_anti_diagonal(Bitboard bb)
{
    // https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating#FlipabouttheAntidiagonal
    Bitboard t = bb ^ (bb << 36);
    bb = bb ^ ((t ^ (bb >> 36)) & 0xf0f0f0f00f0f0f0f);
    t = (bb ^ (bb << 18)) & 0xcccc0000cccc0000;
    bb = bb ^ (t ^ (t >> 18));
    t = (bb ^ (bb << 9)) & 0xaa00aa00aa00aa00;
    bb = bb ^ (t ^ (t >> 9));
    return bb;
}

Bitboard shift_down(Bitboard b)
{
    return b >> 8;
}

Bitboard shift_2_down(Bitboard b)
{
    return b >> 16;
}

Bitboard shift_up(Bitboard b)
{
    return (b << 8) & BB_ALL;
}

Bitboard shift_2_up(Bitboard b)
{
    return (b << 16) & BB_ALL;
}

Bitboard shift_right(Bitboard b)
{
    return (b << 1) & ~BB_FILE_A & BB_ALL;
}

Bitboard shift_2_right(Bitboard b)
{
    return (b << 2) & ~BB_FILE_A & ~BB_FILE_B & BB_ALL;
}

Bitboard shift_left(Bitboard b)
{
    return (b >> 1) & ~BB_FILE_H;
}

Bitboard shift_2_left(Bitboard b)
{
    return (b >> 2) & ~BB_FILE_G & ~BB_FILE_H;
}

Bitboard shift_up_left(Bitboard b)
{
    return (b << 7) & ~BB_FILE_H & BB_ALL;
}

Bitboard shift_up_right(Bitboard b)
{
    return (b << 9) & ~BB_FILE_A & BB_ALL;
}

Bitboard shift_down_left(Bitboard b)
{
    return (b >> 9) & ~BB_FILE_H;
}

Bitboard shift_down_right(Bitboard b)
{
    return (b >> 7) & ~BB_FILE_A;
}

Bitboard _sliding_attacks(Square square, Bitboard occupied, vector<int> deltas)
{
    Bitboard attacks = BB_EMPTY;

    for (int delta : deltas)
    {
        Square sq = square;

        while (true)
        {
            sq += delta;
            if (!(0 <= sq && sq < 64) || square_distance(sq, sq - delta) > 2)
                break;

            attacks |= BB_SQUARES[sq];

            if (occupied & BB_SQUARES[sq])
                break;
        }
    }

    return attacks;
}

Bitboard _step_attacks(Square square, vector<int> deltas)
{
    return _sliding_attacks(square, BB_ALL, deltas);
}

// const Bitboard BB_KNIGHT_ATTACKS[] = [_step_attacks(sq, [17, 15, 10, 6, -17, -15, -10, -6]) for sq in SQUARES]
// const Bitboard BB_KING_ATTACKS[] = [_step_attacks(sq, [9, 8, 7, 1, -9, -8, -7, -1]) for sq in SQUARES]
// const Bitboard BB_PAWN_ATTACKS[][] = [[_step_attacks(sq, deltas) for sq in SQUARES] for deltas in [[-7, -9], [7, 9]]]

Bitboard _edges(Square square)
{
    return (((BB_RANK_1 | BB_RANK_8) & ~BB_RANKS[square_rank(square)]) |
            ((BB_FILE_A | BB_FILE_H) & ~BB_FILES[square_file(square)]));
}

int main()
{
}