/*
This is a line-for-line remake of niklasf's 'python-chess' in C++
All credit for the original code and algorithms go to niklasf and his credits
The original source code can be found here: https://github.com/niklasf/python-chess
*/

/*
A chess library with move generation and validation,
and XBoard/UCI engine communication.
*/
#ifndef CHESS_H
#define CHESS_H
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <limits>
#include <algorithm>
#include <cmath>
#include <bitset>
#include <bit>
#include <vector>
#include <tuple>
#include <regex>
#include <functional>
#include <optional>
#include <sstream>
#include <stack>
#include <variant>
#include <array>
#include <iterator>

namespace chess
{

    std::string __author__ = "Patrick Johnson";

    std::string __email__ = "pjpuzzler@gmail.com";

    std::string __version__ = "1.0.0";

    typedef std::string _EnPassantSpec;

    typedef bool Color;
    const Color COLORS[] = {true, false}, WHITE = true, BLACK = false;
    const std::string COLOR_NAMES[] = {"black", "white"};

    typedef int PieceType;
    const PieceType PIECE_TYPES[] = {1, 2, 3, 4, 5, 6}, PAWN = 1, KNIGHT = 2, BISHOP = 3, ROOK = 4, QUEEN = 5, KING = 6;
    const std::optional<char> PIECE_SYMBOLS[] = {std::nullopt, 'p', 'n', 'b', 'r', 'q', 'k'};
    const std::optional<std::string> PIECE_NAMES[] = {std::nullopt, "pawn", "knight", "bishop", "rook", "queen", "king"};

    char piece_symbol(PieceType);

    std::string piece_name(PieceType);

    const std::unordered_map<char, std::string> UNICODE_PIECE_SYMBOLS = {
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

    const std::string STARTING_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    /* The FEN for the standard chess starting position. */

    const std::string STARTING_BOARD_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";
    /* The board part of the FEN for the standard chess starting position. */

    enum class Status
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

    enum class Termination
    {
        /* Enum with reasons for a game to be over. */

        CHECKMATE,
        /* See :func:`chess::Board::is_checkmate()`. */
        STALEMATE,
        /* See :func:`chess::Board::is_stalemate()`. */
        INSUFFICIENT_MATERIAL,
        /* See :func:`chess::Board::is_insufficient_material()`. */
        SEVENTYFIVE_MOVES,
        /* See :func:`chess::Board::is_seventyfive_moves()`. */
        FIVEFOLD_REPETITION,
        /* See :func:`chess::Board::is_fivefold_repetition()`. */
        FIFTY_MOVES,
        /* See :func:`chess::Board::can_claim_fifty_moves()`. */
        THREEFOLD_REPETITION,
        /* See :func:`chess::Board::can_claim_threefold_repetition()`. */
        VARIANT_WIN,
        /* See :func:`chess::Board::is_variant_win()`. */
        VARIANT_LOSS,
        /* See :func:`chess::Board::is_variant_loss()`. */
        VARIANT_DRAW
        /* See :func:`chess::Board::is_variant_draw()`. */
    };

    std::ostream &operator<<(std::ostream &, Termination);

    class Outcome
    {
        /*
        Information about the outcome of an ended game, usually obtained from
        :func:`chess::Board::outcome()`.
        */

    public:
        Termination termination;
        /* The reason for the game to have ended. */

        std::optional<Color> winner;
        /* The winning color or ``std::nullopt`` if drawn. */

        Outcome(Termination, std::optional<Color>);

        std::string result() const;
    };

    std::ostream &operator<<(std::ostream &, const Outcome &);

    typedef int Square;
    const Square SQUARES[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63}, A1 = 0, B1 = 1, C1 = 2, D1 = 3, E1 = 4, F1 = 5, G1 = 6, H1 = 7, A2 = 8, B2 = 9, C2 = 10, D2 = 11, E2 = 12, F2 = 13, G2 = 14, H2 = 15, A3 = 16, B3 = 17, C3 = 18, D3 = 19, E3 = 20, F3 = 21, G3 = 22, H3 = 23, A4 = 24, B4 = 25, C4 = 26, D4 = 27, E4 = 28, F4 = 29, G4 = 30, H4 = 31, A5 = 32, B5 = 33, C5 = 34, D5 = 35, E5 = 36, F5 = 37, G5 = 38, H5 = 39, A6 = 40, B6 = 41, C6 = 42, D6 = 43, E6 = 44, F6 = 45, G6 = 46, H6 = 47, A7 = 48, B7 = 49, C7 = 50, D7 = 51, E7 = 52, F7 = 53, G7 = 54, H7 = 55, A8 = 56, B8 = 57, C8 = 58, D8 = 59, E8 = 60, F8 = 61, G8 = 62, H8 = 63;

    const std::string SQUARE_NAMES[] = {"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2", "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3", "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4", "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5", "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6", "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7", "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"};

    Square parse_square(const std::string &);

    std::string square_name(Square);

    Square square(int, int);

    int square_file(Square);

    int square_rank(Square);

    int square_distance(Square, Square);

    Square square_mirror(Square);

    const Square SQUARES_180[] = {square_mirror(0), square_mirror(1), square_mirror(2), square_mirror(3), square_mirror(4), square_mirror(5), square_mirror(6), square_mirror(7), square_mirror(8), square_mirror(9), square_mirror(10), square_mirror(11), square_mirror(12), square_mirror(13), square_mirror(14), square_mirror(15), square_mirror(16), square_mirror(17), square_mirror(18), square_mirror(19), square_mirror(20), square_mirror(21), square_mirror(22), square_mirror(23), square_mirror(24), square_mirror(25), square_mirror(26), square_mirror(27), square_mirror(28), square_mirror(29), square_mirror(30), square_mirror(31), square_mirror(32), square_mirror(33), square_mirror(34), square_mirror(35), square_mirror(36), square_mirror(37), square_mirror(38), square_mirror(39), square_mirror(40), square_mirror(41), square_mirror(42), square_mirror(43), square_mirror(44), square_mirror(45), square_mirror(46), square_mirror(47), square_mirror(48), square_mirror(49), square_mirror(50), square_mirror(51), square_mirror(52), square_mirror(53), square_mirror(54), square_mirror(55), square_mirror(56), square_mirror(57), square_mirror(58), square_mirror(59), square_mirror(60), square_mirror(61), square_mirror(62), square_mirror(63)};

    typedef unsigned long Bitboard;
    const Bitboard BB_EMPTY = 0;
    const Bitboard BB_ALL = 0xffff'ffff'ffff'ffff;

    const Bitboard BB_SQUARES[] = {
        1UL << 0,
        1UL << 1,
        1UL << 2,
        1UL << 3,
        1UL << 4,
        1UL << 5,
        1UL << 6,
        1UL << 7,
        1UL << 8,
        1UL << 9,
        1UL << 10,
        1UL << 11,
        1UL << 12,
        1UL << 13,
        1UL << 14,
        1UL << 15,
        1UL << 16,
        1UL << 17,
        1UL << 18,
        1UL << 19,
        1UL << 20,
        1UL << 21,
        1UL << 22,
        1UL << 23,
        1UL << 24,
        1UL << 25,
        1UL << 26,
        1UL << 27,
        1UL << 28,
        1UL << 29,
        1UL << 30,
        1UL << 31,
        1UL << 32,
        1UL << 33,
        1UL << 34,
        1UL << 35,
        1UL << 36,
        1UL << 37,
        1UL << 38,
        1UL << 39,
        1UL << 40,
        1UL << 41,
        1UL << 42,
        1UL << 43,
        1UL << 44,
        1UL << 45,
        1UL << 46,
        1UL << 47,
        1UL << 48,
        1UL << 49,
        1UL << 50,
        1UL << 51,
        1UL << 52,
        1UL << 53,
        1UL << 54,
        1UL << 55,
        1UL << 56,
        1UL << 57,
        1UL << 58,
        1UL << 59,
        1UL << 60,
        1UL << 61,
        1UL << 62,
        1UL << 63,
    },
                   BB_A1 = 1UL << 0, BB_B1 = 1UL << 1, BB_C1 = 1UL << 2, BB_D1 = 1UL << 3, BB_E1 = 1UL << 4, BB_F1 = 1UL << 5, BB_G1 = 1UL << 6, BB_H1 = 1UL << 7, BB_A2 = 1UL << 8, BB_B2 = 1UL << 9, BB_C2 = 1UL << 10, BB_D2 = 1UL << 11, BB_E2 = 1UL << 12, BB_F2 = 1UL << 13, BB_G2 = 1UL << 14, BB_H2 = 1UL << 15, BB_A3 = 1UL << 16, BB_B3 = 1UL << 17, BB_C3 = 1UL << 18, BB_D3 = 1UL << 19, BB_E3 = 1UL << 20, BB_F3 = 1UL << 21, BB_G3 = 1UL << 22, BB_H3 = 1UL << 23, BB_A4 = 1UL << 24, BB_B4 = 1UL << 25, BB_C4 = 1UL << 26, BB_D4 = 1UL << 27, BB_E4 = 1UL << 28, BB_F4 = 1UL << 29, BB_G4 = 1UL << 30, BB_H4 = 1UL << 31, BB_A5 = 1UL << 32, BB_B5 = 1UL << 33, BB_C5 = 1UL << 34, BB_D5 = 1UL << 35, BB_E5 = 1UL << 36, BB_F5 = 1UL << 37, BB_G5 = 1UL << 38, BB_H5 = 1UL << 39, BB_A6 = 1UL << 40, BB_B6 = 1UL << 41, BB_C6 = 1UL << 42, BB_D6 = 1UL << 43, BB_E6 = 1UL << 44, BB_F6 = 1UL << 45, BB_G6 = 1UL << 46, BB_H6 = 1UL << 47, BB_A7 = 1UL << 48, BB_B7 = 1UL << 49, BB_C7 = 1UL << 50, BB_D7 = 1UL << 51, BB_E7 = 1UL << 52, BB_F7 = 1UL << 53, BB_G7 = 1UL << 54, BB_H7 = 1UL << 55, BB_A8 = 1UL << 56, BB_B8 = 1UL << 57, BB_C8 = 1UL << 58, BB_D8 = 1UL << 59, BB_E8 = 1UL << 60, BB_F8 = 1UL << 61, BB_G8 = 1UL << 62, BB_H8 = 1UL << 63;

    const Bitboard BB_CORNERS = BB_A1 | BB_H1 | BB_A8 | BB_H8;
    const Bitboard BB_CENTER = BB_D4 | BB_E4 | BB_D5 | BB_E5;

    const Bitboard BB_LIGHT_SQUARES = 0x55aa'55aa'55aa'55aa;
    const Bitboard BB_DARK_SQUARES = 0xaa55'aa55'aa55'aa55;

    const Bitboard BB_FILES[] = {
        0x0101'0101'0101'0101UL << 0,
        0x0101'0101'0101'0101UL << 1,
        0x0101'0101'0101'0101UL << 2,
        0x0101'0101'0101'0101UL << 3,
        0x0101'0101'0101'0101UL << 4,
        0x0101'0101'0101'0101UL << 5,
        0x0101'0101'0101'0101UL << 6,
        0x0101'0101'0101'0101UL << 7,
    },
                   BB_FILE_A = 0x0101'0101'0101'0101UL << 0, BB_FILE_B = 0x0101'0101'0101'0101UL << 1, BB_FILE_C = 0x0101'0101'0101'0101UL << 2, BB_FILE_D = 0x0101'0101'0101'0101UL << 3, BB_FILE_E = 0x0101'0101'0101'0101UL << 4, BB_FILE_F = 0x0101'0101'0101'0101UL << 5, BB_FILE_G = 0x0101'0101'0101'0101UL << 6, BB_FILE_H = 0x0101'0101'0101'0101UL << 7;

    const Bitboard BB_RANKS[] = {
        0xffUL << (8 * 0),
        0xffUL << (8 * 1),
        0xffUL << (8 * 2),
        0xffUL << (8 * 3),
        0xffUL << (8 * 4),
        0xffUL << (8 * 5),
        0xffUL << (8 * 6),
        0xffUL << (8 * 7),
    },
                   BB_RANK_1 = 0xffUL << (8 * 0), BB_RANK_2 = 0xffUL << (8 * 1), BB_RANK_3 = 0xffUL << (8 * 2), BB_RANK_4 = 0xffUL << (8 * 3), BB_RANK_5 = 0xffUL << (8 * 4), BB_RANK_6 = 0xffUL << (8 * 5), BB_RANK_7 = 0xffUL << (8 * 6), BB_RANK_8 = 0xffUL << (8 * 7);

    const Bitboard BB_BACKRANKS = BB_RANK_1 | BB_RANK_8;

    int lsb(Bitboard);

    std::vector<Square> scan_forward(Bitboard);

    int msb(Bitboard);

    std::vector<Square> scan_reversed(Bitboard);

    std::function<int(Bitboard)> popcount = [](Bitboard bb) -> int
    { return std::bitset<64>(bb).count(); };

    Bitboard flip_vertical(Bitboard);

    Bitboard flip_horizontal(Bitboard);

    Bitboard flip_diagonal(Bitboard);

    Bitboard flip_anti_diagonal(Bitboard);

    Bitboard shift_down(Bitboard);

    Bitboard shift_2_down(Bitboard);

    Bitboard shift_up(Bitboard);

    Bitboard shift_2_up(Bitboard);

    Bitboard shift_right(Bitboard);

    Bitboard shift_2_right(Bitboard);

    Bitboard shift_left(Bitboard);

    Bitboard shift_2_left(Bitboard);

    Bitboard shift_up_left(Bitboard);

    Bitboard shift_up_right(Bitboard);

    Bitboard shift_down_left(Bitboard);

    Bitboard shift_down_right(Bitboard);

    Bitboard _sliding_attacks(Square, Bitboard, const std::vector<int> &);

    Bitboard _step_attacks(Square, const std::vector<int> &);

    const Bitboard BB_KNIGHT_ATTACKS[] = {_step_attacks(0, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(1, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(2, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(3, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(4, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(5, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(6, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(7, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(8, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(9, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(10, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(11, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(12, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(13, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(14, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(15, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(16, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(17, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(18, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(19, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(20, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(21, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(22, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(23, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(24, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(25, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(26, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(27, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(28, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(29, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(30, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(31, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(32, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(33, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(34, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(35, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(36, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(37, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(38, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(39, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(40, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(41, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(42, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(43, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(44, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(45, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(46, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(47, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(48, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(49, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(50, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(51, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(52, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(53, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(54, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(55, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(56, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(57, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(58, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(59, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(60, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(61, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(62, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(63, {17, 15, 10, 6, -17, -15, -10, -6})};
    const Bitboard BB_KING_ATTACKS[] = {_step_attacks(0, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(1, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(2, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(3, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(4, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(5, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(6, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(7, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(8, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(9, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(10, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(11, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(12, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(13, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(14, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(15, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(16, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(17, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(18, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(19, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(20, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(21, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(22, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(23, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(24, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(25, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(26, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(27, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(28, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(29, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(30, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(31, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(32, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(33, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(34, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(35, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(36, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(37, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(38, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(39, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(40, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(41, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(42, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(43, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(44, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(45, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(46, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(47, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(48, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(49, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(50, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(51, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(52, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(53, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(54, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(55, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(56, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(57, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(58, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(59, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(60, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(61, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(62, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(63, {9, 8, 7, 1, -9, -8, -7, -1})};
    const Bitboard BB_PAWN_ATTACKS[][64] = {{_step_attacks(0, {-7, -9}), _step_attacks(1, {-7, -9}), _step_attacks(2, {-7, -9}), _step_attacks(3, {-7, -9}), _step_attacks(4, {-7, -9}), _step_attacks(5, {-7, -9}), _step_attacks(6, {-7, -9}), _step_attacks(7, {-7, -9}), _step_attacks(8, {-7, -9}), _step_attacks(9, {-7, -9}), _step_attacks(10, {-7, -9}), _step_attacks(11, {-7, -9}), _step_attacks(12, {-7, -9}), _step_attacks(13, {-7, -9}), _step_attacks(14, {-7, -9}), _step_attacks(15, {-7, -9}), _step_attacks(16, {-7, -9}), _step_attacks(17, {-7, -9}), _step_attacks(18, {-7, -9}), _step_attacks(19, {-7, -9}), _step_attacks(20, {-7, -9}), _step_attacks(21, {-7, -9}), _step_attacks(22, {-7, -9}), _step_attacks(23, {-7, -9}), _step_attacks(24, {-7, -9}), _step_attacks(25, {-7, -9}), _step_attacks(26, {-7, -9}), _step_attacks(27, {-7, -9}), _step_attacks(28, {-7, -9}), _step_attacks(29, {-7, -9}), _step_attacks(30, {-7, -9}), _step_attacks(31, {-7, -9}), _step_attacks(32, {-7, -9}), _step_attacks(33, {-7, -9}), _step_attacks(34, {-7, -9}), _step_attacks(35, {-7, -9}), _step_attacks(36, {-7, -9}), _step_attacks(37, {-7, -9}), _step_attacks(38, {-7, -9}), _step_attacks(39, {-7, -9}), _step_attacks(40, {-7, -9}), _step_attacks(41, {-7, -9}), _step_attacks(42, {-7, -9}), _step_attacks(43, {-7, -9}), _step_attacks(44, {-7, -9}), _step_attacks(45, {-7, -9}), _step_attacks(46, {-7, -9}), _step_attacks(47, {-7, -9}), _step_attacks(48, {-7, -9}), _step_attacks(49, {-7, -9}), _step_attacks(50, {-7, -9}), _step_attacks(51, {-7, -9}), _step_attacks(52, {-7, -9}), _step_attacks(53, {-7, -9}), _step_attacks(54, {-7, -9}), _step_attacks(55, {-7, -9}), _step_attacks(56, {-7, -9}), _step_attacks(57, {-7, -9}), _step_attacks(58, {-7, -9}), _step_attacks(59, {-7, -9}), _step_attacks(60, {-7, -9}), _step_attacks(61, {-7, -9}), _step_attacks(62, {-7, -9}), _step_attacks(63, {-7, -9})}, {_step_attacks(0, {7, 9}), _step_attacks(1, {7, 9}), _step_attacks(2, {7, 9}), _step_attacks(3, {7, 9}), _step_attacks(4, {7, 9}), _step_attacks(5, {7, 9}), _step_attacks(6, {7, 9}), _step_attacks(7, {7, 9}), _step_attacks(8, {7, 9}), _step_attacks(9, {7, 9}), _step_attacks(10, {7, 9}), _step_attacks(11, {7, 9}), _step_attacks(12, {7, 9}), _step_attacks(13, {7, 9}), _step_attacks(14, {7, 9}), _step_attacks(15, {7, 9}), _step_attacks(16, {7, 9}), _step_attacks(17, {7, 9}), _step_attacks(18, {7, 9}), _step_attacks(19, {7, 9}), _step_attacks(20, {7, 9}), _step_attacks(21, {7, 9}), _step_attacks(22, {7, 9}), _step_attacks(23, {7, 9}), _step_attacks(24, {7, 9}), _step_attacks(25, {7, 9}), _step_attacks(26, {7, 9}), _step_attacks(27, {7, 9}), _step_attacks(28, {7, 9}), _step_attacks(29, {7, 9}), _step_attacks(30, {7, 9}), _step_attacks(31, {7, 9}), _step_attacks(32, {7, 9}), _step_attacks(33, {7, 9}), _step_attacks(34, {7, 9}), _step_attacks(35, {7, 9}), _step_attacks(36, {7, 9}), _step_attacks(37, {7, 9}), _step_attacks(38, {7, 9}), _step_attacks(39, {7, 9}), _step_attacks(40, {7, 9}), _step_attacks(41, {7, 9}), _step_attacks(42, {7, 9}), _step_attacks(43, {7, 9}), _step_attacks(44, {7, 9}), _step_attacks(45, {7, 9}), _step_attacks(46, {7, 9}), _step_attacks(47, {7, 9}), _step_attacks(48, {7, 9}), _step_attacks(49, {7, 9}), _step_attacks(50, {7, 9}), _step_attacks(51, {7, 9}), _step_attacks(52, {7, 9}), _step_attacks(53, {7, 9}), _step_attacks(54, {7, 9}), _step_attacks(55, {7, 9}), _step_attacks(56, {7, 9}), _step_attacks(57, {7, 9}), _step_attacks(58, {7, 9}), _step_attacks(59, {7, 9}), _step_attacks(60, {7, 9}), _step_attacks(61, {7, 9}), _step_attacks(62, {7, 9}), _step_attacks(63, {7, 9})}};

    Bitboard _edges(Square);

    std::vector<Bitboard> _carry_rippler(Bitboard);

    std::tuple<std::vector<Bitboard>, std::vector<std::unordered_map<Bitboard, Bitboard>>> _attack_table(const std::vector<int> &);

    const auto [BB_DIAG_MASKS, BB_DIAG_ATTACKS] = _attack_table({-9, -7, 7, 9});
    const auto [BB_FILE_MASKS, BB_FILE_ATTACKS] = _attack_table({-8, 8});
    const auto [BB_RANK_MASKS, BB_RANK_ATTACKS] = _attack_table({-1, 1});

    std::vector<std::vector<Bitboard>> _rays();

    const std::vector<std::vector<Bitboard>> BB_RAYS = _rays();

    Bitboard ray(Square, Square);

    Bitboard between(Square, Square);

    const std::regex SAN_REGEX(R"(^([NBKRQ])?([a-h])?([1-8])?[\-x]?([a-h][1-8])(=?[nbrqkNBRQK])?[\+#]?$)");

    const std::regex FEN_CASTLING_REGEX(R"(^(?:-|[KQABCDEFGH]{0,2}[kqabcdefgh]{0,2})$)");

    class Piece
    {
        /* A piece with type and color. */

    public:
        PieceType piece_type;
        /* The piece type. */

        Color color;
        /* The piece color. */

        Piece(PieceType, Color);

        Piece();

        char symbol() const;

        std::string unicode_symbol(bool = false) const;

        operator std::string() const;

        static Piece from_symbol(char);
    };

    std::ostream &operator<<(std::ostream &, const Piece &);

    class Move
    {
        /*
        Represents a move from a square to a square and possibly the promotion
        piece type.

        Drops and null moves are supported.
        */

    public:
        Square from_square;
        /* The source square. */

        Square to_square;
        /* The target square. */

        std::optional<PieceType> promotion;
        /* The promotion piece type or ``std::nullopt``. */

        std::optional<PieceType> drop;
        /* The drop piece type or ``std::nullopt``. */

        Move(Square, Square, std::optional<PieceType> = std::nullopt, std::optional<PieceType> = std::nullopt);

        std::string uci() const;

        std::string xboard() const;

        operator bool() const;

        operator std::string() const;

        static Move from_uci(const std::string &);

        static Move null();
    };

    std::ostream &operator<<(std::ostream &, const Move &);

    class SquareSet;

    class BaseBoard
    {
        /*
        A board representing the position of chess pieces. See
        :class:`~chess::Board` for a full board with move generation.

        The board is initialized with the standard chess starting position, unless
        otherwise specified in the optional *board_fen* argument. If *board_fen*
        is ``std::nullopt``, an empty board is created.
        */

    public:
        Bitboard occupied_co[2], pawns, knights, bishops, rooks, queens, kings, promoted, occupied;
        BaseBoard(const std::optional<std::string> & = STARTING_BOARD_FEN);

        void reset_board();

        void clear_board();

        Bitboard pieces_mask(PieceType, Color) const;

        SquareSet pieces(PieceType, Color) const;

        std::optional<Piece> piece_at(Square) const;

        std::optional<PieceType> piece_type_at(Square) const;

        std::optional<Color> color_at(Square) const;

        std::optional<Square> king(Color) const;

        Bitboard attacks_mask(Square) const;

        SquareSet attacks(Square) const;

        Bitboard attackers_mask(Color, Square) const;

        bool is_attacked_by(Color, Square) const;

        SquareSet attackers(Color, Square) const;

        Bitboard pin_mask(Color, Square) const;

        SquareSet pin(Color, Square) const;

        bool is_pinned(Color, Square) const;

        std::optional<Piece> remove_piece_at(Square);

        void set_piece_at(Square, const std::optional<Piece> &, bool = false);

        std::string board_fen(std::optional<bool> = false) const;

        void set_board_fen(const std::string &);

        std::unordered_map<Square, Piece> piece_map(Bitboard = BB_ALL) const;

        void set_piece_map(const std::unordered_map<Square, Piece> &);

        void set_chess960_pos(int);

        std::optional<int> chess960_pos() const;

        operator std::string() const;

        std::string unicode(bool = false, bool = false, const std::string & = "⭘") const;

        bool operator==(const BaseBoard &) const;

        void apply_transform(const std::function<Bitboard(Bitboard)> &);

        BaseBoard transform(const std::function<Bitboard(Bitboard)> &) const;

        void apply_mirror();

        BaseBoard mirror() const;

        BaseBoard copy() const;

        static BaseBoard empty();

        static BaseBoard from_chess960_pos(int);

    protected:
        void _reset_board();

        void _clear_board();

        Bitboard _attackers_mask(Color, Square, Bitboard) const;

        std::optional<PieceType> _remove_piece_at(Square);

        void _set_piece_at(Square, PieceType, Color, bool = false);

        void _set_board_fen(std::string);

        void _set_piece_map(const std::unordered_map<Square, Piece> &);

        void _set_chess960_pos(int);
    };

    std::ostream &operator<<(std::ostream &, const BaseBoard &);

    class Board;

    class _BoardState
    {

    public:
        Bitboard pawns, knights, bishops, rooks, queens, kings, occupied_w, occupied_b, occupied, promoted;
        Color turn;
        Bitboard castling_rights;
        std::optional<Square> ep_square;
        int halfmove_clock, fullmove_number;
        _BoardState(const Board &);

        void restore(Board &) const;
    };

    class LegalMoveGenerator;
    class PseudoLegalMoveGenerator;

    class Board : public BaseBoard
    {
        /*
        A :class:`~chess::BaseBoard`, additional information representing
        a chess position, and a :data:`move stack <chess::Board::move_stack>`.

        Provides :func:`move generation <chess::Board::legal_moves()>`, validation,
        :func:`parsing <chess::Board::parse_san()>`, attack generation,
        :func:`game end detection <chess::Board::is_game_over()>`,
        and the capability to :func:`make <chess::Board::push()>` and
        :func:`unmake <chess::Board::pop()>` moves.

        The board is initialized to the standard chess starting position,
        unless otherwise specified in the optional *fen* argument.
        If *fen* is ``std::nullopt``, an empty board is created.

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
        static std::string aliases[];
        static std::optional<std::string> uci_variant;
        static std::optional<std::string> xboard_variant;
        static std::string starting_fen;

        static std::optional<std::string> tbw_suffix;
        static std::optional<std::string> tbz_suffix;
        static std::optional<std::array<unsigned char, 4>> tbw_magic;
        static std::optional<std::array<unsigned char, 4>> tbz_magic;
        static std::optional<std::string> pawnless_tbw_suffix;
        static std::optional<std::string> pawnless_tbz_suffix;
        static std::optional<std::array<unsigned char, 4>> pawnless_tbw_magic;
        static std::optional<std::array<unsigned char, 4>> pawnless_tbz_magic;
        static bool connected_kings;
        static bool one_king;
        static bool captures_compulsory;

        Color turn;
        /* The side to move (``chess::WHITE`` or ``chess::BLACK``). */

        Bitboard castling_rights;
        /*
        Bitmask of the rooks with castling rights.

        To test for specific squares:

        >>> #include "chess.cpp"
        >>> #include <iostream>
        >>>
        >>> chess::Board board;
        >>> std::cout << bool(board.castling_rights & chess::BB_H1);  // White can castle with the h1 rook
        1

        To add a specific square:

        >>> board.castling_rights |= chess::BB_A1;

        Use :func:`~chess::Board::set_castling_fen()` to set multiple castling
        rights. Also see :func:`~chess::Board::has_castling_rights()`,
        :func:`~chess::Board::has_kingside_castling_rights()`,
        :func:`~chess::Board::has_queenside_castling_rights()`,
        :func:`~chess::Board::has_chess960_castling_rights()`,
        :func:`~chess::Board::clean_castling_rights()`.
        */

        std::optional<Square> ep_square;
        /*
        The potential en passant square on the third or sixth rank or ``std::nullopt``.

        Use :func:`~chess::Board::has_legal_en_passant()` to test if en passant
        capturing would actually be possible on the next move.
        */

        int fullmove_number;
        /*
        Counts move pairs. Starts at `1` and is incremented after every move
        of the black side.
        */

        int halfmove_clock;
        /* The number of half-moves since the last capture or pawn move. */

        bool chess960;
        /*
        Whether the board is in Chess960 mode. In Chess960 castling moves are
        represented as king moves to the corresponding rook square.
        */

        std::vector<Move> move_stack;
        /*
        The move stack. Use :func:`Board::push() <chess::Board::push()>`,
        :func:`Board::pop() <chess::Board::pop()>`,
        :func:`Board::peek() <chess::Board::peek()>` and
        :func:`Board::clear_stack() <chess::Board::clear_stack()>` for
        manipulation.
        */

        Board(const std::optional<std::string> & = STARTING_FEN, bool = false);

        LegalMoveGenerator legal_moves() const;

        PseudoLegalMoveGenerator pseudo_legal_moves() const;

        void reset();

        void reset_board();

        void clear();

        void clear_board();

        void clear_stack();

        Board root() const;

        int ply() const;

        std::optional<Piece> remove_piece_at(Square);

        void set_piece_at(Square, const std::optional<Piece> &, bool = false);

        std::vector<Move> generate_pseudo_legal_moves(Bitboard = BB_ALL, Bitboard = BB_ALL) const;

        std::vector<Move> generate_pseudo_legal_ep(Bitboard = BB_ALL, Bitboard = BB_ALL) const;

        std::vector<Move> generate_pseudo_legal_captures(Bitboard = BB_ALL, Bitboard = BB_ALL) const;

        Bitboard checkers_mask() const;

        SquareSet checkers() const;

        bool is_check() const;

        bool gives_check(const Move &);

        bool is_into_check(const Move &) const;

        bool was_into_check() const;

        bool is_pseudo_legal(Move) const;

        bool is_legal(const Move &) const;

        bool is_variant_end() const;

        bool is_variant_loss() const;

        bool is_variant_win() const;

        bool is_variant_draw() const;

        bool is_game_over(bool = false);

        std::string result(bool = false);

        std::optional<Outcome> outcome(bool = false);

        bool is_checkmate() const;

        bool is_stalemate() const;

        bool is_insufficient_material() const;

        bool has_insufficient_material(Color) const;

        bool is_seventyfive_moves() const;

        bool is_fivefold_repetition();

        bool can_claim_draw();

        bool is_fifty_moves() const;

        bool can_claim_fifty_moves();

        bool can_claim_threefold_repetition();

        bool is_repetition(int = 3);

        void push(Move);

        Move pop();

        Move peek() const;

        Move find_move(Square, Square, std::optional<PieceType> = std::nullopt);

        std::string castling_shredder_fen() const;

        std::string castling_xfen() const;

        bool has_pseudo_legal_en_passant() const;

        bool has_legal_en_passant() const;

        std::string fen(bool = false, _EnPassantSpec = "legal", std::optional<bool> = std::nullopt);

        std::string shredder_fen(_EnPassantSpec = "legal", std::optional<bool> = std::nullopt);

        void set_fen(const std::string &);

        void set_castling_fen(const std::string &);

        void set_board_fen(const std::string &);

        void set_piece_map(const std::unordered_map<Square, Piece> &);

        void set_chess960_pos(int);

        std::optional<int> chess960_pos(bool = false, bool = false, bool = true) const;

        std::string epd(bool = false, const _EnPassantSpec & = "legal", std::optional<bool> = std::nullopt, const std::unordered_map<std::string, std::variant<std::nullopt_t, std::string, int, float, Move, std::vector<Move>>> & = {});

        std::unordered_map<std::string, std::variant<std::nullopt_t, std::string, int, float, Move, std::vector<Move>>> set_epd(const std::string &);

        std::string san(const Move &);

        std::string lan(const Move &);

        std::string san_and_push(const Move &);

        std::string variation_san(const std::vector<Move> &) const;

        Move parse_san(const std::string &);

        Move push_san(const std::string &);

        std::string uci(Move, std::optional<bool> = std::nullopt) const;

        Move parse_uci(const std::string &);

        Move push_uci(const std::string &);

        std::string xboard(const Move &, std::optional<bool> = std::nullopt) const;

        Move parse_xboard(const std::string &);

        Move push_xboard(const std::string &);

        bool is_en_passant(const Move &) const;

        bool is_capture(const Move &) const;

        bool is_zeroing(const Move &) const;

        bool is_irreversible(const Move &) const;

        bool is_castling(const Move &) const;

        bool is_kingside_castling(const Move &) const;

        bool is_queenside_castling(const Move &) const;

        Bitboard clean_castling_rights() const;

        bool has_castling_rights(Color) const;

        bool has_kingside_castling_rights(Color) const;

        bool has_queenside_castling_rights(Color) const;

        bool has_chess960_castling_rights();

        Status status() const;

        bool is_valid() const;

        std::vector<Move> generate_legal_moves(Bitboard = BB_ALL, Bitboard = BB_ALL) const;

        std::vector<Move> generate_legal_ep(Bitboard = BB_ALL, Bitboard = BB_ALL) const;

        std::vector<Move> generate_legal_captures(Bitboard = BB_ALL, Bitboard = BB_ALL) const;

        std::vector<Move> generate_castling_moves(Bitboard = BB_ALL, Bitboard = BB_ALL) const;

        bool operator==(const Board &) const;

        void apply_transform(const std::function<Bitboard(Bitboard)> &);

        Board transform(const std::function<Bitboard(Bitboard)> &) const;

        void apply_mirror();

        Board mirror() const;

        Board copy(std::variant<bool, int> = true) const;

        static Board empty(bool = false);

        static std::tuple<Board, std::unordered_map<std::string, std::variant<std::nullopt_t, std::string, int, float, Move, std::vector<Move>>>> from_epd(const std::string &, bool = false);

        static Board from_chess960_pos(int);

    private:
        std::vector<_BoardState> _stack;

        bool _is_halfmoves(int) const;

        _BoardState _board_state() const;

        void _push_capture(const Move &, Square, PieceType, bool) const;

        void _set_castling_fen(const std::string &);

        std::string _epd_operations(const std::unordered_map<std::string, std::variant<std::nullopt_t, std::string, int, float, Move, std::vector<Move>>> &);

        std::unordered_map<std::string, std::variant<std::nullopt_t, std::string, int, float, Move, std::vector<Move>>> _parse_epd_ops(const std::string &, const std::function<Board()> &) const;

        std::string _algebraic(const Move &, bool = false);

        std::string _algebraic_and_push(const Move &, bool = false);

        std::string _algebraic_without_suffix(const Move &, bool = false);

        bool _reduces_castling_rights(const Move &) const;

        std::optional<Square> _valid_ep_square() const;

        bool _ep_skewered(Square, Square) const;

        Bitboard _slider_blockers(Square) const;

        bool _is_safe(Square, Bitboard, const Move &) const;

        std::vector<Move> _generate_evasions(Square, Bitboard, Bitboard = BB_ALL, Bitboard = BB_ALL) const;

        bool _attacked_for_king(Bitboard, Bitboard) const;

        Move _from_chess960(bool, Square, Square, std::optional<PieceType> = std::nullopt, std::optional<PieceType> = std::nullopt) const;

        Move _to_chess960(const Move &) const;

        std::tuple<Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Color, Bitboard, Square> _transposition_key() const;
    };

    std::ostream &operator<<(std::ostream &, Board);

    class PseudoLegalMoveGenerator
    {

    public:
        PseudoLegalMoveGenerator(const Board &);

        operator bool() const;

        int count() const;

        std::vector<Move>::const_iterator begin() const;

        std::vector<Move>::const_iterator end() const;

        Board get_board() const;

        operator std::vector<Move>() const;

    private:
        Board _board;
        std::vector<Move> _iter;
    };

    std::ostream &operator<<(std::ostream &, PseudoLegalMoveGenerator);

    class LegalMoveGenerator
    {

    public:
        LegalMoveGenerator(const Board &);

        operator bool() const;

        int count() const;

        std::vector<Move>::const_iterator begin() const;

        std::vector<Move>::const_iterator end() const;

        Board get_board() const;

        operator std::vector<Move>() const;

    private:
        Board _board;
        std::vector<Move> _iter;
    };

    std::ostream &operator<<(std::ostream &, LegalMoveGenerator);

    typedef std::variant<Bitboard, SquareSet, std::vector<Square>> IntoSquareSet;

    class SquareSet
    {
        /*
        A set of squares.

        Square sets are internally represented by 64-bit integer masks of the
        included squares. Bitwise operations can be used to compute unions,
        intersections and shifts.

        >>> #include "chess.cpp"
        >>> #include <iostream>
        >>>
        >>> chess::SquareSet squares = chess::SquareSet({chess::A8, chess::A1});
        >>> std::cout << squares;
        SquareSet(0x0100_0000_0000_0001)

        >>> squares = chess::SquareSet(chess::BB_A8 | chess::BB_RANK_1);
        >>> std::cout << squares;
        SquareSet(0x0100_0000_0000_00ff)

        >>> std::cout << std::string(squares);
        1 . . . . . . .
        . . . . . . . .
        . . . . . . . .
        . . . . . . . .
        . . . . . . . .
        . . . . . . . .
        . . . . . . . .
        1 1 1 1 1 1 1 1

        >>> std::cout << std::size(squares);
        9

        >>> std::cout << bool(squares);
        1

        >>> std::cout << (std::find(std::begin(squares), std::end(squares), chess::B1) != std::end(squares));
        1

        >>> for (chess::Square square : squares) {
        ...     // 0 -- chess::A1
        ...     // 1 -- chess::B1
        ...     // 2 -- chess::C1
        ...     // 3 -- chess::D1
        ...     // 4 -- chess::E1
        ...     // 5 -- chess::F1
        ...     // 6 -- chess::G1
        ...     // 7 -- chess::H1
        ...     // 56 -- chess::A8
        ...     std::cout << std::string(square);
        >>> }
        0
        1
        2
        3
        4
        5
        6
        7
        56

        >>> vector<Square>(squares);

        Square sets are internally represented by 64-bit integer masks of the
        included squares. Bitwise operations can be used to compute unions,
        intersections and shifts.

        >>> std::cout << (unsigned long)(squares);
        72057594037928191

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
        SquareSet(const IntoSquareSet & = BB_EMPTY);

        // Set

        std::vector<Square>::const_iterator begin() const;

        std::vector<Square>::const_iterator end() const;

        size_t size() const;

        // MutableSet

        void add(Square);

        void discard(Square);

        // frozenset

        bool isdisjoint(const IntoSquareSet &) const;

        bool issubset(const IntoSquareSet &) const;

        bool issuperset(const IntoSquareSet &) const;

        SquareSet union_(const IntoSquareSet &) const;

        SquareSet operator|(const IntoSquareSet &) const;

        SquareSet intersection(const IntoSquareSet &) const;

        SquareSet operator&(const IntoSquareSet &) const;

        SquareSet difference(const IntoSquareSet &) const;

        SquareSet operator-(const IntoSquareSet &) const;

        SquareSet symmetric_difference(const IntoSquareSet &) const;

        SquareSet operator^(const IntoSquareSet &) const;

        SquareSet copy() const;

        // set

        void update(const std::initializer_list<IntoSquareSet> &);

        SquareSet operator|=(const IntoSquareSet &);

        void intersection_update(const std::initializer_list<IntoSquareSet> &);

        SquareSet operator&=(const IntoSquareSet &);

        void difference_update(const IntoSquareSet &);

        SquareSet operator-=(const IntoSquareSet &);

        void symmetric_difference_update(const IntoSquareSet &);

        SquareSet operator^=(const IntoSquareSet &);

        void remove(Square);

        Square pop();

        void clear();

        // SquareSet

        std::vector<Bitboard> carry_rippler() const;

        SquareSet mirror() const;

        std::array<bool, 64> tolist();

        operator bool() const;

        bool operator==(const Bitboard &) const;

        bool operator==(const SquareSet &) const;

        bool operator==(const std::vector<Square> &) const;

        SquareSet operator<<(int) const;

        SquareSet operator>>(int) const;

        SquareSet operator<<=(int);

        SquareSet operator>>=(int);

        SquareSet operator~() const;

        operator unsigned long int() const;

        operator std::string() const;

        Bitboard get_mask() const;

        operator std::vector<Square>() const;

        static SquareSet ray(Square, Square);

        static SquareSet between(Square, Square);

        static SquareSet from_square(Square);

    private:
        Bitboard _mask;
        std::vector<Square> _iter;
    };

    std::ostream &operator<<(std::ostream &, const SquareSet &);
}

template <>
struct std::hash<chess::Piece>
{
    int operator()(const chess::Piece &piece) const
    {
        return piece.piece_type + (piece.color ? -1 : 5);
    }
};

#endif