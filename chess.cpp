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
#include <bit>
#include <tuple>
#include <regex>
#include <stack>

#include <iostream>

using namespace std;

typedef string _EnPassantSpec;

typedef bool Color;
const Color COLORS[] = {true, false}, WHITE = true, BLACK = false;
const string COLOR_NAMES[] = {"black", "white"};

typedef uint8_t PieceType;
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
    {'R', "♜"},
    {'r', "♖"},
    {'N', "♞"},
    {'n', "♘"},
    {'B', "♝"},
    {'b', "♗"},
    {'Q', "♛"},
    {'q', "♕"},
    {'K', "♚"},
    {'k', "♔"},
    {'P', "♟"},
    {'p', "♙"},
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
    // See :func:`chess::Board::is_checkmate()`.
    CSTALEMATE = 2,
    // See :func:`chess::Board::is_stalemate()`.
    CINSUFFICIENT_MATERIAL = 3,
    // See :func:`chess::Board::is_insufficient_material()`.
    CSEVENTYFIVE_MOVES = 4,
    // See :func:`chess::Board::is_seventyfive_moves()`.
    CFIVEFOLD_REPETITION = 5,
    // See :func:`chess::Board::is_fivefold_repetition()`.
    CFIFTY_MOVES = 6,
    // See :func:`chess::Board::can_claim_fifty_moves()`.
    CTHREEFOLD_REPETITION = 7,
    // See :func:`chess::Board::can_claim_threefold_repetition()`.
    CVARIANT_WIN = 8,
    // See :func:`chess::Board::is_variant_win()`.
    CVARIANT_LOSS = 9,
    // See :func:`chess::Board::is_variant_loss()`.
    CVARIANT_DRAW = 10
    // See :func:`chess::Board::is_variant_draw()`.
};

class Outcome
{
    /*
    Information about the outcome of an ended game, usually obtained from
    :func:`chess::Board::outcome()`.
    */

public:
    Termination termination;
    // The reason for the game to have ended.

    int8_t winner;
    // The winning color or ``-1`` if drawn.

    Outcome(Termination termination, int8_t winner)
    {
        this->termination = termination;
        this->winner = winner;
    }

    string result() const
    {
        // Returns ``1-0``, ``0-1`` or ``1/2-1/2``.
        return this->winner == -1 ? "1/2-1/2" : (this->winner ? "1-0" : "0-1");
    }
};

typedef uint8_t Square;
const Square SQUARES[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63}, A1 = 0, B1 = 1, C1 = 2, D1 = 3, E1 = 4, F1 = 5, G1 = 6, H1 = 7, A2 = 8, B2 = 9, C2 = 10, D2 = 11, E2 = 12, F2 = 13, G2 = 14, H2 = 15, A3 = 16, B3 = 17, C3 = 18, D3 = 19, E3 = 20, F3 = 21, G3 = 22, H3 = 23, A4 = 24, B4 = 25, C4 = 26, D4 = 27, E4 = 28, F4 = 29, G4 = 30, H4 = 31, A5 = 32, B5 = 33, C5 = 34, D5 = 35, E5 = 36, F5 = 37, G5 = 38, H5 = 39, A6 = 40, B6 = 41, C6 = 42, D6 = 43, E6 = 44, F6 = 45, G6 = 46, H6 = 47, A7 = 48, B7 = 49, C7 = 50, D7 = 51, E7 = 52, F7 = 53, G7 = 54, H7 = 55, A8 = 56, B8 = 57, C8 = 58, D8 = 59, E8 = 60, F8 = 61, G8 = 62, H8 = 63;

const string SQUARE_NAMES[] = {"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2", "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3", "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4", "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5", "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6", "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7", "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"};

Square parse_square(const string &name)
{
    /*
    Gets the square index for the given square *name*
    (e.g., ``a1`` returns ``0``).

    :raises: :exc:`invalid_argument` if the square name is invalid.
    */
    const auto &it = find(SQUARE_NAMES, SQUARE_NAMES + sizeof(SQUARE_NAMES) / sizeof(SQUARE_NAMES[0]), name);
    if (it == end(SQUARE_NAMES))
        throw invalid_argument("square name is invalid");
    return distance(SQUARE_NAMES, it);
}

string square_name(Square square)
{
    // Gets the name of the square, like ``a3``.
    return SQUARE_NAMES[square];
}

Square square(uint8_t file_index, uint8_t rank_index)
{
    // Gets a square number by file and rank index.
    return rank_index * 8 + file_index;
}

uint8_t square_file(Square square)
{
    // Gets the file index of the square where ``0`` is the a-file.
    return square & 7;
}

uint8_t square_rank(Square square)
{
    // Gets the rank index of the square where ``0`` is the first rank.
    return square >> 3;
}

uint8_t square_distance(Square a, Square b)
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
const Bitboard BB_ALL = 0xffffffffffffffffUL;

const Bitboard BB_SQUARES[] = {1UL << 0, 1UL << 1, 1UL << 2, 1UL << 3, 1UL << 4, 1UL << 5, 1UL << 6, 1UL << 7, 1UL << 8, 1UL << 9, 1UL << 10, 1UL << 11, 1UL << 12, 1UL << 13, 1UL << 14, 1UL << 15, 1UL << 16, 1UL << 17, 1UL << 18, 1UL << 19, 1UL << 20, 1UL << 21, 1UL << 22, 1UL << 23, 1UL << 24, 1UL << 25, 1UL << 26, 1UL << 27, 1UL << 28, 1UL << 29, 1UL << 30, 1UL << 31, 1UL << 32, 1UL << 33, 1UL << 34, 1UL << 35, 1UL << 36, 1UL << 37, 1UL << 38, 1UL << 39, 1UL << 40, 1UL << 41, 1UL << 42, 1UL << 43, 1UL << 44, 1UL << 45, 1UL << 46, 1UL << 47, 1UL << 48, 1UL << 49, 1UL << 50, 1UL << 51, 1UL << 52, 1UL << 53, 1UL << 54, 1UL << 55, 1UL << 56, 1UL << 57, 1UL << 58, 1UL << 59, 1UL << 60, 1UL << 61, 1UL << 62, 1UL << 63}, BB_A1 = 1UL << 0, BB_B1 = 1UL << 1, BB_C1 = 1UL << 2, BB_D1 = 1UL << 3, BB_E1 = 1UL << 4, BB_F1 = 1UL << 5, BB_G1 = 1UL << 6, BB_H1 = 1UL << 7, BB_A2 = 1UL << 8, BB_B2 = 1UL << 9, BB_C2 = 1UL << 10, BB_D2 = 1UL << 11, BB_E2 = 1UL << 12, BB_F2 = 1UL << 13, BB_G2 = 1UL << 14, BB_H2 = 1UL << 15, BB_A3 = 1UL << 16, BB_B3 = 1UL << 17, BB_C3 = 1UL << 18, BB_D3 = 1UL << 19, BB_E3 = 1UL << 20, BB_F3 = 1UL << 21, BB_G3 = 1UL << 22, BB_H3 = 1UL << 23, BB_A4 = 1UL << 24, BB_B4 = 1UL << 25, BB_C4 = 1UL << 26, BB_D4 = 1UL << 27, BB_E4 = 1UL << 28, BB_F4 = 1UL << 29, BB_G4 = 1UL << 30, BB_H4 = 1UL << 31, BB_A5 = 1UL << 32, BB_B5 = 1UL << 33, BB_C5 = 1UL << 34, BB_D5 = 1UL << 35, BB_E5 = 1UL << 36, BB_F5 = 1UL << 37, BB_G5 = 1UL << 38, BB_H5 = 1UL << 39, BB_A6 = 1UL << 40, BB_B6 = 1UL << 41, BB_C6 = 1UL << 42, BB_D6 = 1UL << 43, BB_E6 = 1UL << 44, BB_F6 = 1UL << 45, BB_G6 = 1UL << 46, BB_H6 = 1UL << 47, BB_A7 = 1UL << 48, BB_B7 = 1UL << 49, BB_C7 = 1UL << 50, BB_D7 = 1UL << 51, BB_E7 = 1UL << 52, BB_F7 = 1UL << 53, BB_G7 = 1UL << 54, BB_H7 = 1UL << 55, BB_A8 = 1UL << 56, BB_B8 = 1UL << 57, BB_C8 = 1UL << 58, BB_D8 = 1UL << 59, BB_E8 = 1UL << 60, BB_F8 = 1UL << 61, BB_G8 = 1UL << 62, BB_H8 = 1UL << 63;

const Bitboard BB_CORNERS = BB_A1 | BB_H1 | BB_A8 | BB_H8;
const Bitboard BB_CENTER = BB_D4 | BB_E4 | BB_D5 | BB_E5;

const Bitboard BB_LIGHT_SQUARES = 0x55aa55aa55aa55aaUL;
const Bitboard BB_DARK_SQUARES = 0xaa55aa55aa55aa55UL;

const Bitboard BB_FILES[] = {0x0101010101010101UL << 0, 0x0101010101010101UL << 1, 0x0101010101010101UL << 2, 0x0101010101010101UL << 3, 0x0101010101010101UL << 4, 0x0101010101010101UL << 5, 0x0101010101010101UL << 6, 0x0101010101010101UL << 7}, BB_FILE_A = 0x0101010101010101UL << 0, BB_FILE_B = 0x0101010101010101UL << 1, BB_FILE_C = 0x0101010101010101UL << 2, BB_FILE_D = 0x0101010101010101UL << 3, BB_FILE_E = 0x0101010101010101UL << 4, BB_FILE_F = 0x0101010101010101UL << 5, BB_FILE_G = 0x0101010101010101UL << 6, BB_FILE_H = 0x0101010101010101UL << 7;

const Bitboard BB_RANKS[] = {0xffUL << (8 * 0), 0xffUL << (8 * 1), 0xffUL << (8 * 2), 0xffUL << (8 * 3), 0xffUL << (8 * 4), 0xffUL << (8 * 5), 0xffUL << (8 * 6), 0xffUL << (8 * 7)}, BB_RANK_1 = 0xffUL << (8 * 0), BB_RANK_2 = 0xffUL << (8 * 1), BB_RANK_3 = 0xffUL << (8 * 2), BB_RANK_4 = 0xffUL << (8 * 3), BB_RANK_5 = 0xffUL << (8 * 4), BB_RANK_6 = 0xffUL << (8 * 5), BB_RANK_7 = 0xffUL << (8 * 6), BB_RANK_8 = 0xffUL << (8 * 7);

const Bitboard BB_BACKRANKS = BB_RANK_1 | BB_RANK_8;

uint8_t lsb(Bitboard bb)
{
    return (uint8_t)log2(bb & -bb);
}

vector<Square> scan_forward(Bitboard bb)
{
    vector<Square> iter;
    while (bb)
    {
        Bitboard r = bb & -bb;
        iter.push_back((Square)log2(r));
        bb ^= r;
    }

    return iter;
}

uint8_t msb(Bitboard bb)
{
    return (uint8_t)log2(bb);
}

vector<Square> scan_reversed(Bitboard bb)
{
    vector<Square> iter;
    while (bb)
    {
        Square r = (Square)log2(bb);
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

Bitboard _sliding_attacks(Square square, Bitboard occupied, const vector<int8_t> &deltas)
{
    Bitboard attacks = BB_EMPTY;

    for (int8_t delta : deltas)
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

Bitboard _step_attacks(Square square, const vector<int8_t> &deltas)
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

tuple<vector<Bitboard>, vector<unordered_map<Bitboard, Bitboard>>> _attack_table(const vector<int8_t> &deltas)
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

const auto &[BB_DIAG_MASKS, BB_DIAG_ATTACKS] = _attack_table({-9, -7, 7, 9});
const auto &[BB_FILE_MASKS, BB_FILE_ATTACKS] = _attack_table({-8, 8});
const auto &[BB_RANK_MASKS, BB_RANK_ATTACKS] = _attack_table({-1, 1});

vector<vector<Bitboard>> _rays()
{
    vector<vector<Bitboard>> rays;
    for (uint8_t a = 0; a < 64; ++a)
    {
        Bitboard bb_a = BB_SQUARES[a];
        vector<Bitboard> rays_row;
        for (uint8_t b = 0; b < 64; ++b)
        {
            Bitboard bb_b = BB_SQUARES[b];
            if (BB_DIAG_ATTACKS[a].at(0) & bb_b)
                rays_row.push_back((BB_DIAG_ATTACKS[a].at(0) & BB_DIAG_ATTACKS[b].at(0)) | bb_a | bb_b);
            else if (BB_RANK_ATTACKS[a].at(0) & bb_b)
                rays_row.push_back(BB_RANK_ATTACKS[a].at(0) | bb_a);
            else if (BB_FILE_ATTACKS[a].at(0) & bb_b)
                rays_row.push_back(BB_FILE_ATTACKS[a].at(0) | bb_a);
            else
                rays_row.push_back(BB_EMPTY);
        }
        rays.push_back(rays_row);
    }
    return rays;
}

const vector<vector<Bitboard>> BB_RAYS = _rays();

Bitboard ray(Square a, Square b)
{
    return BB_RAYS[a][b];
}

Bitboard between(Square a, Square b)
{
    Bitboard bb = BB_RAYS[a][b] & ((BB_ALL << a) ^ (BB_ALL << b));
    return bb & (bb - 1);
}

regex SAN_REGEX(R"(^([NBKRQ])?([a-h])?([1-8])?[\-x]?([a-h][1-8])(=?[nbrqkNBRQK])?[\+#]?$)");

regex FEN_CASTLING_REGEX(R"(^(?:-|[KQABCDEFGH]{0,2}[kqabcdefgh]{0,2})$)");

class Piece
{
    // A piece with type and color.

public:
    PieceType piece_type;
    // The piece type.

    Color color;
    // The piece color.

    Piece(PieceType piece_type, Color color)
    {
        this->piece_type = piece_type;
        this->color = color;
    }

    char symbol() const
    {
        /*
        Gets the symbol ``P``, ``N``, ``B``, ``R``, ``Q`` or ``K`` for white
        pieces or the lower-case variants for the black pieces.
        */
        char symbol = piece_symbol(this->piece_type);
        return this->color ? toupper(symbol) : symbol;
    }

    string unicode_symbol(bool invert_color = false, ...) const
    {
        /*
        Gets the Unicode character for the piece.
        */
        char symbol = this->symbol();
        if (invert_color)
            symbol = isupper(symbol) ? tolower(symbol) : toupper(symbol);
        return UNICODE_PIECE_SYMBOLS.at(symbol);
    }

    operator string() const
    {
        return to_string(this->symbol());
    }

    static Piece *from_symbol(char symbol)
    {
        /*
        Creates a :class:`~chess::Piece` instance from a piece symbol.

        :raises: :exc:`invalid_argument` if the symbol is invalid.
        */
        const auto &it = find(PIECE_SYMBOLS, PIECE_SYMBOLS + sizeof(PIECE_SYMBOLS) / sizeof(PIECE_SYMBOLS[0]), tolower(symbol));
        if (it == end(PIECE_SYMBOLS))
            throw invalid_argument("symbol is invalid");
        return new Piece(distance(PIECE_SYMBOLS, it), toupper(symbol));
    }
};

template <>
struct hash<Piece>
{
    size_t operator()(const Piece &piece) const
    {
        return piece.piece_type + (piece.color ? -1 : 5);
    }
};

class Move
{
    /*
    Represents a move from a square to a square and possibly the promotion
    piece type.

    Drops and null moves are supported.
    */

public:
    Square from_square;
    // The source square.

    Square to_square;
    // The target square.

    PieceType promotion;
    // The promotion piece type or ``0``.

    PieceType drop;
    // The drop piece type or ``0``.

    Move(Square from_square, Square to_square, PieceType promotion = 0, PieceType drop = 0)
    {
        this->from_square = from_square;
        this->to_square = to_square;
        this->promotion = promotion;
        this->drop = drop;
    }

    string uci() const
    {
        /*
        Gets a UCI string for the move.

        For example, a move from a7 to a8 would be ``a7a8`` or ``a7a8q``
        (if the latter is a promotion to a queen).

        The UCI representation of a null move is ``0000``.
        */
        if (this->drop)
            return to_string(toupper(piece_symbol(this->drop))) + "@" + SQUARE_NAMES[this->to_square];
        else if (this->promotion)
            return SQUARE_NAMES[this->from_square] + SQUARE_NAMES[this->to_square] + piece_symbol(this->promotion);
        else if (*this)
            return SQUARE_NAMES[this->from_square] + SQUARE_NAMES[this->to_square];
        else
            return "0000";
    }

    string xboard() const
    {
        return *this ? this->uci() : "@@@@";
    }

    operator bool() const
    {
        return this->from_square || this->to_square || this->promotion || this->drop;
    }

    operator string() const
    {
        return this->uci();
    }

    static Move *from_uci(const string &uci)
    {
        /*
        Parses a UCI string.

        :raises: :exc:`invalid_argument` if the UCI string is invalid.
        */
        if (uci == "0000")
            return Move::null();
        else if (uci.length() == 4 && '@' == uci[1])
        {
            const auto &it = find(PIECE_SYMBOLS, PIECE_SYMBOLS + sizeof(PIECE_SYMBOLS) / sizeof(PIECE_SYMBOLS[0]), tolower(uci[0]));
            if (it == end(PIECE_SYMBOLS))
                throw invalid_argument("uci string is invalid");
            Square drop = distance(PIECE_SYMBOLS, it);
            const auto &it2 = find(SQUARE_NAMES, SQUARE_NAMES + sizeof(SQUARE_NAMES) / sizeof(SQUARE_NAMES[0]), uci.substr(2));
            if (it2 == end(SQUARE_NAMES))
                throw invalid_argument("uci string is invalid");
            Square square = distance(SQUARE_NAMES, it2);
            return new Move(square, square, drop = drop);
        }
        else if (4 <= uci.length() && uci.length() <= 5)
        {
            const auto &it = find(SQUARE_NAMES, SQUARE_NAMES + sizeof(SQUARE_NAMES) / sizeof(SQUARE_NAMES[0]), uci.substr(0, 2));
            if (it == end(SQUARE_NAMES))
                throw invalid_argument("uci string is invalid");
            Square from_square = distance(SQUARE_NAMES, it);
            const auto &it2 = find(SQUARE_NAMES, SQUARE_NAMES + sizeof(SQUARE_NAMES) / sizeof(SQUARE_NAMES[0]), uci.substr(2, 4));
            if (it2 == end(SQUARE_NAMES))
                throw invalid_argument("uci string is invalid");
            Square to_square = distance(SQUARE_NAMES, it2);
            Square promotion;
            if (uci.length() == 5)
            {
                const auto &it3 = find(PIECE_SYMBOLS, PIECE_SYMBOLS + sizeof(PIECE_SYMBOLS) / sizeof(PIECE_SYMBOLS[0]), uci[4]);
                if (it3 == end(PIECE_SYMBOLS))
                    throw invalid_argument("uci string is invalid");
                promotion = distance(PIECE_SYMBOLS, it3);
            }
            else
                promotion = 0;
            if (from_square == to_square)
                throw invalid_argument("invalid uci (use 0000 for null moves): " + uci);
            return new Move(from_square, to_square, promotion = promotion);
        }
        else
            throw invalid_argument("expected uci string to be of length 4 or 5: " + uci);
    }

    static Move *null()
    {
        /*
        Gets a null move.

        A null move just passes the turn to the other side (and possibly
        forfeits en passant capturing). Null moves evaluate to ``false`` in
        boolean contexts.
        */
        return new Move(0, 0);
    }
};

class BaseBoard;

typedef BaseBoard BaseBoardT;

class SquareSet
{
    /*
    A set of squares.

    Square sets are internally represented by 64-bit integer masks of the
    included squares. Bitwise operations can be used to compute unions,
    intersections and shifts.

    Also supports common set operations like
    :func:`~chess::SquareSet::issubset()`, :func:`~chess::SquareSet::issuperset()`,
    :func:`~chess::SquareSet::union()`, :func:`~chess::SquareSet::intersection()`,
    :func:`~chess::SquareSet::difference()`,
    :func:`~chess::SquareSet::symmetric_difference()` and
    :func:`~chess::SquareSet::copy()` as well as
    :func:`~chess::SquareSet::update()`,
    :func:`~chess::SquareSet::intersection_update()`,
    :func:`~chess::SquareSet::difference_update()`,
    :func:`~chess::SquareSet::symmetric_difference_update()` and
    :func:`~chess::SquareSet::clear()`.
    */

public:
    SquareSet(auto squares = BB_EMPTY)
    {
    }
};

class BaseBoard
{
    /*
    A board representing the position of chess pieces. See
    :class:`~chess::Board` for a full board with move generation.

    The board is initialized with the standard chess starting position, unless
    otherwise specified in the optional *board_fen* argument. If *board_fen*
    is ``""``, an empty board is created.
    */

public:
    Bitboard occupied_co[2] = {BB_EMPTY, BB_EMPTY}, pawns, knights, bishops, rooks, queens, kings, promoted, occupied;

    BaseBoard(const string &board_fen = STARTING_BOARD_FEN)
    {
        if (board_fen.empty())
            this->_clear_board();
        else if (board_fen == STARTING_BOARD_FEN)
            this->_reset_board();
        else
            this->_set_board_fen(board_fen);
    }

    void reset_board()
    {
        // Resets pieces to the starting position.
        this->_reset_board();
    }

    void clear_board()
    {
        // Clears the board.
        this->_clear_board();
    }

    Bitboard pieces_mask(PieceType piece_type, Color color) const
    {
        Bitboard bb;
        if (piece_type == PAWN)
            bb = this->pawns;
        else if (piece_type == KNIGHT)
            bb = this->knights;
        else if (piece_type == BISHOP)
            bb = this->bishops;
        else if (piece_type == ROOK)
            bb = this->rooks;
        else if (piece_type == QUEEN)
            bb = this->queens;
        else if (piece_type == KING)
            bb = this->kings;
        else
            throw "expected PieceType, got " + to_string(piece_type);

        return bb & this->occupied_co[color];
    }

    SquareSet *pieces(PieceType piece_type, Color color) const
    {
        /*
        Gets pieces of the given type and color.

        Returns a pointer to a :class:`set of squares <chess::SquareSet>`.
        */
        return new SquareSet(this->pieces_mask(piece_type, color));
    }

    Piece *piece_at(Square square) const
    {
        // Gets the :class:`piece <chess::Piece>` at the given square.
        PieceType piece_type = this->piece_type_at(square);
        if (piece_type)
        {
            Bitboard mask = BB_SQUARES[square];
            Color color = bool(this->occupied_co[WHITE] & mask);
            return new Piece(piece_type, color);
        }
        else
            return nullptr;
    }

    PieceType piece_type_at(Square square) const
    {
        // Gets the piece type at the given square.
        Bitboard mask = BB_SQUARES[square];

        if (!(this->occupied & mask))
            return 0; // Early return
        else if (this->pawns & mask)
            return PAWN;
        else if (this->knights & mask)
            return KNIGHT;
        else if (this->bishops & mask)
            return BISHOP;
        else if (this->rooks & mask)
            return ROOK;
        else if (this->queens & mask)
            return QUEEN;
        else
            return KING;
    }

    int8_t color_at(Square square) const
    {
        // Gets the color of the piece at the given square.
        Bitboard mask = BB_SQUARES[square];
        if (this->occupied_co[WHITE] & mask)
            return WHITE;
        else if (this->occupied_co[BLACK] & mask)
            return BLACK;
        else
            return -1;
    }

    int8_t king(Color color) const
    {
        /*
        Finds the king square of the given side. Returns ``-1`` if there
        is no king of that color.

        In variants with king promotions, only non-promoted kings are
        considered.
        */
        Bitboard king_mask = this->occupied_co[color] & this->kings & ~this->promoted;
        return king_mask ? int8_t(msb(king_mask)) : -1;
    }

    Bitboard attacks_mask(Square square) const
    {
        Bitboard bb_square = BB_SQUARES[square];

        if (bb_square & this->pawns)
        {
            Color color = bool(bb_square & this->occupied_co[WHITE]);
            return BB_PAWN_ATTACKS[color][square];
        }
        else if (bb_square & this->knights)
            return BB_KNIGHT_ATTACKS[square];
        else if (bb_square & this->kings)
            return BB_KING_ATTACKS[square];
        else
        {
            Bitboard attacks = 0;
            if (bb_square & this->bishops or bb_square & this->queens)
                attacks = BB_DIAG_ATTACKS[square].at(BB_DIAG_MASKS[square] & this->occupied);
            if (bb_square & this->rooks || bb_square & this->queens)
                attacks |= (BB_RANK_ATTACKS[square].at(BB_RANK_MASKS[square] & this->occupied) |
                            BB_FILE_ATTACKS[square].at(BB_FILE_MASKS[square] & this->occupied));
            return attacks;
        }
    }

    SquareSet *attacks(Square square) const
    {
        /*
        Gets the set of attacked squares from the given square.

        There will be no attacks if the square is empty. Pinned pieces are
        still attacking other squares.

        Returns a pointer to a :class:`set of squares <chess.SquareSet>`.
        */
        return new SquareSet(this->attacks_mask(square));
    }

    Bitboard attackers_mask(Color color, Square square) const
    {
        return this->_attackers_mask(color, square, this->occupied);
    }

    bool is_attacked_by(Color color, Square square) const
    {
        /*
        Checks if the given side attacks the given square.

        Pinned pieces still count as attackers. Pawns that can be captured
        en passant are **not** considered attacked.
        */
        return bool(this->attackers_mask(color, square));
    }

    SquareSet attackers(Color color, Square square) const
    {
        /*
        Gets the set of attackers of the given color for the given square.

        Pinned pieces still count as attackers.

        Returns a :class:`set of squares <chess::SquareSet>`.
        */
        return SquareSet(this->attackers_mask(color, square));
    }

    Bitboard pin_mask(Color color, Square square) const
    {
        int8_t king = this->king(color);
        if (king == -1)
            return BB_ALL;

        Bitboard square_mask = BB_SQUARES[square];

        for (const auto &[attacks, sliders] : {make_tuple(BB_FILE_ATTACKS, this->rooks | this->queens),
                                               make_tuple(BB_RANK_ATTACKS, this->rooks | this->queens),
                                               make_tuple(BB_DIAG_ATTACKS, this->bishops | this->queens)})
        {
            Bitboard rays = attacks[king].at(0);
            if (rays & square_mask)
            {
                Bitboard snipers = rays & sliders & this->occupied_co[!color];
                for (Square sniper : scan_reversed(snipers))
                {
                    if ((between(sniper, king) & (this->occupied | square_mask)) == square_mask)
                        return ray(king, sniper);
                }

                break;
            }
        }

        return BB_ALL;
    }

    SquareSet pin(Color color, Square square) const
    {
        /*
        Detects an absolute pin (and its direction) of the given square to
        the king of the given color.

        Returns a :class:`set of squares <chess::SquareSet>` that mask the rank,
        file or diagonal of the pin. If there is no pin, then a mask of the
        entire board is returned.
        */
        return SquareSet(this->pin_mask(color, square));
    }

    bool is_pinned(Color color, Square square) const
    {
        /*
        Detects if the given square is pinned to the king of the given color.
        */
        return this->pin_mask(color, square) != BB_ALL;
    }

    Piece *remove_piece_at(Square square)
    {
        /*
        Removes the piece from the given square. Returns a pointer to the
        :class:`~chess::Piece` or ``nullptr`` if the square was already empty.
        */
        Color color = bool(this->occupied_co[WHITE] & BB_SQUARES[square]);
        PieceType piece_type = this->_remove_piece_at(square);
        return piece_type ? new Piece(piece_type, color) : nullptr;
    }

    void set_piece_at(Square square, Piece *piece, bool promoted = false)
    {
        /*
        Sets a piece at the given square.

        An existing piece is replaced. Setting *piece* to ``nullptr`` is
        equivalent to :func:`~chess::Board::remove_piece_at()`.
        */
        if (piece == nullptr)
            this->_remove_piece_at(square);
        else
            this->_set_piece_at(square, piece->piece_type, piece->color, promoted);
    }

    string board_fen(bool promoted = false, ...) const
    {
        /*
        Gets the board FEN (e.g.,
        ``rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR``).
        */
        vector<char> builder;
        uint8_t empty = 0;

        for (Square square : SQUARES_180)
        {
            Piece *piece = this->piece_at(square);

            if (!piece)
                ++empty;
            else
            {
                if (empty)
                {
                    builder.push_back(empty + '0');
                    empty = 0;
                }
                builder.push_back(piece->symbol());
                if (promoted && BB_SQUARES[square] & this->promoted)
                    builder.push_back('~');
            }

            if (BB_SQUARES[square] & BB_FILE_H)
            {
                if (empty)
                {
                    builder.push_back(empty + '0');
                    empty = 0;
                }

                if (square != H1)
                    builder.push_back('/');
            }
        }

        return string(builder.begin(), builder.end());
    }

    void set_board_fen(const string &fen)
    {
        /*
        Parses *fen* and sets up the board, where *fen* is the board part of
        a FEN.

        :raises: :exc:`invalid_argument` if syntactically invalid.
        */
        this->_set_board_fen(fen);
    }

    unordered_map<Square, Piece *> piece_map(Bitboard mask = BB_ALL, ...) const
    {
        /*
        Gets a map of :class:`pieces <chess::Piece>` by square index.
        */
        unordered_map<Square, Piece *> result;
        for (Square square : scan_reversed(this->occupied & mask))
            result[square] = this->piece_at(square);
        return result;
    }

    void set_piece_map(const unordered_map<Square, Piece *> &pieces)
    {
        /*
        Sets up the board from a map of :class:`pieces <chess::Piece>`
        by square index.
        */
        this->_set_piece_map(pieces);
    }

    void set_chess960_pos(uint16_t scharnagl)
    {
        /*
        Sets up a Chess960 starting position given its index between 0 and 959.
        Also see :func:`~chess::BaseBoard::from_chess960_pos()`.
        */
        this->_set_chess960_pos(scharnagl);
    }

    int16_t chess960_pos() const
    {
        /*
        Gets the Chess960 starting position index between 0 and 959,
        or ``-1``.
        */
        if (this->occupied_co[WHITE] != (BB_RANK_1 | BB_RANK_2))
            return -1;
        if (this->occupied_co[BLACK] != (BB_RANK_7 | BB_RANK_8))
            return -1;
        if (this->pawns != (BB_RANK_2 | BB_RANK_7))
            return -1;
        if (this->promoted)
            return -1;

        // Piece counts.
        vector<Bitboard> brnqk = {this->bishops, this->rooks, this->knights, this->queens, this->kings};
        if (popcount(this->bishops) != 4 || popcount(this->rooks) != 4 || popcount(this->knights) != 4 || popcount(this->queens) != 2 || popcount(this->kings) != 2)
            return -1;

        // Symmetry.
        if (((BB_RANK_1 & this->bishops) << 56 != (BB_RANK_8 & this->bishops)) || ((BB_RANK_1 & this->rooks) << 56 != (BB_RANK_8 & this->rooks)) || ((BB_RANK_1 & this->knights) << 56 != (BB_RANK_8 & this->knights)) || ((BB_RANK_1 & this->queens) << 56 != (BB_RANK_8 & this->queens)) || ((BB_RANK_1 & this->kings) << 56 != (BB_RANK_8 & this->kings)))
            return -1;

        // Algorithm from ChessX
        Bitboard x = this->bishops & (2 + 8 + 32 + 128);
        if (!x)
            return -1;
        int8_t bs1 = (lsb(x) - 1) / 2;
        int8_t cc_pos = bs1;
        x = this->bishops & (1 + 4 + 16 + 64);
        if (!x)
            return -1;
        uint8_t bs2 = lsb(x) * 2;
        cc_pos += bs2;

        uint8_t q = 0;
        bool qf = false;
        uint8_t n0 = 0;
        uint8_t n1 = 0;
        bool n0f = false;
        bool n1f = false;
        uint8_t rf = 0;
        vector<uint8_t> n0s = {0, 4, 7, 9};
        for (Square square = A1; square <= H1; ++square)
        {
            Bitboard bb = BB_SQUARES[square];
            if (bb & this->queens)
                qf = true;
            else if (bb & this->rooks || bb & this->kings)
            {
                if (bb & this->kings)
                {
                    if (rf != 1)
                        return -1;
                }
                else
                    ++rf;

                if (!qf)
                    ++q;

                if (!n0f)
                    ++n0;
                else if (!n1f)
                    ++n1;
            }
            else if (bb & this->knights)
            {
                if (!qf)
                    ++q;

                if (!n0f)
                    n0f = true;
                else if (!n1f)
                    n1f = true;
            }
        }

        if (n0 < 4 && n1f && qf)
        {
            cc_pos += q * 16;
            uint8_t krn = n0s[n0] + n1;
            cc_pos += krn * 96;
            return cc_pos;
        }
        else
            return -1;
    }

    string unicode(bool invert_color = false, bool borders = false, string empty_square = "⭘", ...) const
    {
        /*
        Returns a string representation of the board with Unicode pieces.
        Useful for pretty-printing to a terminal.

        :param invert_color: Invert color of the Unicode pieces.
        :param borders: Show borders and a coordinate margin.
        */
        vector<char> builder;
        for (int rank_index = 7; rank_index >= 0; --rank_index)
        {
            if (borders)
                builder.insert(builder.end(), 2, ' ');
            builder.insert(builder.end(), 17, '-');
            builder.push_back('\n');

            builder.push_back(RANK_NAMES[rank_index]);
            builder.push_back(' ');

            for (int file_index = 0; file_index < 8; ++file_index)
            {
                Square square_index = square(file_index, rank_index);

                if (borders)
                    builder.push_back('|');
                else if (file_index > 0)
                    builder.push_back(' ');

                Piece *piece = this->piece_at(square_index);

                if (piece)
                {
                    string unicode_symbol = piece->unicode_symbol(invert_color = invert_color);
                    std::copy(unicode_symbol.begin(), unicode_symbol.end(), back_inserter(builder));
                }
                else
                    std::copy(empty_square.begin(), empty_square.end(), back_inserter(builder));
            }

            if (borders)
                builder.push_back('|');

            if (borders || rank_index > 0)
                builder.push_back('\n');
        }

        if (borders)
        {
            builder.insert(builder.end(), 2, ' ');
            builder.insert(builder.end(), 17, '-');
            builder.push_back('\n');
            builder.insert(builder.end(), 3, ' ');
            string letters = "   a b c d e f g h";
            std::copy(letters.begin(), letters.end(), back_inserter(builder));
        }

        return string(builder.begin(), builder.end());
    }

    bool operator==(const BaseBoard &board) const
    {
        return (
            this->occupied == board.occupied &&
            this->occupied_co[WHITE] == board.occupied_co[WHITE] &&
            this->pawns == board.pawns &&
            this->knights == board.knights &&
            this->bishops == board.bishops &&
            this->rooks == board.rooks &&
            this->queens == board.queens &&
            this->kings == board.kings);
    }

    void apply_transform(function<Bitboard(Bitboard)> f)
    {
        this->pawns = f(this->pawns);
        this->knights = f(this->knights);
        this->bishops = f(this->bishops);
        this->rooks = f(this->rooks);
        this->queens = f(this->queens);
        this->kings = f(this->kings);

        this->occupied_co[WHITE] = f(this->occupied_co[WHITE]);
        this->occupied_co[BLACK] = f(this->occupied_co[BLACK]);
        this->occupied = f(this->occupied);
        this->promoted = f(this->promoted);
    }

    BaseBoardT *transform(function<Bitboard(Bitboard)> f) const
    {
        /*
        Returns a transformed copy of the board by applying a bitboard
        transformation function.

        Available transformations include :func:`chess::flip_vertical()`,
        :func:`chess::flip_horizontal()`, :func:`chess::flip_diagonal()`,
        :func:`chess::flip_anti_diagonal()`, :func:`chess::shift_down()`,
        :func:`chess::shift_up()`, :func:`chess::shift_left()`, and
        :func:`chess::shift_right()`.

        Alternatively, :func:`~chess::BaseBoard::apply_transform()` can be used
        to apply the transformation on the board.
        */
        BaseBoardT *board = this->copy();
        board->apply_transform(f);
        return board;
    }

    void apply_mirror()
    {
        this->apply_transform(flip_vertical);
        swap(this->occupied_co[WHITE], this->occupied_co[BLACK]);
    }

    BaseBoardT *mirror() const
    {
        /*
        Returns a mirrored copy of the board.

        The board is mirrored vertically and piece colors are swapped, so that
        the position is equivalent modulo color.

        Alternatively, :func:`~chess::BaseBoard::apply_mirror()` can be used
        to mirror the board.
        */
        BaseBoardT *board = this->copy();
        board->apply_mirror();
        return board;
    }

    BaseBoardT *copy() const
    {
        // Creates a copy of the board.
        BaseBoardT *board = new BaseBoard("");

        board->pawns = this->pawns;
        board->knights = this->knights;
        board->bishops = this->bishops;
        board->rooks = this->rooks;
        board->queens = this->queens;
        board->kings = this->kings;

        board->occupied_co[WHITE] = this->occupied_co[WHITE];
        board->occupied_co[BLACK] = this->occupied_co[BLACK];
        board->occupied = this->occupied;
        board->promoted = this->promoted;

        return board;
    }

    static BaseBoardT *empty()
    {
        /*
        Creates a new empty board. Also see
        :func:`~chess::BaseBoard::clear_board()`.
        */
        return new BaseBoardT("");
    }

    static BaseBoardT *from_chess960_pos(uint16_t scharnagl)
    {
        /*
        Creates a new board, initialized with a Chess960 starting position.
        */
        BaseBoardT *board = BaseBoard::empty();
        board->set_chess960_pos(scharnagl);
        return board;
    }

private:
    void _reset_board()
    {
        this->pawns = BB_RANK_2 | BB_RANK_7;
        this->knights = BB_B1 | BB_G1 | BB_B8 | BB_G8;
        this->bishops = BB_C1 | BB_F1 | BB_C8 | BB_F8;
        this->rooks = BB_CORNERS;
        this->queens = BB_D1 | BB_D8;
        this->kings = BB_E1 | BB_E8;

        this->promoted = BB_EMPTY;

        this->occupied_co[WHITE] = BB_RANK_1 | BB_RANK_2;
        this->occupied_co[BLACK] = BB_RANK_7 | BB_RANK_8;
        this->occupied = BB_RANK_1 | BB_RANK_2 | BB_RANK_7 | BB_RANK_8;
    }

    void _clear_board()
    {
        this->pawns = BB_EMPTY;
        this->knights = BB_EMPTY;
        this->bishops = BB_EMPTY;
        this->rooks = BB_EMPTY;
        this->queens = BB_EMPTY;
        this->kings = BB_EMPTY;

        this->promoted = BB_EMPTY;

        this->occupied_co[WHITE] = BB_EMPTY;
        this->occupied_co[BLACK] = BB_EMPTY;
        this->occupied = BB_EMPTY;
    }

    Bitboard _attackers_mask(Color color, Square square, Bitboard occupied) const
    {
        Bitboard rank_pieces = BB_RANK_MASKS[square] & occupied;
        Bitboard file_pieces = BB_FILE_MASKS[square] & occupied;
        Bitboard diag_pieces = BB_DIAG_MASKS[square] & occupied;

        Bitboard queens_and_rooks = this->queens | this->rooks;
        Bitboard queens_and_bishops = this->queens | this->bishops;

        Bitboard attackers = ((BB_KING_ATTACKS[square] & this->kings) |
                              (BB_KNIGHT_ATTACKS[square] & this->knights) |
                              (BB_RANK_ATTACKS[square].at(rank_pieces) & queens_and_rooks) |
                              (BB_FILE_ATTACKS[square].at(file_pieces) & queens_and_rooks) |
                              (BB_DIAG_ATTACKS[square].at(diag_pieces) & queens_and_bishops) |
                              (BB_PAWN_ATTACKS[!color][square] & this->pawns));

        return attackers & this->occupied_co[color];
    }

    PieceType _remove_piece_at(Square square)
    {
        PieceType piece_type = this->piece_type_at(square);
        Bitboard mask = BB_SQUARES[square];

        if (piece_type == PAWN)
            this->pawns ^= mask;
        else if (piece_type == KNIGHT)
            this->knights ^= mask;
        else if (piece_type == BISHOP)
            this->bishops ^= mask;
        else if (piece_type == ROOK)
            this->rooks ^= mask;
        else if (piece_type == QUEEN)
            this->queens ^= mask;
        else if (piece_type == KING)
            this->kings ^= mask;
        else
            return 0;

        this->occupied ^= mask;
        this->occupied_co[WHITE] &= ~mask;
        this->occupied_co[BLACK] &= ~mask;

        this->promoted &= ~mask;

        return piece_type;
    }

    void _set_piece_at(Square square, PieceType piece_type, Color color, bool promoted = false)
    {
        this->_remove_piece_at(square);

        Bitboard mask = BB_SQUARES[square];

        if (piece_type == PAWN)
            this->pawns |= mask;
        else if (piece_type == KNIGHT)
            this->knights |= mask;
        else if (piece_type == BISHOP)
            this->bishops |= mask;
        else if (piece_type == ROOK)
            this->rooks |= mask;
        else if (piece_type == QUEEN)
            this->queens |= mask;
        else if (piece_type == KING)
            this->kings |= mask;
        else
            return;

        this->occupied ^= mask;
        this->occupied_co[color] ^= mask;

        if (promoted)
            this->promoted ^= mask;
    }

    void _set_board_fen(string fen)
    {
        // Compatibility with set_fen().
        auto it = fen.begin();
        auto it2 = fen.rbegin();
        while (isspace(*it))
            ++it;
        while (isspace(*it2))
            ++it2;
        fen = string(it, it2.base());
        if (fen.find(' ') != string::npos)
            throw invalid_argument("expected position part of fen, got multiple parts: " + fen);

        // Ensure the FEN is valid.
        vector<string> rows;
        for (int i = 0, dist = 0; i < fen.length(); ++i, ++dist)
        {
            if (fen[i] == '/')
            {
                rows.push_back(fen.substr(i - dist, dist));
                dist = 0;
            }
        }
        if (rows.size() != 8)
            throw invalid_argument("expected 8 rows in position part of fen: " + fen);

        // Validate each row.
        for (const string &row : rows)
        {
            uint8_t field_sum = 0;
            bool previous_was_digit = false;
            bool previous_was_piece = false;

            for (char c : row)
            {
                if (c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8')
                {
                    if (previous_was_digit)
                        throw invalid_argument("two subsequent digits in position part of fen: " + fen);
                    field_sum += uint8_t(c - '0');
                    previous_was_digit = true;
                    previous_was_piece = false;
                }
                else if (c == '~')
                {
                    if (!previous_was_piece)
                        throw invalid_argument("'~' not after piece in position part of fen: " + fen);
                    previous_was_digit = false;
                    previous_was_piece = false;
                }
                else if (find(begin(PIECE_SYMBOLS), end(PIECE_SYMBOLS), tolower(c)) != end(PIECE_SYMBOLS))
                {
                    ++field_sum;
                    previous_was_digit = false;
                    previous_was_piece = true;
                }
                else
                    throw invalid_argument("invalid character in position part of fen: " + fen);
            }

            if (field_sum != 8)
                throw invalid_argument("expected 8 columns per row in position part of fen: " + fen);
        }

        // Clear the board.
        this->_clear_board();

        // Put pieces on the board.
        uint8_t square_index = 0;
        for (char c : fen)
        {
            if (c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8')
                square_index += uint8_t(c);
            else if (find(begin(PIECE_SYMBOLS), end(PIECE_SYMBOLS), tolower(c)) != end(PIECE_SYMBOLS))
            {
                Piece *piece = Piece::from_symbol(c);
                this->_set_piece_at(SQUARES_180[square_index], piece->piece_type, piece->color);
                ++square_index;
            }
            else if (c == '~')
                this->promoted |= BB_SQUARES[SQUARES_180[square_index - 1]];
        }
    }

    void _set_piece_map(const unordered_map<Square, Piece *> &pieces)
    {
        this->_clear_board();
        for (const auto &[square, piece] : pieces)
            this->_set_piece_at(square, piece->piece_type, piece->color);
    }

    void _set_chess960_pos(uint16_t scharnagl)
    {
        if (!(0 <= scharnagl && scharnagl <= 959))
            throw invalid_argument("chess960 position index not 0 <= " + to_string(scharnagl) + " <= 959");

        // See http://www.russellcottrell.com/Chess/Chess960.htm for
        // a description of the algorithm.
        uint8_t n = scharnagl / 4, bw = scharnagl % 4;
        uint8_t bb = n % 4;
        n /= 4;
        uint8_t q = n % 6;
        n /= 6;

        uint8_t n1, n2;
        for (n1 = 0; n1 < 4; ++n1)
        {
            n2 = n + (3 - n1) * (4 - n1) / 2 - 5;
            if (n1 < n2 && 1 <= n2 && n2 <= 4)
                break;
        }

        // Bishops.
        uint8_t bw_file = bw * 2 + 1;
        uint8_t bb_file = bb * 2;
        this->bishops = (BB_FILES[bw_file] | BB_FILES[bb_file]) & BB_BACKRANKS;

        // Queens.
        uint8_t q_file = q;
        q_file += uint8_t(min(bw_file, bb_file) <= q_file);
        q_file += uint8_t(max(bw_file, bb_file) <= q_file);
        this->queens = BB_FILES[q_file] & BB_BACKRANKS;

        vector<uint8_t> used = {bw_file, bb_file, q_file};

        // Knights.
        this->knights = BB_EMPTY;
        for (uint8_t i = 0; i < 8; ++i)
        {
            if (find(used.begin(), used.end(), i) == used.end())
                if (n1 == 0 || n2 == 0)
                {
                    this->knights |= BB_FILES[i] & BB_BACKRANKS;
                    used.push_back(i);
                }
            --n1;
            --n2;
        }

        // RKR.
        for (uint8_t i = 0; i < 8; ++i)
        {
            if (find(used.begin(), used.end(), i) == used.end())
            {
                this->rooks = BB_FILES[i] & BB_BACKRANKS;
                used.push_back(i);
                break;
            }
        }
        for (uint8_t i = 1; i < 8; ++i)
        {
            if (find(used.begin(), used.end(), i) == used.end())
            {
                this->kings = BB_FILES[i] & BB_BACKRANKS;
                used.push_back(i);
                break;
            }
        }
        for (uint8_t i = 2; i < 8; ++i)
        {
            if (find(used.begin(), used.end(), i) == used.end())
            {
                this->rooks |= BB_FILES[i] & BB_BACKRANKS;
                break;
            }
        }

        // Finalize.
        this->pawns = BB_RANK_2 | BB_RANK_7;
        this->occupied_co[WHITE] = BB_RANK_1 | BB_RANK_2;
        this->occupied_co[BLACK] = BB_RANK_7 | BB_RANK_8;
        this->occupied = BB_RANK_1 | BB_RANK_2 | BB_RANK_7 | BB_RANK_8;
        this->promoted = BB_EMPTY;
    }
};

class Board;

typedef Board BoardT;

class Board : public BaseBoard
{
    /*
    A :class:`~chess::BaseBoard`, additional information representing
    a chess position, and a :data:`move stack <chess::Board::move_stack>`.

    Provides :data:`move generation <chess::Board::legal_moves>`, validation,
    :func:`parsing <chess::Board::parse_san()>`, attack generation,
    :func:`game end detection <chess::Board::is_game_over()>`,
    and the capability to :func:`make <chess::Board::push()>` and
    :func:`unmake <chess::Board::pop()>` moves.

    The board is initialized to the standard chess starting position,
    unless otherwise specified in the optional *fen* argument.
    If *fen* is ``""``, an empty board is created.

    Optionally supports *chess960*. In Chess960, castling moves are encoded
    by a king move to the corresponding rook square.
    Use :func:`chess::Board::from_chess960_pos()` to create a board with one
    of the Chess960 starting positions.

    It's safe to set :data:`~Board::turn`, :data:`~Board::castling_rights`,
    :data:`~Board::ep_square`, :data:`~Board::halfmove_clock` and
    :data:`~Board::fullmove_number` directly.

    .. warning::
        It is possible to set up and work with invalid positions. In this
        case, :class:`~chess::Board` implements a kind of "pseudo-chess"
        (useful to gracefully handle errors or to implement chess variants).
        Use :func:`~chess::Board::is_valid()` to detect invalid positions.
    */

public:
    const static string aliases[6] = {"Standard", "Chess", "Classical", "Normal", "Illegal", "From Position"};
    string uci_variant = "chess";
    string xboard_variant = "normal";
    string starting_fen = STARTING_FEN;

    string tbw_suffix = ".rtbw";
    string tbz_suffix = ".rtbz";
    unsigned char tbw_magic[4] = {0x71, 0xe8, 0x23, 0x5d};
    unsigned char tbz_magic[4] = {0xd7, 0x66, 0x0c, 0xa5};
    string pawnless_tbw_suffix = "";
    string pawnless_tbz_suffix = "";
    unsigned char pawnless_tbw_magic;
    unsigned char pawnless_tbz_magic;
    bool connected_kings = false;
    bool one_king = true;
    bool captures_compulsory = false;

    Color turn;
    // The side to move (``chess::WHITE`` or ``chess::BLACK``).

    Bitboard castling_rights;
    /*
    Bitmask of the rooks with castling rights.

    Use :func:`~chess::Board::set_castling_fen()` to set multiple castling
    rights. Also see :func:`~chess::Board::has_castling_rights()`,
    :func:`~chess::Board::has_kingside_castling_rights()`,
    :func:`~chess::Board::has_queenside_castling_rights()`,
    :func:`~chess::Board::has_chess960_castling_rights()`,
    :func:`~chess::Board::clean_castling_rights()`.
    */

    int8_t ep_square;
    /*
    The potential en passant square on the third or sixth rank or ``-1``.

    Use :func:`~chess::Board::has_legal_en_passant()` to test if en passant
    capturing would actually be possible on the next move.
    */

    uint16_t fullmove_number;
    /*
    Counts move pairs. Starts at `1` and is incremented after every move
    of the black side.
    */

    uint8_t halfmove_clock;
    // The number of half-moves since the last capture or pawn move.

    Bitboard promoted;
    // A bitmask of pieces that have been promoted.

    bool chess960;
    /*
    Whether the board is in Chess960 mode. In Chess960 castling moves are
    represented as king moves to the corresponding rook square.
    */

    stack<Move> move_stack;
    /*
    The move stack. Use :func:`Board::push() <chess::Board::push()>`,
    :func:`Board::pop() <chess::Board::pop()>`,
    :func:`Board::peek() <chess::Board::peek()>` and
    :func:`Board::clear_stack() <chess::Board::clear_stack()>` for
    manipulation.
    */

    Board(string fen = STARTING_FEN, bool chess960 = false, ...)
    {
        BaseBoard.__init__(self, None);

        this->chess960 = chess960;

        this->ep_square = -1;

        if (fen.empty())
            this->clear();
        else if (fen == Board::starting_fen)
            this->reset();
        else
            this->set_fen(fen);
    }

private:
    stack<_BoardState> _stack;
};

class _BoardState
{

public:
    Bitboard pawns, knights, bishops, rooks, queens, kings, occupied_w, occupied_b, occupied, promoted;
    Color turn;
    Bitboard castling_rights;
    int8_t ep_square;
    uint8_t halfmove_clock;
    uint16_t fullmove_number;

    _BoardState(BoardT *board)
    {
        this->pawns = board->pawns;
        this->knights = board->knights;
        this->bishops = board->bishops;
        this->rooks = board->rooks;
        this->queens = board->queens;
        this->kings = board->kings;

        this->occupied_w = board->occupied_co[WHITE];
        this->occupied_b = board->occupied_co[BLACK];
        this->occupied = board->occupied;

        this->promoted = board->promoted;

        this->turn = board->turn;
        this->castling_rights = board->castling_rights;
        this->ep_square = board->ep_square;
        this->halfmove_clock = board->halfmove_clock;
        this->fullmove_number = board->fullmove_number;
    }

    void restore(BoardT *board) const
    {
        board->pawns = this->pawns;
        board->knights = this->knights;
        board->bishops = this->bishops;
        board->rooks = this->rooks;
        board->queens = this->queens;
        board->kings = this->kings;

        board->occupied_co[WHITE] = this->occupied_w;
        board->occupied_co[BLACK] = this->occupied_b;
        board->occupied = this->occupied;

        board->promoted = this->promoted;

        board->turn = this->turn;
        board->castling_rights = this->castling_rights;
        board->ep_square = this->ep_square;
        board->halfmove_clock = this->halfmove_clock;
        board->fullmove_number = this->fullmove_number;
    }
};

int main()
{
}