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
#include <bit>
#include <tuple>

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

enum Status
{
    VALID = 0,
    NO_WHITE_KING = 1 << 0,
    NO_BLACK_KING = 1 << 1,
    TOO_MANY_KINGS = 1 << 2,
    TOO_MANY_WHITE_PAWNS = 1 << 3,
    TOO_MANY_BLACK_PAWNS = 1 << 4,
    PAWNS_ON_BACKRANK = 1 << 5,
    TOO_MANY_WHITE_PIECES = 1 << 6,
    TOO_MANY_BLACK_PIECES = 1 << 7,
    BAD_CASTLING_RIGHTS = 1 << 8,
    INVALID_EP_SQUARE = 1 << 9,
    OPPOSITE_CHECK = 1 << 10,
    EMPTY = 1 << 11,
    RACE_CHECK = 1 << 12,
    RACE_OVER = 1 << 13,
    RACE_MATERIAL = 1 << 14,
    TOO_MANY_CHECKERS = 1 << 15,
    IMPOSSIBLE_CHECK = 1 << 16
};

const Status STATUS_VALID = Status::VALID;
const Status STATUS_NO_WHITE_KING = Status::NO_WHITE_KING;
const Status STATUS_NO_BLACK_KING = Status::NO_BLACK_KING;
const Status STATUS_TOO_MANY_KINGS = Status::TOO_MANY_KINGS;
const Status STATUS_TOO_MANY_WHITE_PAWNS = Status::TOO_MANY_WHITE_PAWNS;
const Status STATUS_TOO_MANY_BLACK_PAWNS = Status::TOO_MANY_BLACK_PAWNS;
const Status STATUS_PAWNS_ON_BACKRANK = Status::PAWNS_ON_BACKRANK;
const Status STATUS_TOO_MANY_WHITE_PIECES = Status::TOO_MANY_WHITE_PIECES;
const Status STATUS_TOO_MANY_BLACK_PIECES = Status::TOO_MANY_BLACK_PIECES;
const Status STATUS_BAD_CASTLING_RIGHTS = Status::BAD_CASTLING_RIGHTS;
const Status STATUS_INVALID_EP_SQUARE = Status::INVALID_EP_SQUARE;
const Status STATUS_OPPOSITE_CHECK = Status::OPPOSITE_CHECK;
const Status STATUS_EMPTY = Status::EMPTY;
const Status STATUS_RACE_CHECK = Status::RACE_CHECK;
const Status STATUS_RACE_OVER = Status::RACE_OVER;
const Status STATUS_RACE_MATERIAL = Status::RACE_MATERIAL;
const Status STATUS_TOO_MANY_CHECKERS = Status::TOO_MANY_CHECKERS;
const Status STATUS_IMPOSSIBLE_CHECK = Status::IMPOSSIBLE_CHECK;

enum Termination
{
    // Enum with reasons for a game to be over.

    CHECKMATE = 1,
    // See :func:`chess.Board.is_checkmate()`.
    CSTALEMATE = 2,
    // See :func:`chess.Board.is_stalemate()`.
    CINSUFFICIENT_MATERIAL = 3,
    // See :func:`chess.Board.is_insufficient_material()`.
    CSEVENTYFIVE_MOVES = 4,
    // See :func:`chess.Board.is_seventyfive_moves()`.
    CFIVEFOLD_REPETITION = 5,
    // See :func:`chess.Board.is_fivefold_repetition()`.
    CFIFTY_MOVES = 6,
    // See :func:`chess.Board.can_claim_fifty_moves()`.
    CTHREEFOLD_REPETITION = 7,
    // See :func:`chess.Board.can_claim_threefold_repetition()`.
    CVARIANT_WIN = 8,
    // See :func:`chess.Board.is_variant_win()`.
    CVARIANT_LOSS = 9,
    // See :func:`chess.Board.is_variant_loss()`.
    CVARIANT_DRAW = 10
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

typedef unsigned long Bitboard;
const Bitboard BB_EMPTY = 0;
const Bitboard BB_ALL = 0xffffffffffffffff;

const Bitboard BB_SQUARES[] = {1UL << 0, 1UL << 1, 1UL << 2, 1UL << 3, 1UL << 4, 1UL << 5, 1UL << 6, 1UL << 7, 1UL << 8, 1UL << 9, 1UL << 10, 1UL << 11, 1UL << 12, 1UL << 13, 1UL << 14, 1UL << 15, 1UL << 16, 1UL << 17, 1UL << 18, 1UL << 19, 1UL << 20, 1UL << 21, 1UL << 22, 1UL << 23, 1UL << 24, 1UL << 25, 1UL << 26, 1UL << 27, 1UL << 28, 1UL << 29, 1UL << 30, 1UL << 31, 1UL << 32, 1UL << 33, 1UL << 34, 1UL << 35, 1UL << 36, 1UL << 37, 1UL << 38, 1UL << 39, 1UL << 40, 1UL << 41, 1UL << 42, 1UL << 43, 1UL << 44, 1UL << 45, 1UL << 46, 1UL << 47, 1UL << 48, 1UL << 49, 1UL << 50, 1UL << 51, 1UL << 52, 1UL << 53, 1UL << 54, 1UL << 55, 1UL << 56, 1UL << 57, 1UL << 58, 1UL << 59, 1UL << 60, 1UL << 61, 1UL << 62, 1UL << 63}, BB_A1 = 1UL << 0, BB_B1 = 1UL << 1, BB_C1 = 1UL << 2, BB_D1 = 1UL << 3, BB_E1 = 1UL << 4, BB_F1 = 1UL << 5, BB_G1 = 1UL << 6, BB_H1 = 1UL << 7, BB_A2 = 1UL << 8, BB_B2 = 1UL << 9, BB_C2 = 1UL << 10, BB_D2 = 1UL << 11, BB_E2 = 1UL << 12, BB_F2 = 1UL << 13, BB_G2 = 1UL << 14, BB_H2 = 1UL << 15, BB_A3 = 1UL << 16, BB_B3 = 1UL << 17, BB_C3 = 1UL << 18, BB_D3 = 1UL << 19, BB_E3 = 1UL << 20, BB_F3 = 1UL << 21, BB_G3 = 1UL << 22, BB_H3 = 1UL << 23, BB_A4 = 1UL << 24, BB_B4 = 1UL << 25, BB_C4 = 1UL << 26, BB_D4 = 1UL << 27, BB_E4 = 1UL << 28, BB_F4 = 1UL << 29, BB_G4 = 1UL << 30, BB_H4 = 1UL << 31, BB_A5 = 1UL << 32, BB_B5 = 1UL << 33, BB_C5 = 1UL << 34, BB_D5 = 1UL << 35, BB_E5 = 1UL << 36, BB_F5 = 1UL << 37, BB_G5 = 1UL << 38, BB_H5 = 1UL << 39, BB_A6 = 1UL << 40, BB_B6 = 1UL << 41, BB_C6 = 1UL << 42, BB_D6 = 1UL << 43, BB_E6 = 1UL << 44, BB_F6 = 1UL << 45, BB_G6 = 1UL << 46, BB_H6 = 1UL << 47, BB_A7 = 1UL << 48, BB_B7 = 1UL << 49, BB_C7 = 1UL << 50, BB_D7 = 1UL << 51, BB_E7 = 1UL << 52, BB_F7 = 1UL << 53, BB_G7 = 1UL << 54, BB_H7 = 1UL << 55, BB_A8 = 1UL << 56, BB_B8 = 1UL << 57, BB_C8 = 1UL << 58, BB_D8 = 1UL << 59, BB_E8 = 1UL << 60, BB_F8 = 1UL << 61, BB_G8 = 1UL << 62, BB_H8 = 1UL << 63;

const Bitboard BB_CORNERS = BB_A1 | BB_H1 | BB_A8 | BB_H8;
const Bitboard BB_CENTER = BB_D4 | BB_E4 | BB_D5 | BB_E5;

const Bitboard BB_LIGHT_SQUARES = 0x55aa55aa55aa55aa;
const Bitboard BB_DARK_SQUARES = 0xaa55aa55aa55aa55;

const Bitboard BB_FILES[] = {0x0101010101010101UL << 0, 0x0101010101010101UL << 1, 0x0101010101010101UL << 2, 0x0101010101010101UL << 3, 0x0101010101010101UL << 4, 0x0101010101010101UL << 5, 0x0101010101010101UL << 6, 0x0101010101010101UL << 7}, BB_FILE_A = 0x0101010101010101UL << 0, BB_FILE_B = 0x0101010101010101UL << 1, BB_FILE_C = 0x0101010101010101UL << 2, BB_FILE_D = 0x0101010101010101UL << 3, BB_FILE_E = 0x0101010101010101UL << 4, BB_FILE_F = 0x0101010101010101UL << 5, BB_FILE_G = 0x0101010101010101UL << 6, BB_FILE_H = 0x0101010101010101UL << 7;

const Bitboard BB_RANKS[] = {0xffUL << (8 * 0), 0xffUL << (8 * 1), 0xffUL << (8 * 2), 0xffUL << (8 * 3), 0xffUL << (8 * 4), 0xffUL << (8 * 5), 0xffUL << (8 * 6), 0xffUL << (8 * 7)}, BB_RANK_1 = 0xffUL << (8 * 0), BB_RANK_2 = 0xffUL << (8 * 1), BB_RANK_3 = 0xffUL << (8 * 2), BB_RANK_4 = 0xffUL << (8 * 3), BB_RANK_5 = 0xffUL << (8 * 4), BB_RANK_6 = 0xffUL << (8 * 5), BB_RANK_7 = 0xffUL << (8 * 6), BB_RANK_8 = 0xffUL << (8 * 7);

const Bitboard BB_BACKRANKS = BB_RANK_1 | BB_RANK_8;

int lsb(Bitboard bb)
{
    return (int)log2(bb & -bb);
}

vector<Square> scan_forward(Bitboard bb)
{
    vector<Square> iter;
    while (bb)
    {
        Bitboard r = bb & -bb;
        iter.push_back((int)log2(r));
        bb ^= r;
    }

    return iter;
}

int msb(Bitboard bb)
{
    return (int)log2(bb);
}

vector<Square> scan_reversed(Bitboard bb)
{
    vector<Square> iter;
    while (bb)
    {
        int r = (int)log2(bb);
        iter.push_back(r);
        bb ^= BB_SQUARES[r];
    }

    return iter;
}

Bitboard flip_vertical(Bitboard bb)
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

const Bitboard BB_KNIGHT_ATTACKS[] = {_step_attacks(0, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(1, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(2, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(3, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(4, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(5, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(6, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(7, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(8, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(9, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(10, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(11, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(12, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(13, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(14, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(15, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(16, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(17, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(18, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(19, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(20, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(21, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(22, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(23, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(24, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(25, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(26, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(27, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(28, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(29, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(30, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(31, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(32, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(33, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(34, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(35, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(36, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(37, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(38, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(39, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(40, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(41, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(42, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(43, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(44, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(45, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(46, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(47, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(48, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(49, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(50, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(51, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(52, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(53, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(54, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(55, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(56, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(57, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(58, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(59, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(60, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(61, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(62, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(63, {17, 15, 10, 6, -17, -15, -10, -6})};
const Bitboard BB_KING_ATTACKS[] = {_step_attacks(0, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(1, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(2, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(3, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(4, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(5, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(6, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(7, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(8, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(9, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(10, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(11, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(12, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(13, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(14, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(15, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(16, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(17, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(18, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(19, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(20, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(21, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(22, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(23, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(24, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(25, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(26, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(27, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(28, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(29, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(30, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(31, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(32, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(33, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(34, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(35, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(36, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(37, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(38, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(39, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(40, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(41, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(42, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(43, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(44, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(45, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(46, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(47, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(48, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(49, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(50, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(51, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(52, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(53, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(54, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(55, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(56, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(57, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(58, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(59, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(60, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(61, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(62, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(63, {9, 8, 7, 1, -9, -8, -7, -1})};
const Bitboard BB_PAWN_ATTACKS[][64] = {{_step_attacks(0, {-7, -9}), _step_attacks(1, {-7, -9}), _step_attacks(2, {-7, -9}), _step_attacks(3, {-7, -9}), _step_attacks(4, {-7, -9}), _step_attacks(5, {-7, -9}), _step_attacks(6, {-7, -9}), _step_attacks(7, {-7, -9}), _step_attacks(8, {-7, -9}), _step_attacks(9, {-7, -9}), _step_attacks(10, {-7, -9}), _step_attacks(11, {-7, -9}), _step_attacks(12, {-7, -9}), _step_attacks(13, {-7, -9}), _step_attacks(14, {-7, -9}), _step_attacks(15, {-7, -9}), _step_attacks(16, {-7, -9}), _step_attacks(17, {-7, -9}), _step_attacks(18, {-7, -9}), _step_attacks(19, {-7, -9}), _step_attacks(20, {-7, -9}), _step_attacks(21, {-7, -9}), _step_attacks(22, {-7, -9}), _step_attacks(23, {-7, -9}), _step_attacks(24, {-7, -9}), _step_attacks(25, {-7, -9}), _step_attacks(26, {-7, -9}), _step_attacks(27, {-7, -9}), _step_attacks(28, {-7, -9}), _step_attacks(29, {-7, -9}), _step_attacks(30, {-7, -9}), _step_attacks(31, {-7, -9}), _step_attacks(32, {-7, -9}), _step_attacks(33, {-7, -9}), _step_attacks(34, {-7, -9}), _step_attacks(35, {-7, -9}), _step_attacks(36, {-7, -9}), _step_attacks(37, {-7, -9}), _step_attacks(38, {-7, -9}), _step_attacks(39, {-7, -9}), _step_attacks(40, {-7, -9}), _step_attacks(41, {-7, -9}), _step_attacks(42, {-7, -9}), _step_attacks(43, {-7, -9}), _step_attacks(44, {-7, -9}), _step_attacks(45, {-7, -9}), _step_attacks(46, {-7, -9}), _step_attacks(47, {-7, -9}), _step_attacks(48, {-7, -9}), _step_attacks(49, {-7, -9}), _step_attacks(50, {-7, -9}), _step_attacks(51, {-7, -9}), _step_attacks(52, {-7, -9}), _step_attacks(53, {-7, -9}), _step_attacks(54, {-7, -9}), _step_attacks(55, {-7, -9}), _step_attacks(56, {-7, -9}), _step_attacks(57, {-7, -9}), _step_attacks(58, {-7, -9}), _step_attacks(59, {-7, -9}), _step_attacks(60, {-7, -9}), _step_attacks(61, {-7, -9}), _step_attacks(62, {-7, -9}), _step_attacks(63, {-7, -9})}, {_step_attacks(0, {7, 9}), _step_attacks(1, {7, 9}), _step_attacks(2, {7, 9}), _step_attacks(3, {7, 9}), _step_attacks(4, {7, 9}), _step_attacks(5, {7, 9}), _step_attacks(6, {7, 9}), _step_attacks(7, {7, 9}), _step_attacks(8, {7, 9}), _step_attacks(9, {7, 9}), _step_attacks(10, {7, 9}), _step_attacks(11, {7, 9}), _step_attacks(12, {7, 9}), _step_attacks(13, {7, 9}), _step_attacks(14, {7, 9}), _step_attacks(15, {7, 9}), _step_attacks(16, {7, 9}), _step_attacks(17, {7, 9}), _step_attacks(18, {7, 9}), _step_attacks(19, {7, 9}), _step_attacks(20, {7, 9}), _step_attacks(21, {7, 9}), _step_attacks(22, {7, 9}), _step_attacks(23, {7, 9}), _step_attacks(24, {7, 9}), _step_attacks(25, {7, 9}), _step_attacks(26, {7, 9}), _step_attacks(27, {7, 9}), _step_attacks(28, {7, 9}), _step_attacks(29, {7, 9}), _step_attacks(30, {7, 9}), _step_attacks(31, {7, 9}), _step_attacks(32, {7, 9}), _step_attacks(33, {7, 9}), _step_attacks(34, {7, 9}), _step_attacks(35, {7, 9}), _step_attacks(36, {7, 9}), _step_attacks(37, {7, 9}), _step_attacks(38, {7, 9}), _step_attacks(39, {7, 9}), _step_attacks(40, {7, 9}), _step_attacks(41, {7, 9}), _step_attacks(42, {7, 9}), _step_attacks(43, {7, 9}), _step_attacks(44, {7, 9}), _step_attacks(45, {7, 9}), _step_attacks(46, {7, 9}), _step_attacks(47, {7, 9}), _step_attacks(48, {7, 9}), _step_attacks(49, {7, 9}), _step_attacks(50, {7, 9}), _step_attacks(51, {7, 9}), _step_attacks(52, {7, 9}), _step_attacks(53, {7, 9}), _step_attacks(54, {7, 9}), _step_attacks(55, {7, 9}), _step_attacks(56, {7, 9}), _step_attacks(57, {7, 9}), _step_attacks(58, {7, 9}), _step_attacks(59, {7, 9}), _step_attacks(60, {7, 9}), _step_attacks(61, {7, 9}), _step_attacks(62, {7, 9}), _step_attacks(63, {7, 9})}};

Bitboard _edges(Square square)
{
    return (((BB_RANK_1 | BB_RANK_8) & ~BB_RANKS[square_rank(square)]) |
            ((BB_FILE_A | BB_FILE_H) & ~BB_FILES[square_file(square)]));
}

vector<Bitboard> _carry_rippler(Bitboard mask)
{
    // Carry-Rippler trick to iterate subsets of mask.
    vector<Bitboard> iter;
    Bitboard subset = BB_EMPTY;
    while (true)
    {
        iter.push_back(subset);
        subset = (subset - mask) & mask;
        if (!subset)
            break;
    }

    return iter;
}

tuple<vector<Bitboard>, vector<unordered_map<Bitboard, Bitboard>>> _attack_table(vector<int> deltas)
{
    vector<Bitboard> mask_table;
    vector<unordered_map<Bitboard, Bitboard>> attack_table;

    for (Square square : SQUARES)
    {
        unordered_map<Bitboard, Bitboard> attacks;

        Bitboard mask = _sliding_attacks(square, 0, deltas) & ~_edges(square);
        for (Bitboard subset : _carry_rippler(mask))
            attacks[subset] = _sliding_attacks(square, subset, deltas);

        attack_table.push_back(attacks);
        mask_table.push_back(mask);
    }

    return {mask_table, attack_table};
}

const auto [BB_DIAG_MASKS, BB_DIAG_ATTACKS] = _attack_table({-9, -7, 7, 9});
const auto [BB_FILE_MASKS, BB_FILE_ATTACKS] = _attack_table({-8, 8});
const auto [BB_RANK_MASKS, BB_RANK_ATTACKS] = _attack_table({-1, 1});

int main()
{
}