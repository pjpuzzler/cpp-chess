/*
This is a complete remake of niklasf's 'python-chess' in C++
The original version can be found here: https://github.com/niklasf/python-chess
*/

/*
A chess library with move generation and validation,
and XBoard/UCI engine communication.
*/

#include "chess.h"
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <bitset>
#include <vector>
#include <tuple>
#include <regex>
#include <functional>
#include <optional>
#include <sstream>
#include <queue>
#include <stack>
#include <variant>

#include <iostream>

namespace std {
    namespace chess {
        string __author__ = "Patrick Johnson";

        string __email__ = "pjpuzzler@gmail.com";

        string __version__ = "1.0.0";

        typedef string _EnPassantSpec;


        typedef bool Color;
        const Color COLORS[] = {true, false}, WHITE = true, BLACK = false;
        const string COLOR_NAMES[] = {"black", "white"};

        typedef int PieceType;
        const PieceType PIECE_TYPES[] = {1, 2, 3, 4, 5, 6}, PAWN = 1, KNIGHT = 2, BISHOP = 3, ROOK = 4, QUEEN = 5, KING = 6;
        const char PIECE_SYMBOLS[] = {'\0', 'p', 'n', 'b', 'r', 'q', 'k'};
        const string PIECE_NAMES[] = {"", "pawn", "knight", "bishop", "rook", "queen", "king"};

        char piece_symbol(PieceType piece_type) {
            return PIECE_SYMBOLS[piece_type];
        }


        string piece_name(PieceType piece_type) {
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
        /* The FEN for the standard chess starting position. */

        const string STARTING_BOARD_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";
        /* The board part of the FEN for the standard chess starting position. */


        enum class Status {
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


        enum class Termination {
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

        class Outcome {
            /*
            Information about the outcome of an ended game, usually obtained from
            :func:`chess::Board::outcome()`.
            */

        public:
            Termination termination;
            /* The reason for the game to have ended. */

            optional<Color> winner;
            /* The winning color or ``std::nullopt`` if drawn. */

            Outcome(Termination termination, optional<Color> winner) : termination(termination), winner(winner) {}

            string result() const {
                /* Returns ``1-0``, ``0-1`` or ``1/2-1/2``. */
                return this->winner == nullopt ? "1/2-1/2" : (*this->winner ? "1-0" : "0-1");
            }
        };



        typedef int Square;
        const Square SQUARES[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63}, A1 = 0, B1 = 1, C1 = 2, D1 = 3, E1 = 4, F1 = 5, G1 = 6, H1 = 7, A2 = 8, B2 = 9, C2 = 10, D2 = 11, E2 = 12, F2 = 13, G2 = 14, H2 = 15, A3 = 16, B3 = 17, C3 = 18, D3 = 19, E3 = 20, F3 = 21, G3 = 22, H3 = 23, A4 = 24, B4 = 25, C4 = 26, D4 = 27, E4 = 28, F4 = 29, G4 = 30, H4 = 31, A5 = 32, B5 = 33, C5 = 34, D5 = 35, E5 = 36, F5 = 37, G5 = 38, H5 = 39, A6 = 40, B6 = 41, C6 = 42, D6 = 43, E6 = 44, F6 = 45, G6 = 46, H6 = 47, A7 = 48, B7 = 49, C7 = 50, D7 = 51, E7 = 52, F7 = 53, G7 = 54, H7 = 55, A8 = 56, B8 = 57, C8 = 58, D8 = 59, E8 = 60, F8 = 61, G8 = 62, H8 = 63;

        const string SQUARE_NAMES[] = {"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2", "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3", "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4", "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5", "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6", "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7", "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"};

        Square parse_square(const string &name) {
            /*
            Gets the square index for the given square *name*
            (e.g., ``a1`` returns ``0``).

            :throws: :exc:`std::invalid_argument` if the square name is invalid.
            */
            auto it = find(begin(SQUARE_NAMES), end(SQUARE_NAMES), name);
            if (it == end(SQUARE_NAMES)) {
                throw invalid_argument("square name is invalid");
            }
            return distance(SQUARE_NAMES, it);
        }


        string square_name(Square square) {
            /* Gets the name of the square, like ``a3``. */
            return SQUARE_NAMES[square];
        }


        Square square(int file_index, int rank_index) {
            /* Gets a square number by file and rank index. */
            return rank_index * 8 + file_index;
        }


        int square_file(Square square) {
            /* Gets the file index of the square where ``0`` is the a-file. */
            return square & 7;
        }


        int square_rank(Square square) {
            /* Gets the rank index of the square where ``0`` is the first rank. */
            return square >> 3;
        }


        int square_distance(Square a, Square b) {
            /*
            Gets the distance (i.e., the number of king steps) from square *a* to *b*.
            */
            return max(abs(square_file(a) - square_file(b)), abs(square_rank(a) - square_rank(b)));
        }


        Square square_mirror(Square square) {
            /* Mirrors the square vertically. */
            return square ^ 0x38;
        }


        const Square SQUARES_180[] = {square_mirror(0), square_mirror(1), square_mirror(2), square_mirror(3), square_mirror(4), square_mirror(5), square_mirror(6), square_mirror(7), square_mirror(8), square_mirror(9), square_mirror(10), square_mirror(11), square_mirror(12), square_mirror(13), square_mirror(14), square_mirror(15), square_mirror(16), square_mirror(17), square_mirror(18), square_mirror(19), square_mirror(20), square_mirror(21), square_mirror(22), square_mirror(23), square_mirror(24), square_mirror(25), square_mirror(26), square_mirror(27), square_mirror(28), square_mirror(29), square_mirror(30), square_mirror(31), square_mirror(32), square_mirror(33), square_mirror(34), square_mirror(35), square_mirror(36), square_mirror(37), square_mirror(38), square_mirror(39), square_mirror(40), square_mirror(41), square_mirror(42), square_mirror(43), square_mirror(44), square_mirror(45), square_mirror(46), square_mirror(47), square_mirror(48), square_mirror(49), square_mirror(50), square_mirror(51), square_mirror(52), square_mirror(53), square_mirror(54), square_mirror(55), square_mirror(56), square_mirror(57), square_mirror(58), square_mirror(59), square_mirror(60), square_mirror(61), square_mirror(62), square_mirror(63)};


        typedef long Bitboard;
        const Bitboard BB_EMPTY = 0;
        const Bitboard BB_ALL = 0xffff'ffff'ffff'ffff;

        const Bitboard BB_SQUARES[] = {
            1UL << 0, 1UL << 1, 1UL << 2, 1UL << 3, 1UL << 4, 1UL << 5, 1UL << 6, 1UL << 7,
            1UL << 8, 1UL << 9, 1UL << 10, 1UL << 11, 1UL << 12, 1UL << 13, 1UL << 14, 1UL << 15,
            1UL << 16, 1UL << 17, 1UL << 18, 1UL << 19, 1UL << 20, 1UL << 21, 1UL << 22, 1UL << 23,
            1UL << 24, 1UL << 25, 1UL << 26, 1UL << 27, 1UL << 28, 1UL << 29, 1UL << 30, 1UL << 31,
            1UL << 32, 1UL << 33, 1UL << 34, 1UL << 35, 1UL << 36, 1UL << 37, 1UL << 38, 1UL << 39,
            1UL << 40, 1UL << 41, 1UL << 42, 1UL << 43, 1UL << 44, 1UL << 45, 1UL << 46, 1UL << 47,
            1UL << 48, 1UL << 49, 1UL << 50, 1UL << 51, 1UL << 52, 1UL << 53, 1UL << 54, 1UL << 55,
            1UL << 56, 1UL << 57, 1UL << 58, 1UL << 59, 1UL << 60, 1UL << 61, 1UL << 62, 1UL << 63,
        }, BB_A1 = 1UL << 0, BB_B1 = 1UL << 1, BB_C1 = 1UL << 2, BB_D1 = 1UL << 3, BB_E1 = 1UL << 4, BB_F1 = 1UL << 5, BB_G1 = 1UL << 6, BB_H1 = 1UL << 7, BB_A2 = 1UL << 8, BB_B2 = 1UL << 9, BB_C2 = 1UL << 10, BB_D2 = 1UL << 11, BB_E2 = 1UL << 12, BB_F2 = 1UL << 13, BB_G2 = 1UL << 14, BB_H2 = 1UL << 15, BB_A3 = 1UL << 16, BB_B3 = 1UL << 17, BB_C3 = 1UL << 18, BB_D3 = 1UL << 19, BB_E3 = 1UL << 20, BB_F3 = 1UL << 21, BB_G3 = 1UL << 22, BB_H3 = 1UL << 23, BB_A4 = 1UL << 24, BB_B4 = 1UL << 25, BB_C4 = 1UL << 26, BB_D4 = 1UL << 27, BB_E4 = 1UL << 28, BB_F4 = 1UL << 29, BB_G4 = 1UL << 30, BB_H4 = 1UL << 31, BB_A5 = 1UL << 32, BB_B5 = 1UL << 33, BB_C5 = 1UL << 34, BB_D5 = 1UL << 35, BB_E5 = 1UL << 36, BB_F5 = 1UL << 37, BB_G5 = 1UL << 38, BB_H5 = 1UL << 39, BB_A6 = 1UL << 40, BB_B6 = 1UL << 41, BB_C6 = 1UL << 42, BB_D6 = 1UL << 43, BB_E6 = 1UL << 44, BB_F6 = 1UL << 45, BB_G6 = 1UL << 46, BB_H6 = 1UL << 47, BB_A7 = 1UL << 48, BB_B7 = 1UL << 49, BB_C7 = 1UL << 50, BB_D7 = 1UL << 51, BB_E7 = 1UL << 52, BB_F7 = 1UL << 53, BB_G7 = 1UL << 54, BB_H7 = 1UL << 55, BB_A8 = 1UL << 56, BB_B8 = 1UL << 57, BB_C8 = 1UL << 58, BB_D8 = 1UL << 59, BB_E8 = 1UL << 60, BB_F8 = 1UL << 61, BB_G8 = 1UL << 62, BB_H8 = 1UL << 63;

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
        }, BB_FILE_A = 0x0101'0101'0101'0101UL << 0, BB_FILE_B = 0x0101'0101'0101'0101UL << 1, BB_FILE_C = 0x0101'0101'0101'0101UL << 2, BB_FILE_D = 0x0101'0101'0101'0101UL << 3, BB_FILE_E = 0x0101'0101'0101'0101UL << 4, BB_FILE_F = 0x0101'0101'0101'0101UL << 5, BB_FILE_G = 0x0101'0101'0101'0101UL << 6, BB_FILE_H = 0x0101'0101'0101'0101UL << 7;

        const Bitboard BB_RANKS[] = {
            0xffUL << (8 * 0),
            0xffUL << (8 * 1),
            0xffUL << (8 * 2),
            0xffUL << (8 * 3),
            0xffUL << (8 * 4),
            0xffUL << (8 * 5),
            0xffUL << (8 * 6),
            0xffUL << (8 * 7),
        }, BB_RANK_1 = 0xffUL << (8 * 0), BB_RANK_2 = 0xffUL << (8 * 1), BB_RANK_3 = 0xffUL << (8 * 2), BB_RANK_4 = 0xffUL << (8 * 3), BB_RANK_5 = 0xffUL << (8 * 4), BB_RANK_6 = 0xffUL << (8 * 5), BB_RANK_7 = 0xffUL << (8 * 6), BB_RANK_8 = 0xffUL << (8 * 7);

        const Bitboard BB_BACKRANKS = BB_RANK_1 | BB_RANK_8;


        int lsb(Bitboard bb) {
            return bitset<32>(bb & -bb).size() - 1;
        }

        vector<Square> scan_forward(Bitboard bb) {
            vector<Square> iter;
            while (bb) {
                Bitboard r = bb & -bb;
                iter.push_back(bitset<32>(r).count() - 1);
                bb ^= r;
            }
            return iter;
        }

        int msb(Bitboard bb) {
            return bitset<32>(bb).size() - 1;
        }

        vector<Square> scan_reversed(Bitboard bb) {
            vector<Square> iter;
            while (bb) {
                Square r = bitset<32>(bb).count() - 1;
                iter.push_back(r);
                bb ^= BB_SQUARES[r];
            }
            return iter;
        }

        function<int(Bitboard)> popcount = [](Bitboard bb) -> int {return bitset<32>(bb).count();};

        Bitboard flip_vertical(Bitboard bb) {
            // https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating#FlipVertically
            bb = ((bb >> 8) & 0x00ff'00ff'00ff'00ff) | ((bb & 0x00ff'00ff'00ff'00ff) << 8);
            bb = ((bb >> 16) & 0x0000'ffff'0000'ffff) | ((bb & 0x0000'ffff'0000'ffff) << 16);
            bb = (bb >> 32) | ((bb & 0x0000'0000'ffff'ffff) << 32);
            return bb;
        }

        Bitboard flip_horizontal(Bitboard bb) {
            // https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating#MirrorHorizontally
            bb = ((bb >> 1) & 0x5555'5555'5555'5555) | ((bb & 0x5555'5555'5555'5555) << 1);
            bb = ((bb >> 2) & 0x3333'3333'3333'3333) | ((bb & 0x3333'3333'3333'3333) << 2);
            bb = ((bb >> 4) & 0x0f0f'0f0f'0f0f'0f0f) | ((bb & 0x0f0f'0f0f'0f0f'0f0f) << 4);
            return bb;
        }

        Bitboard flip_diagonal(Bitboard bb) {
            // https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating#FlipabouttheDiagonal
            Bitboard t = (bb ^ (bb << 28)) & 0x0f0f'0f0f'0000'0000;
            bb = bb ^ (t ^ (t >> 28));
            t = (bb ^ (bb << 14)) & 0x3333'0000'3333'0000;
            bb = bb ^ (t ^ (t >> 14));
            t = (bb ^ (bb << 7)) & 0x5500'5500'5500'5500;
            bb = bb ^ (t ^ (t >> 7));
            return bb;
        }

        Bitboard flip_anti_diagonal(Bitboard bb) {
            // https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating#FlipabouttheAntidiagonal
            Bitboard t = bb ^ (bb << 36);
            bb = bb ^ ((t ^ (bb >> 36)) & 0xf0f0'f0f0'0f0f'0f0f);
            t = (bb ^ (bb << 18)) & 0xcccc'0000'cccc'0000;
            bb = bb ^ (t ^ (t >> 18));
            t = (bb ^ (bb << 9)) & 0xaa00'aa00'aa00'aa00;
            bb = bb ^ (t ^ (t >> 9));
            return bb;
        }

        Bitboard shift_down(Bitboard b) {
            return b >> 8;
        }

        Bitboard shift_2_down(Bitboard b) {
            return b >> 16;
        }

        Bitboard shift_up(Bitboard b) {
            return (b << 8) & BB_ALL;
        }

        Bitboard shift_2_up(Bitboard b) {
            return (b << 16) & BB_ALL;
        }

        Bitboard shift_right(Bitboard b) {
            return (b << 1) & ~BB_FILE_A & BB_ALL;
        }

        Bitboard shift_2_right(Bitboard b) {
            return (b << 2) & ~BB_FILE_A & ~BB_FILE_B & BB_ALL;
        }

        Bitboard shift_left(Bitboard b) {
            return (b >> 1) & ~BB_FILE_H;
        }

        Bitboard shift_2_left(Bitboard b) {
            return (b >> 2) & ~BB_FILE_G & ~BB_FILE_H;
        }

        Bitboard shift_up_left(Bitboard b) {
            return (b << 7) & ~BB_FILE_H & BB_ALL;
        }

        Bitboard shift_up_right(Bitboard b) {
            return (b << 9) & ~BB_FILE_A & BB_ALL;
        }

        Bitboard shift_down_left(Bitboard b) {
            return (b >> 9) & ~BB_FILE_H;
        }

        Bitboard shift_down_right(Bitboard b) {
            return (b >> 7) & ~BB_FILE_A;
        }


        Bitboard _sliding_attacks(Square square, Bitboard occupied, const vector<int> &deltas) {
            Bitboard attacks = BB_EMPTY;

            for (int delta : deltas) {
                Square sq = square;

                while (true) {
                    sq += delta;
                    if (!(0 <= sq && sq < 64) || square_distance(sq, sq - delta) > 2) {
                        break;
                    }

                    attacks |= BB_SQUARES[sq];

                    if (occupied & BB_SQUARES[sq]) {
                        break;
                    }
                }
            }

            return attacks;
        }

        Bitboard _step_attacks(Square square, const vector<int> &deltas) {
            return _sliding_attacks(square, BB_ALL, deltas);
        }

        const Bitboard BB_KNIGHT_ATTACKS[] = {_step_attacks(0, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(1, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(2, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(3, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(4, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(5, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(6, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(7, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(8, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(9, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(10, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(11, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(12, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(13, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(14, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(15, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(16, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(17, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(18, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(19, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(20, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(21, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(22, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(23, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(24, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(25, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(26, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(27, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(28, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(29, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(30, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(31, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(32, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(33, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(34, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(35, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(36, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(37, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(38, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(39, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(40, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(41, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(42, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(43, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(44, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(45, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(46, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(47, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(48, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(49, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(50, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(51, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(52, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(53, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(54, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(55, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(56, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(57, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(58, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(59, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(60, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(61, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(62, {17, 15, 10, 6, -17, -15, -10, -6}), _step_attacks(63, {17, 15, 10, 6, -17, -15, -10, -6})};
        const Bitboard BB_KING_ATTACKS[] = {_step_attacks(0, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(1, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(2, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(3, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(4, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(5, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(6, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(7, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(8, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(9, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(10, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(11, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(12, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(13, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(14, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(15, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(16, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(17, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(18, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(19, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(20, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(21, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(22, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(23, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(24, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(25, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(26, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(27, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(28, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(29, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(30, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(31, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(32, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(33, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(34, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(35, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(36, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(37, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(38, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(39, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(40, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(41, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(42, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(43, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(44, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(45, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(46, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(47, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(48, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(49, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(50, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(51, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(52, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(53, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(54, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(55, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(56, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(57, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(58, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(59, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(60, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(61, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(62, {9, 8, 7, 1, -9, -8, -7, -1}), _step_attacks(63, {9, 8, 7, 1, -9, -8, -7, -1})};
        const Bitboard BB_PAWN_ATTACKS[][64] = {{_step_attacks(0, {-7, -9}), _step_attacks(1, {-7, -9}), _step_attacks(2, {-7, -9}), _step_attacks(3, {-7, -9}), _step_attacks(4, {-7, -9}), _step_attacks(5, {-7, -9}), _step_attacks(6, {-7, -9}), _step_attacks(7, {-7, -9}), _step_attacks(8, {-7, -9}), _step_attacks(9, {-7, -9}), _step_attacks(10, {-7, -9}), _step_attacks(11, {-7, -9}), _step_attacks(12, {-7, -9}), _step_attacks(13, {-7, -9}), _step_attacks(14, {-7, -9}), _step_attacks(15, {-7, -9}), _step_attacks(16, {-7, -9}), _step_attacks(17, {-7, -9}), _step_attacks(18, {-7, -9}), _step_attacks(19, {-7, -9}), _step_attacks(20, {-7, -9}), _step_attacks(21, {-7, -9}), _step_attacks(22, {-7, -9}), _step_attacks(23, {-7, -9}), _step_attacks(24, {-7, -9}), _step_attacks(25, {-7, -9}), _step_attacks(26, {-7, -9}), _step_attacks(27, {-7, -9}), _step_attacks(28, {-7, -9}), _step_attacks(29, {-7, -9}), _step_attacks(30, {-7, -9}), _step_attacks(31, {-7, -9}), _step_attacks(32, {-7, -9}), _step_attacks(33, {-7, -9}), _step_attacks(34, {-7, -9}), _step_attacks(35, {-7, -9}), _step_attacks(36, {-7, -9}), _step_attacks(37, {-7, -9}), _step_attacks(38, {-7, -9}), _step_attacks(39, {-7, -9}), _step_attacks(40, {-7, -9}), _step_attacks(41, {-7, -9}), _step_attacks(42, {-7, -9}), _step_attacks(43, {-7, -9}), _step_attacks(44, {-7, -9}), _step_attacks(45, {-7, -9}), _step_attacks(46, {-7, -9}), _step_attacks(47, {-7, -9}), _step_attacks(48, {-7, -9}), _step_attacks(49, {-7, -9}), _step_attacks(50, {-7, -9}), _step_attacks(51, {-7, -9}), _step_attacks(52, {-7, -9}), _step_attacks(53, {-7, -9}), _step_attacks(54, {-7, -9}), _step_attacks(55, {-7, -9}), _step_attacks(56, {-7, -9}), _step_attacks(57, {-7, -9}), _step_attacks(58, {-7, -9}), _step_attacks(59, {-7, -9}), _step_attacks(60, {-7, -9}), _step_attacks(61, {-7, -9}), _step_attacks(62, {-7, -9}), _step_attacks(63, {-7, -9})}, {_step_attacks(0, {7, 9}), _step_attacks(1, {7, 9}), _step_attacks(2, {7, 9}), _step_attacks(3, {7, 9}), _step_attacks(4, {7, 9}), _step_attacks(5, {7, 9}), _step_attacks(6, {7, 9}), _step_attacks(7, {7, 9}), _step_attacks(8, {7, 9}), _step_attacks(9, {7, 9}), _step_attacks(10, {7, 9}), _step_attacks(11, {7, 9}), _step_attacks(12, {7, 9}), _step_attacks(13, {7, 9}), _step_attacks(14, {7, 9}), _step_attacks(15, {7, 9}), _step_attacks(16, {7, 9}), _step_attacks(17, {7, 9}), _step_attacks(18, {7, 9}), _step_attacks(19, {7, 9}), _step_attacks(20, {7, 9}), _step_attacks(21, {7, 9}), _step_attacks(22, {7, 9}), _step_attacks(23, {7, 9}), _step_attacks(24, {7, 9}), _step_attacks(25, {7, 9}), _step_attacks(26, {7, 9}), _step_attacks(27, {7, 9}), _step_attacks(28, {7, 9}), _step_attacks(29, {7, 9}), _step_attacks(30, {7, 9}), _step_attacks(31, {7, 9}), _step_attacks(32, {7, 9}), _step_attacks(33, {7, 9}), _step_attacks(34, {7, 9}), _step_attacks(35, {7, 9}), _step_attacks(36, {7, 9}), _step_attacks(37, {7, 9}), _step_attacks(38, {7, 9}), _step_attacks(39, {7, 9}), _step_attacks(40, {7, 9}), _step_attacks(41, {7, 9}), _step_attacks(42, {7, 9}), _step_attacks(43, {7, 9}), _step_attacks(44, {7, 9}), _step_attacks(45, {7, 9}), _step_attacks(46, {7, 9}), _step_attacks(47, {7, 9}), _step_attacks(48, {7, 9}), _step_attacks(49, {7, 9}), _step_attacks(50, {7, 9}), _step_attacks(51, {7, 9}), _step_attacks(52, {7, 9}), _step_attacks(53, {7, 9}), _step_attacks(54, {7, 9}), _step_attacks(55, {7, 9}), _step_attacks(56, {7, 9}), _step_attacks(57, {7, 9}), _step_attacks(58, {7, 9}), _step_attacks(59, {7, 9}), _step_attacks(60, {7, 9}), _step_attacks(61, {7, 9}), _step_attacks(62, {7, 9}), _step_attacks(63, {7, 9})}};


        Bitboard _edges(Square square) {
            return (((BB_RANK_1 | BB_RANK_8) & ~BB_RANKS[square_rank(square)]) |
                    ((BB_FILE_A | BB_FILE_H) & ~BB_FILES[square_file(square)]));
        }

        vector<Bitboard> _carry_rippler(Bitboard mask) {
            // Carry-Rippler trick to iterate subsets of mask.
            vector<Bitboard> iter;
            Bitboard subset = BB_EMPTY;
            while (true) {
                iter.push_back(subset);
                subset = (subset - mask) & mask;
                if (!subset) {
                    break;
                }
            }
            return iter;
        }

        tuple<vector<Bitboard>, vector<unordered_map<Bitboard, Bitboard>>> _attack_table(const vector<int> &deltas) {
            vector<Bitboard> mask_table;
            vector<unordered_map<Bitboard, Bitboard>> attack_table;

            for (Square square : SQUARES) {
                unordered_map<Bitboard, Bitboard> attacks;

                Bitboard mask = _sliding_attacks(square, 0, deltas) & ~_edges(square);
                for (Bitboard subset : _carry_rippler(mask)) {
                    attacks[subset] = _sliding_attacks(square, subset, deltas);
                }

                attack_table.push_back(attacks);
                mask_table.push_back(mask);
            }

            return {mask_table, attack_table};
        }

        const auto [BB_DIAG_MASKS, BB_DIAG_ATTACKS] = _attack_table({-9, -7, 7, 9});
        const auto [BB_FILE_MASKS, BB_FILE_ATTACKS] = _attack_table({-8, 8});
        const auto [BB_RANK_MASKS, BB_RANK_ATTACKS] = _attack_table({-1, 1});


        vector<vector<Bitboard>> _rays() {
            vector<vector<Bitboard>> rays;
            for (int a = 0; a < 64; ++a) {
                Bitboard bb_a = BB_SQUARES[a];
                vector<Bitboard> rays_row;
                for (int b = 0; b < 64; ++b) {
                    Bitboard bb_b = BB_SQUARES[b];
                    if (BB_DIAG_ATTACKS[a].at(0) & bb_b) {
                        rays_row.push_back((BB_DIAG_ATTACKS[a].at(0) & BB_DIAG_ATTACKS[b].at(0)) | bb_a | bb_b);
                    } else if (BB_RANK_ATTACKS[a].at(0) & bb_b) {
                        rays_row.push_back(BB_RANK_ATTACKS[a].at(0) | bb_a);
                    } else if (BB_FILE_ATTACKS[a].at(0) & bb_b) {
                        rays_row.push_back(BB_FILE_ATTACKS[a].at(0) | bb_a);
                    } else {
                        rays_row.push_back(BB_EMPTY);
                    }
                }
                rays.push_back(rays_row);
            }
            return rays;
        }

        const vector<vector<Bitboard>> BB_RAYS = _rays();

        Bitboard ray(Square a, Square b) {
            return BB_RAYS[a][b];
        }

        Bitboard between(Square a, Square b) {
            Bitboard bb = BB_RAYS[a][b] & ((BB_ALL << a) ^ (BB_ALL << b));
            return bb & (bb - 1);
        }


        const regex SAN_REGEX(R"(^([NBKRQ])?([a-h])?([1-8])?[\-x]?([a-h][1-8])(=?[nbrqkNBRQK])?[\+#]?$)");

        const regex FEN_CASTLING_REGEX(R"(^(?:-|[KQABCDEFGH]{0,2}[kqabcdefgh]{0,2})$)");


        class Piece {
            /* A piece with type and color. */

        public:
            PieceType piece_type;
            /* The piece type. */

            Color color;
            /* The piece color. */

            Piece(PieceType piece_type, Color color) : piece_type(piece_type), color(color) {}

            char symbol() const {
                /*
                Gets the symbol ``P``, ``N``, ``B``, ``R``, ``Q`` or ``K`` for white
                pieces or the lower-case variants for the black pieces.
                */
                char symbol = piece_symbol(this->piece_type);
                return this->color ? toupper(symbol) : symbol;
            }


            string unicode_symbol(bool invert_color = false) const {
                /*
                Gets the Unicode character for the piece.
                */
                char symbol = invert_color ? isupper(this->symbol()) ? tolower(this->symbol()) : toupper(this->symbol()) : this->symbol();
                return UNICODE_PIECE_SYMBOLS.at(symbol);
            }


            operator string() const {
                return to_string(this->symbol());
            }

            static Piece from_symbol(char symbol) {
                /*
                Creates a :class:`~chess::Piece` instance from a piece symbol.

                :throws: :exc:`std::invalid_argument` if the symbol is invalid.
                */
                auto it = find(begin(PIECE_SYMBOLS), end(PIECE_SYMBOLS), tolower(symbol));
                if (it == end(PIECE_SYMBOLS)) {
                    throw invalid_argument("symbol is invalid");
                }
                return Piece(distance(PIECE_SYMBOLS, it), toupper(symbol));
            }
        };

        ostream &operator<<(ostream &os, const Piece &piece) {
            os << "Piece::from_symbol('" << piece.symbol() << "')";
            return os;
        }



        class Move {
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

            optional<PieceType> promotion;
            /* The promotion piece type or ``std::nullopt``. */

            optional<PieceType> drop;
            /* The drop piece type or ``std::nullopt``. */

            Move(Square from_square, Square to_square, optional<PieceType> promotion = nullopt, optional<PieceType> drop = nullopt) : from_square(from_square), to_square(to_square), promotion(promotion), drop(drop) {}

            string uci() const {
                /*
                Gets a UCI string for the move.

                For example, a move from a7 to a8 would be ``a7a8`` or ``a7a8q``
                (if the latter is a promotion to a queen).

                The UCI representation of a null move is ``0000``.
                */
                if (this->drop) {
                    return toupper(piece_symbol(*this->drop)) + "@" + SQUARE_NAMES[this->to_square];
                } else if (this->promotion) {
                    return SQUARE_NAMES[this->from_square] + SQUARE_NAMES[this->to_square] + piece_symbol(*this->promotion);
                } else if (*this) {
                    return SQUARE_NAMES[this->from_square] + SQUARE_NAMES[this->to_square];
                } else {
                    return "0000";
                }
            }


            string xboard() const {
                return *this ? this->uci() : "@@@@";
            }

            operator bool() const {
                return bool(this->from_square || this->to_square || this->promotion || this->drop);
            }

            operator string() const {
                return this->uci();
            }

            static Move from_uci(const string &uci) {
                /*
                Parses a UCI string.

                :throws: :exc:`std::invalid_argument` if the UCI string is invalid.
                */
                if (uci == "0000") {
                    return Move::null();
                } else if (uci.length() == 4 && '@' == uci[1]) {
                    auto it = find(begin(PIECE_SYMBOLS), end(PIECE_SYMBOLS), tolower(uci[0]));
                    if (it == end(PIECE_SYMBOLS)) {
                        throw invalid_argument("");
                    }
                    Square drop = distance(PIECE_SYMBOLS, it);
                    auto it2 = find(begin(SQUARE_NAMES), end(SQUARE_NAMES), uci.substr(2));
                    if (it2 == end(SQUARE_NAMES)) {
                        throw invalid_argument("");
                    }
                    Square square = distance(SQUARE_NAMES, it2);
                    return Move(square, square, drop);
                } else if (4 <= uci.length() && uci.length() <= 5) {
                    auto it = find(begin(SQUARE_NAMES), end(SQUARE_NAMES), uci.substr(0, 2));
                    if (it == end(SQUARE_NAMES)) {
                        throw invalid_argument("");
                    }
                    Square from_square = distance(SQUARE_NAMES, it);
                    auto it2 = find(begin(SQUARE_NAMES), end(SQUARE_NAMES), uci.substr(2, 4));
                    if (it2 == end(SQUARE_NAMES)) {
                        throw invalid_argument("");
                    }
                    Square to_square = distance(SQUARE_NAMES, it2);
                    optional<Square> promotion;
                    if (uci.length() == 5) {
                        auto it3 = find(begin(PIECE_SYMBOLS), end(PIECE_SYMBOLS), uci[4]);
                        if (it3 == end(PIECE_SYMBOLS)) {
                            throw invalid_argument("");
                        }
                        promotion = distance(PIECE_SYMBOLS, it3);
                    } else {
                        promotion = nullopt;
                    }
                    if (from_square == to_square) {
                        throw invalid_argument("invalid uci (use 0000 for null moves): '" + uci + "'");
                    }
                    return Move(from_square, to_square, promotion);
                } else {
                    throw invalid_argument("expected uci string to be of length 4 or 5: '" + uci + "'");
                }
            }


            static Move null() {
                /*
                Gets a null move.

                A null move just passes the turn to the other side (and possibly
                forfeits en passant capturing). Null moves evaluate to ``false`` in
                boolean contexts.

                >>> #include "chess.cpp"
                >>> #include <iostream>
                >>>
                >>> std::cout << bool(chess::Move::null());
                0
                */
                return Move(0, 0);
            }
        };

        ostream &operator<<(ostream &os, const Move &move) {
            os << "Move::from_uci('" << move.uci() << "')";
            return os;
        }



        typedef BaseBoard BaseBoardT;

        class BaseBoard {
            /*
            A board representing the position of chess pieces. See
            :class:`~chess::Board` for a full board with move generation.

            The board is initialized with the standard chess starting position, unless
            otherwise specified in the optional *board_fen* argument. If *board_fen*
            is ``std::nullopt``, an empty board is created.
            */

        public:
            Bitboard occupied_co[2], pawns, knights, bishops, rooks, queens, kings, promoted, occupied;

            BaseBoard(optional<string> board_fen = STARTING_BOARD_FEN) : occupied_co{BB_EMPTY, BB_EMPTY} {
                if (board_fen == nullopt) {
                    this->_clear_board();
                } else if (*board_fen == STARTING_BOARD_FEN) {
                    this->_reset_board();
                } else {
                    this->_set_board_fen(*board_fen);
                }
            }

            void reset_board() const {
                /* Resets pieces to the starting position. */
                this->_reset_board();
            }

            void clear_board() const {
                /* Clears the board. */
                this->_clear_board();
            }


            Bitboard pieces_mask(PieceType piece_type, Color color) const {
                Bitboard bb;
                if (piece_type == PAWN) {
                    bb = this->pawns;
                } else if (piece_type == KNIGHT) {
                    bb = this->knights;
                } else if (piece_type == BISHOP) {
                    bb = this->bishops;
                } else if (piece_type == ROOK) {
                    bb = this->rooks;
                } else if (piece_type == QUEEN) {
                    bb = this->queens;
                } else if (piece_type == KING) {
                    bb = this->kings;
                } else {
                    throw "expected PieceType, got '" + to_string(piece_type) + "'";
                }

                return bb & this->occupied_co[color];
            }

            SquareSet pieces(PieceType piece_type, Color color) const {
                /*
                Gets pieces of the given type and color.

                Returns a :class:`set of squares <chess::SquareSet>`.
                */
                return SquareSet(this->pieces_mask(piece_type, color));
            }

            optional<Piece> piece_at(Square square) const {
                /* Gets the :class:`piece <chess::Piece>` at the given square. */
                optional<PieceType> piece_type = this->piece_type_at(square);
                if (piece_type) {
                    Bitboard mask = BB_SQUARES[square];
                    Color color = bool(this->occupied_co[WHITE] & mask);
                    return Piece(*piece_type, color);
                } else {
                    return nullopt;
                }
            }


            optional<PieceType> piece_type_at(Square square) const {
                /* Gets the piece type at the given square. */
                Bitboard mask = BB_SQUARES[square];

                if (!(this->occupied & mask)) {
                    return nullopt; // Early return
                } else if (this->pawns & mask) {
                    return PAWN;
                } else if (this->knights & mask) {
                    return KNIGHT;
                } else if (this->bishops & mask) {
                    return BISHOP;
                } else if (this->rooks & mask) {
                    return ROOK;
                } else if (this->queens & mask) {
                    return QUEEN;
                } else {
                    return KING;
                }
            }


            optional<Color> color_at(Square square) const {
                /* Gets the color of the piece at the given square. */
                Bitboard mask = BB_SQUARES[square];
                if (this->occupied_co[WHITE] & mask) {
                    return WHITE;
                } else if (this->occupied_co[BLACK] & mask) {
                    return BLACK;
                } else {
                    return nullopt;
                }
            }


            optional<Square> king(Color color) const {
                /*
                Finds the king square of the given side. Returns ``std::nullopt`` if there
                is no king of that color.

                In variants with king promotions, only non-promoted kings are
                considered.
                */
                Bitboard king_mask = this->occupied_co[color] & this->kings & ~this->promoted;
                return king_mask ? optional(msb(king_mask)) : nullopt;
            }


            Bitboard attacks_mask(Square square) const {
                Bitboard bb_square = BB_SQUARES[square];

                if (bb_square & this->pawns) {
                    Color color = bool(bb_square & this->occupied_co[WHITE]);
                    return BB_PAWN_ATTACKS[color][square];
                } else if (bb_square & this->knights) {
                    return BB_KNIGHT_ATTACKS[square];
                } else if (bb_square & this->kings) {
                    return BB_KING_ATTACKS[square];
                } else {
                    Bitboard attacks = 0;
                    if (bb_square & this->bishops || bb_square & this->queens) {
                        attacks = BB_DIAG_ATTACKS[square].at(BB_DIAG_MASKS[square] & this->occupied);
                    }
                    if (bb_square & this->rooks || bb_square & this->queens) {
                        attacks |= (BB_RANK_ATTACKS[square].at(BB_RANK_MASKS[square] & this->occupied) |
                                    BB_FILE_ATTACKS[square].at(BB_FILE_MASKS[square] & this->occupied));
                    }
                    return attacks;
                }
            }

            SquareSet attacks(Square square) const {
                /*
                Gets the set of attacked squares from the given square.

                There will be no attacks if the square is empty. Pinned pieces are
                still attacking other squares.

                Returns a :class:`set of squares <chess::SquareSet>`.
                */
                return SquareSet(this->attacks_mask(square));
            }


            Bitboard attackers_mask(Color color, Square square) const {
                return this->_attackers_mask(color, square, this->occupied);
            }

            bool is_attacked_by(Color color, Square square) const {
                /*
                Checks if the given side attacks the given square.

                Pinned pieces still count as attackers. Pawns that can be captured
                en passant are **not** considered attacked.
                */
                return bool(this->attackers_mask(color, square));
            }


            SquareSet attackers(Color color, Square square) const {
                /*
                Gets the set of attackers of the given color for the given square.

                Pinned pieces still count as attackers.

                Returns a :class:`set of squares <chess::SquareSet>`.
                */
                return SquareSet(this->attackers_mask(color, square));
            }


            Bitboard pin_mask(Color color, Square square) const {
                optional<Square> king = this->king(color);
                if (king == nullopt)
                    return BB_ALL;

                Bitboard square_mask = BB_SQUARES[square];

                for (auto [attacks, sliders] : {make_tuple(BB_FILE_ATTACKS, this->rooks | this->queens),
                                                make_tuple(BB_RANK_ATTACKS, this->rooks | this->queens),
                                                make_tuple(BB_DIAG_ATTACKS, this->bishops | this->queens)}) {
                    Bitboard rays = attacks[*king].at(0);
                    if (rays & square_mask) {
                        Bitboard snipers = rays & sliders & this->occupied_co[!color];
                        for (Square sniper : scan_reversed(snipers)) {
                            if ((between(sniper, *king) & (this->occupied | square_mask)) == square_mask) {
                                return ray(*king, sniper);
                            }
                        }

                        break;
                    }
                }

                return BB_ALL;
            }

            SquareSet pin(Color color, Square square) const {
                /*
                Detects an absolute pin (and its direction) of the given square to
                the king of the given color.

                >>> #include "chess.cpp"
                >>> #include <iostream>
                >>>
                >>> chess::Board board = chess::Board("rnb1k2r/ppp2ppp/5n2/3q4/1b1P4/2N5/PP3PPP/R1BQKBNR w KQkq - 3 7");
                >>> std::cout << board.is_pinned(chess::WHITE, chess::C3);
                1
                >>> chess::SquareSet direction = board.pin(chess::WHITE, chess::C3);
                >>> std::cout << direction;
                SquareSet(0x0000'0001'0204'0810)
                >>> std::cout << string(direction);
                . . . . . . . .
                . . . . . . . .
                . . . . . . . .
                1 . . . . . . .
                . 1 . . . . . .
                . . 1 . . . . .
                . . . 1 . . . .
                . . . . 1 . . .

                Returns a :class:`set of squares <chess::SquareSet>` that mask the rank,
                file or diagonal of the pin. If there is no pin, then a mask of the
                entire board is returned.
                */
                return SquareSet(this->pin_mask(color, square));
            }


            bool is_pinned(Color color, Square square) const {
                /*
                Detects if the given square is pinned to the king of the given color.
                */
                return this->pin_mask(color, square) != BB_ALL;
            }


            optional<Piece> remove_piece_at(Square square) const {
                /*
                Removes the piece from the given square. Returns the
                :class:`~chess::Piece` or ``std::nullopt`` if the square was already empty.
                */
                Color color = bool(this->occupied_co[WHITE] & BB_SQUARES[square]);
                optional<PieceType> piece_type = this->_remove_piece_at(square);
                return piece_type ? optional(Piece(*piece_type, color)) : nullopt;
            }


            void set_piece_at(Square square, optional<Piece> piece, bool promoted = false) const {
                /*
                Sets a piece at the given square.

                An existing piece is replaced. Setting *piece* to ``std::nullopt`` is
                equivalent to :func:`~chess::Board::remove_piece_at()`.
                */
                if (piece == nullopt) {
                    this->_remove_piece_at(square);
                } else {
                    this->_set_piece_at(square, piece->piece_type, piece->color, promoted);
                }
            }


            string board_fen(optional<bool> promoted = false) const {
                /*
                Gets the board FEN (e.g.,
                ``rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR``).
                */
                vector<char> builder;
                int empty = 0;

                for (Square square : SQUARES_180) {
                    optional<Piece> piece = this->piece_at(square);

                    if (!piece) {
                        ++empty;
                    } else {
                        if (empty) {
                            builder.push_back(to_string(empty)[0]);
                            empty = 0;
                        }
                        builder.push_back(piece->symbol());
                        if (promoted && BB_SQUARES[square] & this->promoted) {
                            builder.push_back('~');
                        }
                    }

                    if (BB_SQUARES[square] & BB_FILE_H) {
                        if (empty) {
                            builder.push_back(to_string(empty)[0]);
                            empty = 0;
                        }

                        if (square != H1) {
                            builder.push_back('/');
                        }
                    }
                }

                return string(begin(builder), end(builder));
            }


            void set_board_fen(const string &fen) const {
                /*
                Parses *fen* and sets up the board, where *fen* is the board part of
                a FEN.

                :throws: :exc:`std::invalid_argument` if syntactically invalid.
                */
                this->_set_board_fen(fen);
            }


            unordered_map<Square, Piece> piece_map(Bitboard mask = BB_ALL) const {
                /*
                Gets a map of :class:`pieces <chess::Piece>` by square index.
                */
                unordered_map<Square, Piece> result;
                for (Square square : scan_reversed(this->occupied & mask)) {
                    result[square] = *this->piece_at(square);
                }
                return result;
            }


            void set_piece_map(const unordered_map<Square, Piece> &pieces) const {
                /*
                Sets up the board from a map of :class:`pieces <chess::Piece>`
                by square index.
                */
                this->_set_piece_map(pieces);
            }


            void set_chess960_pos(int scharnagl) const {
                /*
                Sets up a Chess960 starting position given its index between 0 and 959.
                Also see :func:`~chess::BaseBoard::from_chess960_pos()`.
                */
                this->_set_chess960_pos(scharnagl);
            }


            optional<int> chess960_pos() const {
                /*
                Gets the Chess960 starting position index between 0 and 959,
                or ``std::nullopt``.
                */
                if (this->occupied_co[WHITE] != (BB_RANK_1 | BB_RANK_2)) {
                    return nullopt;
                }
                if (this->occupied_co[BLACK] != (BB_RANK_7 | BB_RANK_8)) {
                    return nullopt;
                }
                if (this->pawns != (BB_RANK_2 | BB_RANK_7)) {
                    return nullopt;
                }
                if (this->promoted) {
                    return nullopt;
                }

                // Piece counts.
                vector<Bitboard> brnqk = {this->bishops, this->rooks, this->knights, this->queens, this->kings};
                if (popcount(this->bishops) != 4 || popcount(this->rooks) != 4 || popcount(this->knights) != 4 || popcount(this->queens) != 2 || popcount(this->kings) != 2) {
                    return nullopt;
                }

                // Symmetry.
                if (((BB_RANK_1 & this->bishops) << 56 != (BB_RANK_8 & this->bishops)) || ((BB_RANK_1 & this->rooks) << 56 != (BB_RANK_8 & this->rooks)) || ((BB_RANK_1 & this->knights) << 56 != (BB_RANK_8 & this->knights)) || ((BB_RANK_1 & this->queens) << 56 != (BB_RANK_8 & this->queens)) || ((BB_RANK_1 & this->kings) << 56 != (BB_RANK_8 & this->kings))) {
                    return nullopt;
                }

                // Algorithm from ChessX
                Bitboard x = this->bishops & (2 + 8 + 32 + 128);
                if (!x) {
                    return nullopt;
                }
                int bs1 = (lsb(x) - 1) / 2;
                int cc_pos = bs1;
                x = this->bishops & (1 + 4 + 16 + 64);
                if (!x) {
                    return nullopt;
                }
                int bs2 = lsb(x) * 2;
                cc_pos += bs2;

                int q = 0;
                bool qf = false;
                int n0 = 0;
                int n1 = 0;
                bool n0f = false;
                bool n1f = false;
                int rf = 0;
                vector<int> n0s = {0, 4, 7, 9};
                for (Square square = A1; square <= H1; ++square) {
                    Bitboard bb = BB_SQUARES[square];
                    if (bb & this->queens) {
                        qf = true;
                    } else if (bb & this->rooks || bb & this->kings) {
                        if (bb & this->kings) {
                            if (rf != 1) {
                                return nullopt;
                            }
                        } else {
                            ++rf;
                        }

                        if (!qf) {
                            ++q;
                        }

                        if (!n0f) {
                            ++n0;
                        } else if (!n1f) {
                            ++n1;
                        }
                    } else if (bb & this->knights) {
                        if (!qf) {
                            ++q;
                        }

                        if (!n0f) {
                            n0f = true;
                        } else if (!n1f) {
                            n1f = true;
                        }
                    }
                }

                if (n0 < 4 && n1f && qf) {
                    cc_pos += q * 16;
                    int krn = n0s[n0] + n1;
                    cc_pos += krn * 96;
                    return cc_pos;
                } else
                    return nullopt;
            }


            operator string() const {
                vector<char> builder;

                for (Square square : SQUARES_180) {
                    optional<Piece> piece = this->piece_at(square);

                    if (piece) {
                        builder.push_back(piece->symbol());
                    } else {
                        builder.push_back('.');
                    }

                    if (BB_SQUARES[square] & BB_FILE_H) {
                        if (square != H1) {
                            builder.push_back('\n');
                        }
                    } else {
                        builder.push_back(' ');
                    }
                }

                return string(begin(builder), end(builder));
            }

            string unicode(bool invert_color = false, bool borders = false, const string &empty_square = "⭘") const {
                /*
                Returns a string representation of the board with Unicode pieces.
                Useful for pretty-printing to a terminal.

                :param invert_color: Invert color of the Unicode pieces.
                :param borders: Show borders and a coordinate margin.
                */
                vector<char> builder;
                for (int rank_index = 7; rank_index >= 0; --rank_index) {
                    if (borders) {
                        builder.insert(end(builder), 2, ' ');
                        builder.insert(end(builder), 17, '-');
                        builder.push_back('\n');

                        builder.push_back(RANK_NAMES[rank_index]);
                        builder.push_back(' ');
                    }

                    for (int file_index = 0; file_index < 8; ++file_index) {
                        Square square_index = square(file_index, rank_index);

                        if (borders) {
                            builder.push_back('|');
                        } else if (file_index > 0) {
                            builder.push_back(' ');
                        }

                        optional<Piece> piece = this->piece_at(square_index);

                        if (piece) {
                            string unicode_symbol = piece->unicode_symbol(invert_color);
                            builder.insert(end(builder), begin(unicode_symbol), end(unicode_symbol));
                        } else {
                            builder.insert(end(builder), begin(empty_square), end(empty_square));
                        }
                    }

                    if (borders) {
                        builder.push_back('|');
                    }

                    if (borders || rank_index > 0) {
                        builder.push_back('\n');
                    }
                }

                if (borders) {
                    builder.insert(end(builder), 2, ' ');
                    builder.insert(end(builder), 17, '-');
                    builder.push_back('\n');
                    builder.insert(end(builder), begin("   a b c d e f g h"), end("   a b c d e f g h"));
                }

                return string(begin(builder), end(builder));
            }


            bool operator==(const BaseBoard &board) const {
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

            void apply_transform(function<Bitboard(Bitboard)> f) const {
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

            BaseBoardT transform(function<Bitboard(Bitboard)> f) const {
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
                BaseBoardT board = this->copy();
                board.apply_transform(f);
                return board;
            }


            void apply_mirror() const {
                this->apply_transform(flip_vertical);
                swap(this->occupied_co[WHITE], this->occupied_co[BLACK]);
            }

            BaseBoardT mirror() const {
                /*
                Returns a mirrored copy of the board.

                The board is mirrored vertically and piece colors are swapped, so that
                the position is equivalent modulo color.

                Alternatively, :func:`~chess::BaseBoard::apply_mirror()` can be used
                to mirror the board.
                */
                BaseBoardT board = this->copy();
                board.apply_mirror();
                return board;
            }


            BaseBoardT copy() const {
                /* Creates a copy of the board. */
                BaseBoard board = BaseBoard(nullopt);

                board.pawns = this->pawns;
                board.knights = this->knights;
                board.bishops = this->bishops;
                board.rooks = this->rooks;
                board.queens = this->queens;
                board.kings = this->kings;

                board.occupied_co[WHITE] = this->occupied_co[WHITE];
                board.occupied_co[BLACK] = this->occupied_co[BLACK];
                board.occupied = this->occupied;
                board.promoted = this->promoted;

                return board;
            }


            static BaseBoardT empty() {
                /*
                Creates a new empty board. Also see
                :func:`~chess::BaseBoard::clear_board()`.
                */
                return BaseBoardT(nullopt);
            }


            static BaseBoardT from_chess960_pos(int scharnagl) {
                /*
                Creates a new board, initialized with a Chess960 starting position.

                >>> #include "chess.cpp"
                >>> #include <stdlib.h>
                >>> #include <time.h>
                >>>
                >>> srand(time(0));
                >>> chess::Board board = chess::Board::from_chess960_pos(rand() % 960);
                */
                BaseBoardT board = BaseBoard::empty();
                board.set_chess960_pos(scharnagl);
                return board;
            }

        protected:
            void _reset_board() const {
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


            void _clear_board() const {
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


            Bitboard _attackers_mask(Color color, Square square, Bitboard occupied) const {
                Bitboard rank_pieces = BB_RANK_MASKS[square] & occupied;
                Bitboard file_pieces = BB_FILE_MASKS[square] & occupied;
                Bitboard diag_pieces = BB_DIAG_MASKS[square] & occupied;

                Bitboard queens_and_rooks = this->queens | this->rooks;
                Bitboard queens_and_bishops = this->queens | this->bishops;

                Bitboard attackers = (
                    (BB_KING_ATTACKS[square] & this->kings) |
                    (BB_KNIGHT_ATTACKS[square] & this->knights) |
                    (BB_RANK_ATTACKS[square].at(rank_pieces) & queens_and_rooks) |
                    (BB_FILE_ATTACKS[square].at(file_pieces) & queens_and_rooks) |
                    (BB_DIAG_ATTACKS[square].at(diag_pieces) & queens_and_bishops) |
                    (BB_PAWN_ATTACKS[!color][square] & this->pawns));

                return attackers & this->occupied_co[color];
            }

            optional<PieceType> _remove_piece_at(Square square) const {
                optional<PieceType> piece_type = this->piece_type_at(square);
                Bitboard mask = BB_SQUARES[square];

                if (*piece_type == PAWN) {
                    this->pawns ^= mask;
                } else if (*piece_type == KNIGHT) {
                    this->knights ^= mask;
                } else if (*piece_type == BISHOP) {
                    this->bishops ^= mask;
                } else if (*piece_type == ROOK) {
                    this->rooks ^= mask;
                } else if (*piece_type == QUEEN) {
                    this->queens ^= mask;
                } else if (*piece_type == KING) {
                    this->kings ^= mask;
                } else {
                    return nullopt;
                }

                this->occupied ^= mask;
                this->occupied_co[WHITE] &= ~mask;
                this->occupied_co[BLACK] &= ~mask;

                this->promoted &= ~mask;

                return piece_type;
            }

            void _set_piece_at(Square square, PieceType piece_type, Color color, bool promoted = false) const {
                this->_remove_piece_at(square);

                Bitboard mask = BB_SQUARES[square];

                if (piece_type == PAWN) {
                    this->pawns |= mask;
                } else if (piece_type == KNIGHT) {
                    this->knights |= mask;
                } else if (piece_type == BISHOP) {
                    this->bishops |= mask;
                } else if (piece_type == ROOK) {
                    this->rooks |= mask;
                } else if (piece_type == QUEEN) {
                    this->queens |= mask;
                } else if (piece_type == KING) {
                    this->kings |= mask;
                } else {
                    return;
                }

                this->occupied ^= mask;
                this->occupied_co[color] ^= mask;

                if (promoted) {
                    this->promoted ^= mask;
                }
            }

            void _set_board_fen(const string &fen) const {
                // Compatibility with set_fen().
                auto it = begin(fen);
                auto it2 = rbegin(fen);
                while (isspace(*it)) {
                    ++it;
                }
                while (isspace(*it2)) {
                    ++it2;
                }
                fen = string(it, it2.base());
                if (fen.find(' ') != string::npos) {
                    throw invalid_argument("expected position part of fen, got multiple parts: '" + fen + "'");
                }

                // Ensure the FEN is valid.
                vector<string> rows;
                for (size_t i = 0, dist = 0; i < fen.length(); ++i, ++dist) {
                    if (fen[i] == '/') {
                        rows.push_back(fen.substr(i++ - dist, dist));
                        dist = 0;
                    } else if (i == fen.length() - 1) {
                        rows.push_back(fen.substr(i - dist));
                    }
                }
                if (rows.size() != 8) {
                    throw invalid_argument("expected 8 rows in position part of fen: '" + fen + "'");
                }

                // Validate each row.
                for (const string &row : rows) {
                    int field_sum = 0;
                    bool previous_was_digit = false;
                    bool previous_was_piece = false;

                    for (char c : row) {
                        if (c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8') {
                            if (previous_was_digit) {
                                throw invalid_argument("two subsequent digits in position part of fen: '" + fen + "'");
                            }
                            field_sum += int(c - '0');
                            previous_was_digit = true;
                            previous_was_piece = false;
                        } else if (c == '~') {
                            if (!previous_was_piece) {
                                throw invalid_argument("'~' not after piece in position part of fen: '" + fen + "'");
                            }
                            previous_was_digit = false;
                            previous_was_piece = false;
                        } else if (find(begin(PIECE_SYMBOLS), end(PIECE_SYMBOLS), tolower(c)) != end(PIECE_SYMBOLS)) {
                            ++field_sum;
                            previous_was_digit = false;
                            previous_was_piece = true;
                        } else {
                            throw invalid_argument("invalid character in position part of fen: '" + fen + "'");
                        }
                    }

                    if (field_sum != 8) {
                        throw invalid_argument("expected 8 columns per row in position part of fen: '" + fen + "'");
                    }
                }

                // Clear the board.
                this->_clear_board();

                // Put pieces on the board.
                int square_index = 0;
                for (char c : fen) {
                    if (c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8') {
                        square_index += int(c);
                    } else if (find(begin(PIECE_SYMBOLS), end(PIECE_SYMBOLS), tolower(c)) != end(PIECE_SYMBOLS)) {
                        Piece piece = Piece::from_symbol(c);
                        this->_set_piece_at(SQUARES_180[square_index], piece.piece_type, piece.color);
                        ++square_index;
                    } else if (c == '~') {
                        this->promoted |= BB_SQUARES[SQUARES_180[square_index - 1]];
                    }
                }
            }

            void _set_piece_map(const unordered_map<Square, Piece> &pieces) const {
                this->_clear_board();
                for (auto [square, piece] : pieces) {
                    this->_set_piece_at(square, piece.piece_type, piece.color);
                }
            }

            void _set_chess960_pos(int scharnagl) const {
                if (!(0 <= scharnagl && scharnagl <= 959)) {
                    throw invalid_argument("chess960 position index not 0 <= '" + to_string(scharnagl) + "' <= 959");
                }

                // See http://www.russellcottrell.com/Chess/Chess960.htm for
                // a description of the algorithm.
                int n = scharnagl / 4, bw = scharnagl % 4;
                int bb = n % 4;
                n /= 4;
                int q = n % 6;
                n /= 6;

                int n1, n2;
                for (n1 = 0; n1 < 4; ++n1) {
                    n2 = n + (3 - n1) * (4 - n1) / 2 - 5;
                    if (n1 < n2 && 1 <= n2 && n2 <= 4) {
                        break;
                    }
                }

                // Bishops.
                int bw_file = bw * 2 + 1;
                int bb_file = bb * 2;
                this->bishops = (BB_FILES[bw_file] | BB_FILES[bb_file]) & BB_BACKRANKS;

                // Queens.
                int q_file = q;
                q_file += int(min(bw_file, bb_file) <= q_file);
                q_file += int(max(bw_file, bb_file) <= q_file);
                this->queens = BB_FILES[q_file] & BB_BACKRANKS;

                vector<int> used = {bw_file, bb_file, q_file};

                // Knights.
                this->knights = BB_EMPTY;
                for (int i = 0; i < 8; ++i) {
                    if (find(begin(used), end(used), i) == end(used)) {
                        if (n1 == 0 || n2 == 0) {
                            this->knights |= BB_FILES[i] & BB_BACKRANKS;
                            used.push_back(i);
                        }
                    }
                    --n1;
                    --n2;
                }

                // RKR.
                for (int i = 0; i < 8; ++i) {
                    if (find(begin(used), end(used), i) == end(used)) {
                        this->rooks = BB_FILES[i] & BB_BACKRANKS;
                        used.push_back(i);
                        break;
                    }
                }
                for (int i = 1; i < 8; ++i) {
                    if (find(begin(used), end(used), i) == end(used)) {
                        this->kings = BB_FILES[i] & BB_BACKRANKS;
                        used.push_back(i);
                        break;
                    }
                }
                for (int i = 2; i < 8; ++i) {
                    if (find(begin(used), end(used), i) == end(used)) {
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

        ostream &operator<<(ostream &os, const BaseBoard &board) {
            os << "BaseBoard('" << board.board_fen() << "')";
            return os;
        }



        typedef Board BoardT;

        class _BoardState {

        public:
            Bitboard pawns, knights, bishops, rooks, queens, kings, occupied_w, occupied_b, occupied, promoted;
            Color turn;
            Bitboard castling_rights;
            optional<Square> ep_square;
            int halfmove_clock;
            int fullmove_number;

            _BoardState(const BoardT &board) {
                this->pawns = board.pawns;
                this->knights = board.knights;
                this->bishops = board.bishops;
                this->rooks = board.rooks;
                this->queens = board.queens;
                this->kings = board.kings;

                this->occupied_w = board.occupied_co[WHITE];
                this->occupied_b = board.occupied_co[BLACK];
                this->occupied = board.occupied;

                this->promoted = board.promoted;

                this->turn = board.turn;
                this->castling_rights = board.castling_rights;
                this->ep_square = board.ep_square;
                this->halfmove_clock = board.halfmove_clock;
                this->fullmove_number = board.fullmove_number;
            }

            void restore(BoardT &board) const {
                board.pawns = this->pawns;
                board.knights = this->knights;
                board.bishops = this->bishops;
                board.rooks = this->rooks;
                board.queens = this->queens;
                board.kings = this->kings;

                board.occupied_co[WHITE] = this->occupied_w;
                board.occupied_co[BLACK] = this->occupied_b;
                board.occupied = this->occupied;

                board.promoted = this->promoted;

                board.turn = this->turn;
                board.castling_rights = this->castling_rights;
                board.ep_square = this->ep_square;
                board.halfmove_clock = this->halfmove_clock;
                board.fullmove_number = this->fullmove_number;
            }
        };

        class Board : public BaseBoard {
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
            inline static string aliases[6] = {"Standard", "Chess", "Classical", "Normal", "Illegal", "From Position"};
            inline static optional<string> uci_variant = "chess";
            inline static optional<string> xboard_variant = "normal";
            inline static string starting_fen = STARTING_FEN;

            inline static optional<string> tbw_suffix = ".rtbw";
            inline static optional<string> tbz_suffix = ".rtbz";
            inline static optional<unsigned char> tbw_magic[4] = {0x71, 0xe8, 0x23, 0x5d};
            inline static optional<unsigned char> tbz_magic[4] = {0xd7, 0x66, 0x0c, 0xa5};
            inline static optional<string> pawnless_tbw_suffix = nullopt;
            inline static optional<string> pawnless_tbz_suffix = nullopt;
            inline static optional<unsigned char> pawnless_tbw_magic = nullopt;
            inline static optional<unsigned char> pawnless_tbz_magic = nullopt;
            inline static bool connected_kings = false;
            inline static bool one_king = true;
            inline static bool captures_compulsory = false;

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
            >>> std::cout << bool(board.castling_rights & chess.BB_H1);  // White can castle with the h1 rook
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

            optional<Square> ep_square;
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

            Bitboard promoted;
            /* A bitmask of pieces that have been promoted. */

            bool chess960;
            /*
            Whether the board is in Chess960 mode. In Chess960 castling moves are
            represented as king moves to the corresponding rook square.
            */

            vector<Move> move_stack;
            /*
            The move stack. Use :func:`Board::push() <chess::Board::push()>`,
            :func:`Board::pop() <chess::Board::pop()>`,
            :func:`Board::peek() <chess::Board::peek()>` and
            :func:`Board::clear_stack() <chess::Board::clear_stack()>` for
            manipulation.
            */

            Board(optional<string> fen = STARTING_FEN, bool chess960 = false) : BaseBoard(nullopt) {
                this->chess960 = chess960;

                this->ep_square = nullopt;

                if (fen == nullopt) {
                    this->clear();
                } else if (*fen == Board::starting_fen) {
                    this->reset();
                } else {
                    this->set_fen(*fen);
                }
            }

            LegalMoveGenerator legal_moves() const {
                /*
                A dynamic list of legal moves.

                >>> #include "chess.cpp"
                >>> #include <iostream>
                >>>
                >>> chess::Board board;
                >>> std::cout << board.legal_moves().count();
                20
                >>> std::cout << bool(board.legal_moves());
                1
                >>> chess::Move move = chess::Move::from_uci("g1f3");
                chess::LegalMoveGenerator legal_moves = board.legal_moves();
                >>> std::cout << std::find(std::begin(legal_moves), std::end(legal_moves), move) != std::end(legal_moves);
                1

                Wraps :func:`~chess::Board::generate_legal_moves()` and
                :func:`~chess::Board::is_legal()`.
                */
                return LegalMoveGenerator(this->generate_legal_moves());
            }

            PseudoLegalMoveGenerator pseudo_legal_moves() const {
                /*
                A dynamic list of pseudo-legal moves, much like the legal move list.

                Pseudo-legal moves might leave or put the king in check, but are
                otherwise valid. Null moves are not pseudo-legal. Castling moves are
                only included if they are completely legal.

                Wraps :func:`~chess::Board::generate_pseudo_legal_moves()` and
                :func:`~chess::Board::is_pseudo_legal()`.
                */
                return PseudoLegalMoveGenerator(this->generate_pseudo_legal_moves());
            }

            void reset() const {
                /* Restores the starting position. */
                this->turn = WHITE;
                this->castling_rights = BB_CORNERS;
                this->ep_square = nullopt;
                this->halfmove_clock = 0;
                this->fullmove_number = 1;

                this->reset_board();
            }


            void reset_board() const {
                /*
                Resets only pieces to the starting position. Use
                :func:`~chess::Board::reset()` to fully restore the starting position
                (including turn, castling rights, etc.).
                */
                BaseBoard::reset_board();
                this->clear_stack();
            }


            void clear() const {
                /*
                Clears the board.

                Resets move stack and move counters. The side to move is white. There
                are no rooks or kings, so castling rights are removed.

                In order to be in a valid :func:`~chess::Board::status()`, at least kings
                need to be put on the board.
                */
                this->turn = WHITE;
                this->castling_rights = BB_EMPTY;
                this->ep_square = nullopt;
                this->halfmove_clock = 0;
                this->fullmove_number = 1;

                this->clear_board();
            }
            

            void clear_board() const {
                BaseBoard::clear_board();
                this->clear_stack();
            }


            void clear_stack() const {
                /* Clears the move stack. */
                this->move_stack.clear();
                this->_stack.clear();
            }


            BoardT root() const {
                /* Returns a copy of the root position. */
                if (!this->_stack.empty()) {
                    BoardT board = Board(nullopt, this->chess960);
                    this->_stack.front().restore(board);
                    return board;
                } else {
                    return this->copy(false);
                }
            }


            int ply() const {
                /*
                Returns the number of half-moves since the start of the game, as
                indicated by :data:`~chess::Board::fullmove_number` and
                :data:`~chess::Board::turn`.

                If moves have been pushed from the beginning, this is usually equal to
                ``board.move_stack.size()``. But note that a board can be set up with
                arbitrary starting positions, and the stack can be cleared.
                */
                return 2 * (this->fullmove_number - 1) + (this->turn == BLACK);
            }


            optional<Piece> remove_piece_at(Square square) const {
                optional<Piece> piece = BaseBoard::remove_piece_at(square);
                this->clear_stack();
                return piece;
            }


            void set_piece_at(Square square, optional<Piece> piece, bool promoted = false) const {
                BaseBoard::set_piece_at(square, piece, promoted);
                this->clear_stack();
            }


            vector<Move> generate_pseudo_legal_moves(Bitboard from_mask = BB_ALL, Bitboard to_mask = BB_ALL) const {
                vector<Move> iter;
                Bitboard our_pieces = this->occupied_co[this->turn];

                // Generate piece moves.
                Bitboard non_pawns = our_pieces & ~this->pawns & from_mask;
                for (Square from_square : scan_reversed(non_pawns)) {
                    Bitboard moves = this->attacks_mask(from_square) & ~our_pieces & to_mask;
                    for (Square to_square : scan_reversed(moves)) {
                        iter.push_back(Move(from_square, to_square));
                    }
                }

                // Generate castling moves.
                if (from_mask & this->kings) {
                    for (const Move &move : this->generate_castling_moves(from_mask, to_mask)) {
                        iter.push_back(move);
                    }
                }

                // The remaining moves are all pawn moves.
                Bitboard pawns = this->pawns & this->occupied_co[this->turn] & from_mask;
                if (!pawns) {
                    return;
                }

                // Generate pawn captures.
                Bitboard capturers = pawns;
                for (Square from_square : scan_reversed(capturers)) {
                    Bitboard targets = (
                        BB_PAWN_ATTACKS[this->turn][from_square] &
                        this->occupied_co[!this->turn] & to_mask);

                    for (Square to_square : scan_reversed(targets)) {
                        if (square_rank(to_square) == 0 || square_rank(to_square) == 7) {
                            iter.push_back(Move(from_square, to_square, QUEEN));
                            iter.push_back(Move(from_square, to_square, ROOK));
                            iter.push_back(Move(from_square, to_square, BISHOP));
                            iter.push_back(Move(from_square, to_square, KNIGHT));
                        } else {
                            iter.push_back(Move(from_square, to_square));
                        }
                    }
                }

                // Prepare pawn advance generation.
                Bitboard single_moves, double_moves;
                if (this->turn == WHITE) {
                    single_moves = pawns << 8 & ~this->occupied;
                    double_moves = single_moves << 8 & ~this->occupied & (BB_RANK_3 | BB_RANK_4);
                } else {
                    single_moves = pawns >> 8 & ~this->occupied;
                    double_moves = single_moves >> 8 & ~this->occupied & (BB_RANK_6 | BB_RANK_5);
                }

                single_moves &= to_mask;
                double_moves &= to_mask;

                // Generate single pawn moves.
                for (Square to_square : scan_reversed(single_moves)) {
                    Square from_square = to_square + (this->turn == BLACK ? 8 : -8);

                    if (square_rank(to_square) == 0 || square_rank(to_square) == 7) {
                        iter.push_back(Move(from_square, to_square, QUEEN));
                        iter.push_back(Move(from_square, to_square, ROOK));
                        iter.push_back(Move(from_square, to_square, BISHOP));
                        iter.push_back(Move(from_square, to_square, KNIGHT));
                    } else {
                        iter.push_back(Move(from_square, to_square));
                    }
                }

                // Generate double pawn moves.
                for (Square to_square : scan_reversed(double_moves)) {
                    Square from_square = to_square + (this->turn == BLACK ? 16 : -16);
                    iter.push_back(Move(from_square, to_square));
                }

                // Generate en passant captures.
                if (this->ep_square) {
                    for (const Move &move : this->generate_pseudo_legal_ep(from_mask, to_mask)) {
                        iter.push_back(move);
                    }
                }
                return iter;
            }

            vector<Move> generate_pseudo_legal_ep(Bitboard from_mask = BB_ALL, Bitboard to_mask = BB_ALL) const {
                vector<Move> iter;
                if (!this->ep_square || !(BB_SQUARES[*this->ep_square] & to_mask)) {
                    return iter;
                }

                if (BB_SQUARES[*this->ep_square] & this->occupied) {
                    return iter;
                }

                Bitboard capturers = (
                    this->pawns & this->occupied_co[this->turn] & from_mask &
                    BB_PAWN_ATTACKS[!this->turn][*this->ep_square] &
                    BB_RANKS[this->turn ? 4 : 3]);

                for (Square capturer : scan_reversed(capturers)) {
                    iter.push_back(Move(capturer, *this->ep_square));
                }
                return iter;
            }

            vector<Move> generate_pseudo_legal_captures(Bitboard from_mask = BB_ALL, Bitboard to_mask = BB_ALL) const {
                vector<Move> iter;
                for (const Move &move : this->generate_pseudo_legal_moves(from_mask, to_mask & this->occupied_co[!this->turn])) {
                    iter.push_back(move);
                }
                for (const Move &move : this->generate_pseudo_legal_ep(from_mask, to_mask)) {
                    iter.push_back(move);
                }
                return iter;
            }

            Bitboard checkers_mask() const {
                optional<Square> king = this->king(this->turn);
                return king == nullopt ? BB_EMPTY : this->attackers_mask(!this->turn, *king);
            }

            SquareSet checkers() const {
                /*
                Gets the pieces currently giving check.

                Returns a :class:`set of squares <chess::SquareSet>`.
                */
                return SquareSet(this->checkers_mask());
            }


            bool is_check() const {
                /* Tests if the current side to move is in check. */
                return bool(this->checkers_mask());
            }


            bool gives_check(const Move &move) const {
                /*
                Probes if the given move would put the opponent in check. The move
                must be at least pseudo-legal.
                */
                this->push(move);
                try {
                    return this->is_check();
                } catch (...) {}
                this->pop();
            }


            bool is_into_check(const Move &move) const {
                optional<Square> king = this->king(this->turn);
                if (king == nullopt) {
                    return false;
                }

                // If already in check, look if it is an evasion.
                Bitboard checkers = this->attackers_mask(!this->turn, *king);
                vector<Move> evasions = this->_generate_evasions(*king, checkers, BB_SQUARES[move.from_square], BB_SQUARES[move.to_square]);
                if (checkers && find(begin(evasions), end(evasions), move) == end(evasions)) {
                    return true;
                }

                return !this->_is_safe(*king, this->_slider_blockers(*king), move);
            }

            bool was_into_check() const {
                optional<Square> king = this->king(!this->turn);
                return king != nullopt && this->is_attacked_by(this->turn, *king);
            }

            bool is_pseudo_legal(const Move &move) const {
                // Null moves are not pseudo-legal.
                if (!move) {
                    return false;
                }

                // Drops are not pseudo-legal.
                if (move.drop) {
                    return false;
                }

                // Source square must not be vacant.
                optional<PieceType> piece = this->piece_type_at(move.from_square);
                if (!piece) {
                    return false;
                }

                // Get square masks.
                Bitboard from_mask = BB_SQUARES[move.from_square];
                Bitboard to_mask = BB_SQUARES[move.to_square];

                // Check turn.
                if (!(this->occupied_co[this->turn] & from_mask)) {
                    return false;
                }

                // Only pawns can promote and only on the backrank.
                if (move.promotion) {
                    if (*piece != PAWN) {
                        return false;
                    }

                    if (this->turn == WHITE && square_rank(move.to_square) != 7) {
                        return false;
                    } else if (this->turn == BLACK && square_rank(move.to_square) != 0) {
                        return false;
                    }
                }

                // Handle castling.
                if (*piece == KING) {
                    Move move = this->_from_chess960(this->chess960, move.from_square, move.to_square);
                    vector<Move> castling_moves = this->generate_castling_moves();
                    if (find(begin(castling_moves), end(castling_moves), move) != end(castling_moves)) {
                        return true;
                    }
                }

                // Destination square can not be occupied.
                if (this->occupied_co[this->turn] & to_mask) {
                    return false;
                }

                // Handle pawn moves.
                if (*piece == PAWN) {
                    vector<Move> pseudo_legal_moves = this->generate_pseudo_legal_moves(from_mask, to_mask);
                    return find(begin(pseudo_legal_moves), end(pseudo_legal_moves), move) != end(pseudo_legal_moves);
                }

                // Handle all other pieces.
                return bool(this->attacks_mask(move.from_square) & to_mask);
            }

            bool is_legal(const Move &move) const {
                return !this->is_variant_end() && this->is_pseudo_legal(move) && !this->is_into_check(move);
            }

            bool is_variant_end() const {
                /*
                Checks if the game is over due to a special variant end condition.

                Note, for example, that stalemate is not considered a variant-specific
                end condition (this method will return ``false``), yet it can have a
                special **result** in suicide chess (any of
                :func:`~chess::Board::is_variant_loss()`,
                :func:`~chess::Board::is_variant_win()`,
                :func:`~chess::Board::is_variant_draw()` might return ``true``).
                */
                return false;
            }


            bool is_variant_loss() const {
                /*
                Checks if the current side to move lost due to a variant-specific
                condition.
                */
                return false;
            }


            bool is_variant_win() const {
                /*
                Checks if the current side to move won due to a variant-specific
                condition.
                */
                return false;
            }


            bool is_variant_draw() const {
                /*
                Checks if a variant-specific drawing condition is fulfilled.
                */
                return false;
            }


            bool is_game_over(bool claim_draw = false) const {
                return this->outcome(claim_draw) != nullopt;
            }

            string result(bool claim_draw = false) const {
                optional<Outcome> outcome = this->outcome(claim_draw);
                return outcome ? outcome->result() : "*";
            }

            optional<Outcome> outcome(bool claim_draw = false) const {
                /*
                Checks if the game is over due to
                :func:`checkmate <chess::Board::is_checkmate()>`,
                :func:`stalemate <chess::Board::is_stalemate()>`,
                :func:`insufficient material <chess::Board::is_insufficient_material()>`,
                the :func:`seventyfive-move rule <chess::Board::is_seventyfive_moves()>`,
                :func:`fivefold repetition <chess::Board::is_fivefold_repetition()>`,
                or a :func:`variant end condition <chess::Board::is_variant_end()>`.
                Returns the :class:`chess::Outcome` if the game has ended, otherwise
                ``std::nullopt``.

                Alternatively, use :func:`~chess::Board::is_game_over()` if you are not
                interested in who won the game and why.

                The game is not considered to be over by the
                :func:`fifty-move rule <chess::Board::can_claim_fifty_moves()>` or
                :func:`threefold repetition <chess::Board::can_claim_threefold_repetition()>`,
                unless *claim_draw* is given. Note that checking the latter can be
                slow.
                */
                // Variant support.
                if (this->is_variant_loss()) {
                    return Outcome(Termination::VARIANT_LOSS, !this->turn);
                }
                if (this->is_variant_win()) {
                    return Outcome(Termination::VARIANT_WIN, this->turn);
                }
                if (this->is_variant_draw()) {
                    return Outcome(Termination::VARIANT_DRAW, nullopt);
                }

                // Normal game end.
                if (this->is_checkmate()) {
                    return Outcome(Termination::CHECKMATE, !this->turn);
                }
                if (this->is_insufficient_material()) {
                    return Outcome(Termination::INSUFFICIENT_MATERIAL, nullopt);
                }
                if (this->generate_legal_moves().empty()) {
                    return Outcome(Termination::STALEMATE, nullopt);
                }

                // Automatic draws.
                if (this->is_seventyfive_moves()) {
                    return Outcome(Termination::SEVENTYFIVE_MOVES, nullopt);
                }
                if (this->is_fivefold_repetition()) {
                    return Outcome(Termination::FIVEFOLD_REPETITION, nullopt);
                }

                // Claimable draws.
                if (claim_draw) {
                    if (this->can_claim_fifty_moves()) {
                        return Outcome(Termination::FIFTY_MOVES, nullopt);
                    }
                    if (this->can_claim_threefold_repetition()) {
                        return Outcome(Termination::THREEFOLD_REPETITION, nullopt);
                    }
                }

                return nullopt;
            }


            bool is_checkmate() const {
                /* Checks if the current position is a checkmate. */
                if (!this->is_check()) {
                    return false;
                }

                return this->generate_legal_moves().empty();
            }


            bool is_stalemate() const {
                /* Checks if the current position is a stalemate. */
                if (this->is_check()) {
                    return false;
                }

                if (this->is_variant_end()) {
                    return false;
                }

                return this->generate_legal_moves().empty();
            }


            bool is_insufficient_material() const {
                /*
                Checks if neither side has sufficient winning material
                (:func:`~chess::Board::has_insufficient_material()`).
                */
                return this->has_insufficient_material(WHITE) && this->has_insufficient_material(BLACK);
            }


            bool has_insufficient_material(Color color) const {
                /*
                Checks if *color* has insufficient winning material.

                This is guaranteed to return ``false`` if *color* can still win the
                game.

                The converse does not necessarily hold:
                The implementation only looks at the material, including the colors
                of bishops, but not considering piece positions. So fortress
                positions or positions with forced lines may return ``false``, even
                though there is no possible winning line.
                */
                if (this->occupied_co[color] & (this->pawns | this->rooks | this->queens)) {
                    return false;
                }

                // Knights are only insufficient material if:
                // (1) We do not have any other pieces, including more than one knight.
                // (2) The opponent does not have pawns, knights, bishops or rooks.
                //     These would allow selfmate.
                if (this->occupied_co[color] & this->knights) {
                    return (popcount(this->occupied_co[color]) <= 2 &&
                            !(this->occupied_co[!color] & ~this->kings & ~this->queens));
                }

                // Bishops are only insufficient material if:
                // (1) We do not have any other pieces, including bishops of the
                //     opposite color.
                // (2) The opponent does not have bishops of the opposite color,
                //     pawns or knights. These would allow selfmate.
                if (this->occupied_co[color] & this->bishops) {
                    bool same_color = (!(this->bishops & BB_DARK_SQUARES)) || (!(this->bishops & BB_LIGHT_SQUARES));
                    return same_color && !this->pawns && !this->knights;
                }

                return true;
            }


            bool is_seventyfive_moves() const {
                /*
                Since the 1st of July 2014, a game is automatically drawn (without
                a claim by one of the players) if the half-move clock since a capture
                or pawn move is equal to or greater than 150. Other means to end a game
                take precedence.
                */
                return this->_is_halfmoves(150);
            }


            bool is_fivefold_repetition() const {
                /*
                Since the 1st of July 2014 a game is automatically drawn (without
                a claim by one of the players) if a position occurs for the fifth time.
                Originally this had to occur on consecutive alternating moves, but
                this has since been revised.
                */
                return this->is_repetition(5);
            }


            bool can_claim_draw() const {
                /*
                Checks if the player to move can claim a draw by the fifty-move rule or
                by threefold repetition.

                Note that checking the latter can be slow.
                */
                return this->can_claim_fifty_moves() || this->can_claim_threefold_repetition();
            }


            bool is_fifty_moves() const {
                return this->_is_halfmoves(100);
            }

            bool can_claim_fifty_moves() const {
                /*
                Checks if the player to move can claim a draw by the fifty-move rule.

                Draw by the fifty-move rule can be claimed once the clock of halfmoves
                since the last capture or pawn move becomes equal or greater to 100,
                or if there is a legal move that achieves this. Other means of ending
                the game take precedence.
                */
                if (this->is_fifty_moves()) {
                    return true;
                }

                if (this->halfmove_clock >= 99) {
                    for (const Move &move : this->generate_legal_moves()) {
                        if (!this->is_zeroing(move)) {
                            this->push(move);
                            try {
                                if (this->is_fifty_moves()) {
                                    return true;
                                }
                            } catch (...) {}
                            this->pop();
                        }
                    }
                }

                return false;
            }


            bool can_claim_threefold_repetition() const {
                /*
                Checks if the player to move can claim a draw by threefold repetition.

                Draw by threefold repetition can be claimed if the position on the
                board occured for the third time or if such a repetition is reached
                with one of the possible legal moves.

                Note that checking this can be slow: In the worst case
                scenario, every legal move has to be tested and the entire game has to
                be replayed because there is no incremental transposition table.
                */
                auto transposition_key = this->_transposition_key();
                struct transposition_hash {
                    size_t operator()(const tuple<Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Color, Bitboard, optional<Square>> &key) const {
                        return hash<Bitboard>()(get<0>(key)) ^ hash<Bitboard>()(get<1>(key)) ^ hash<Bitboard>()(get<2>(key)) ^ hash<Bitboard>()(get<3>(key)) ^ hash<Bitboard>()(get<4>(key)) ^ hash<Bitboard>()(get<5>(key)) ^ hash<Bitboard>()(get<6>(key)) ^ hash<Bitboard>()(get<7>(key)) ^ hash<Color>()(get<8>(key)) ^ hash<Bitboard>()(get<9>(key)) ^ (get<10>(key) ? hash<Square>()(*get<10>(key)) : hash<int>()(64));
                    }
                };
                unordered_map<tuple<Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Color, Bitboard, optional<Square>>, int, transposition_hash> transpositions;
                ++transpositions[transposition_key];

                // Count positions.
                stack<Move> switchyard;
                while (!this->move_stack.empty()) {
                    Move move = this->pop();
                    switchyard.push(move);

                    if (this->is_irreversible(move)) {
                        break;
                    }

                    ++transpositions[this->_transposition_key()];
                }

                while (!switchyard.empty()) {
                    this->push(switchyard.top());
                    switchyard.pop();
                }

                // Threefold repetition occured.
                if (transpositions.at(transposition_key) >= 3) {
                    return true;
                }

                // The next legal move is a threefold repetition.
                for (const Move &move : this->generate_legal_moves()) {
                    this->push(move);
                    try {
                        if (transpositions.at(this->_transposition_key()) >= 2) {
                            return true;
                        }
                    } catch (...) {}
                    this->pop();
                }

                return false;
            }


            bool is_repetition(int count = 3) const {
                /*
                Checks if the current position has repeated 3 (or a given number of)
                times.

                Unlike :func:`~chess::Board::can_claim_threefold_repetition()`,
                this does not consider a repetition that can be played on the next
                move.

                Note that checking this can be slow: In the worst case, the entire
                game has to be replayed because there is no incremental transposition
                table.
                */
                // Fast check, based on occupancy only.
                int maybe_repetitions = 1;
                for (auto it = rbegin(this->_stack); it != rend(this->_stack); ++it) {
                    _BoardState state = *it;
                    if (state.occupied == this->occupied) {
                        ++maybe_repetitions;
                        if (maybe_repetitions >= count) {
                            break;
                        }
                    }
                }
                if (maybe_repetitions < count) {
                    return false;
                }

                // Check full replay.
                auto transposition_key = this->_transposition_key();
                stack<Move> switchyard;

                try {
                    while (true) {
                        if (count <= 1) {
                            return true;
                        }

                        if (this->move_stack.size() < count - 1) {
                            break;
                        }

                        Move move = this->pop();
                        switchyard.push(move);

                        if (this->is_irreversible(move)) {
                            break;
                        }

                        if (this->_transposition_key() == transposition_key) {
                            --count;
                        }
                    }
                } catch (...) {}
                while (!switchyard.empty()) {
                    this->push(switchyard.top());
                    switchyard.pop();
                }

                return false;
            }


            void push(Move move) const {
                /*
                Updates the position with the given *move* and puts it onto the
                move stack.

                >>> #include "chess.cpp"
                >>> #include <iostream>
                >>>
                >>> chess::Board board;
                >>>
                >>> chess::Move Nf3 = chess::Move::from_uci("g1f3");
                >>> board.push(Nf3);  // Make the move

                >>> std::cout << board.pop();  // Unmake the last move
                Move::from_uci('g1f3')

                Null moves just increment the move counters, switch turns and forfeit
                en passant capturing.

                .. warning::
                    Moves are not checked for legality. It is the caller's
                    responsibility to ensure that the move is at least pseudo-legal or
                    a null move.
                */
                // Push move and remember board state.
                move = this->_to_chess960(move);
                _BoardState board_state = this->_board_state();
                this->castling_rights = this->clean_castling_rights(); // Before pushing stack
                this->move_stack.push_back(this->_from_chess960(this->chess960, move.from_square, move.to_square, move.promotion, move.drop));
                this->_stack.push_back(board_state);

                // Reset en passant square.
                optional<Square> ep_square = this->ep_square;
                this->ep_square = nullopt;

                // Increment move counters.
                ++this->halfmove_clock;
                if (this->turn == BLACK) {
                    ++this->fullmove_number;
                }

                // On a null move, simply swap turns and reset the en passant square.
                if (!move) {
                    this->turn = !this->turn;
                    return;
                }

                // Drops.
                if (move.drop) {
                    this->_set_piece_at(move.to_square, *move.drop, this->turn);
                    this->turn = !this->turn;
                    return;
                }

                // Zero the half-move clock.
                if (this->is_zeroing(move)) {
                    this->halfmove_clock = 0;
                }

                Bitboard from_bb = BB_SQUARES[move.from_square];
                Bitboard to_bb = BB_SQUARES[move.to_square];

                bool promoted = bool(this->promoted & from_bb);
                optional<PieceType> piece_type = this->_remove_piece_at(move.from_square);
                if (piece_type == nullopt) {
                    throw "push() expects move to be pseudo-legal, but got " + string(move) + " in " + this->board_fen();
                }
                Square capture_square = move.to_square;
                optional<PieceType> captured_piece_type = this->piece_type_at(capture_square);

                // Update castling rights.
                this->castling_rights &= ~to_bb & ~from_bb;
                if (*piece_type == KING && !promoted) {
                    if (this->turn == WHITE) {
                        this->castling_rights &= ~BB_RANK_1;
                    } else {
                        this->castling_rights &= ~BB_RANK_8;
                    }
                } else if (captured_piece_type && *captured_piece_type == KING && !(this->promoted & to_bb)) {
                    if (this->turn == WHITE && square_rank(move.to_square) == 7) {
                        this->castling_rights &= ~BB_RANK_8;
                    } else if (this->turn == BLACK && square_rank(move.to_square) == 0) {
                        this->castling_rights &= ~BB_RANK_1;
                    }
                }

                // Handle special pawn moves.
                if (*piece_type == PAWN) {
                    int diff = move.to_square - move.from_square;

                    if (diff == 16 && square_rank(move.from_square) == 1) {
                        this->ep_square = move.from_square + 8;
                    } else if (diff == -16 && square_rank(move.from_square) == 6) {
                        this->ep_square = move.from_square - 8;
                    } else if (move.to_square == *ep_square && (abs(diff) == 7 || abs(diff) == 9) && !captured_piece_type) {
                        // Remove pawns captured en passant.
                        int down = this->turn == WHITE ? -8 : 8;
                        capture_square = *ep_square + down;
                        captured_piece_type = this->_remove_piece_at(capture_square);
                    }
                }

                // Promotion.
                if (move.promotion) {
                    promoted = true;
                    piece_type = move.promotion;
                }

                // Castling.
                bool castling = *piece_type == KING && this->occupied_co[this->turn] & to_bb;
                if (castling) {
                    bool a_side = square_file(move.to_square) < square_file(move.from_square);

                    this->_remove_piece_at(move.from_square);
                    this->_remove_piece_at(move.to_square);

                    if (a_side) {
                        this->_set_piece_at(this->turn == WHITE ? C1 : C8, KING, this->turn);
                        this->_set_piece_at(this->turn == WHITE ? D1 : D8, ROOK, this->turn);
                    } else {
                        this->_set_piece_at(this->turn == WHITE ? G1 : G8, KING, this->turn);
                        this->_set_piece_at(this->turn == WHITE ? F1 : F8, ROOK, this->turn);
                    }
                }

                // Put the piece on the target square.
                if (!castling) {
                    bool was_promoted = bool(this->promoted & to_bb);
                    this->_set_piece_at(move.to_square, *piece_type, this->turn, promoted);

                    if (captured_piece_type) {
                        this->_push_capture(move, capture_square, *captured_piece_type, was_promoted);
                    }
                }

                // Swap turn.
                this->turn = !this->turn;
            }


            Move pop() const {
                /*
                Restores the previous position and returns the last move from the stack.

                :throws: :exc:`std::out_of_range` if the move stack is empty.
                */
                if (this->move_stack.empty()) {
                    throw out_of_range("");
                }
                Move move = this->move_stack.back();
                this->move_stack.pop_back();
                this->_stack.back().restore(this);
                this->_stack.pop_back();
                return move;
            }


            Move peek() const {
                /*
                Gets the last move from the move stack.

                :throws: :exc:`std::out_of_range` if the move stack is empty.
                */
                if (this->move_stack.empty()) {
                    throw out_of_range("");
                }
                return this->move_stack.back();
            }


            Move find_move(Square from_square, Square to_square, optional<PieceType> promotion = nullopt) const {
                /*
                Finds a matching legal move for an origin square, a target square, and
                an optional promotion piece type.

                For pawn moves to the backrank, the promotion piece type defaults to
                :data:`chess::QUEEN`, unless otherwise specified.

                Castling moves are normalized to king moves by two steps, except in
                Chess960.

                :throws: :exc:`std::invalid_argument` if no matching legal move is found.
                */
                if (promotion == nullopt && this->pawns & BB_SQUARES[from_square] && BB_SQUARES[to_square] & BB_BACKRANKS) {
                    promotion = QUEEN;
                }

                Move move = this->_from_chess960(this->chess960, from_square, to_square, promotion);
                if (!this->is_legal(move)) {
                    throw invalid_argument("no matching legal move for " + move.uci() + " (" + SQUARE_NAMES[from_square] + " -> " + SQUARE_NAMES[to_square] + " in " + this->fen());
                }

                return move;
            }


            string castling_shredder_fen() {
                Bitboard castling_rights = this->clean_castling_rights();
                if (!castling_rights) {
                    return "-";
                }

                vector<char> builder;

                for (Square square : scan_reversed(castling_rights & BB_RANK_1)) {
                    builder.push_back(toupper(FILE_NAMES[square_file(square)]));
                }

                for (Square square : scan_reversed(castling_rights & BB_RANK_8)) {
                    builder.push_back(FILE_NAMES[square_file(square)]);
                }

                return string(begin(builder), end(builder));
            }

            string castling_xfen() const {
                vector<char> builder;

                for (Color color : COLORS) {
                    optional<Square> king = this->king(color);
                    if (king == nullopt) {
                        continue;
                    }

                    int king_file = square_file(*king);
                    Bitboard backrank = color == WHITE ? BB_RANK_1 : BB_RANK_8;

                    for (Square rook_square : scan_reversed(this->clean_castling_rights() & backrank)) {
                        int rook_file = square_file(rook_square);
                        bool a_side = rook_file < king_file;

                        Bitboard other_rooks = this->occupied_co[color] & this->rooks & backrank & ~BB_SQUARES[rook_square];

                        char ch = a_side ? 'q' : 'k';
                        for (Square other : scan_reversed(other_rooks)) {
                            if ((square_file(other) < rook_file) == a_side) {
                                ch = FILE_NAMES[rook_file];
                                break;
                            }
                        }

                        builder.push_back(color == WHITE ? toupper(ch) : ch);
                    }
                }

                if (!builder.empty()) {
                    return string(begin(builder), end(builder));
                } else {
                    return "-";
                }
            }

            bool has_pseudo_legal_en_passant() const {
                /* Checks if there is a pseudo-legal en passant capture. */
                return this->ep_square != nullopt && !this->generate_pseudo_legal_ep().empty();
            }

            bool has_legal_en_passant() const {
                /* Checks if there is a legal en passant capture. */
                return this->ep_square != nullopt && !this->generate_legal_ep().empty();
            }

            string fen(bool shredder = false, _EnPassantSpec en_passant = "legal", optional<bool> promoted = nullopt) const {
                /*
                Gets a FEN representation of the position.

                A FEN string (e.g.,
                ``rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1``) consists
                of the board part :func:`~chess::Board::board_fen()`, the
                :data:`~chess::Board::turn`, the castling part
                (:data:`~chess::Board::castling_rights`),
                the en passant square (:data:`~chess::Board::ep_square`),
                the :data:`~chess::Board::halfmove_clock`
                and the :data:`~chess::Board::fullmove_number`.

                :param shredder: Use :func:`~chess::Board::castling_shredder_fen()`
                    and encode castling rights by the file of the rook
                    (like ``HAha``) instead of the default
                    :func:`~chess::Board::castling_xfen()` (like ``KQkq``).
                :param en_passant: By default, only fully legal en passant squares
                    are included (:func:`~chess::Board::has_legal_en_passant()`).
                    Pass ``fen`` to strictly follow the FEN specification
                    (always include the en passant square after a two-step pawn move)
                    or ``xfen`` to follow the X-FEN specification
                    (:func:`~chess::Board::has_pseudo_legal_en_passant()`).
                :param promoted: Mark promoted pieces like ``Q~``. By default, this is
                    only enabled in chess variants where this is relevant.
                */
                return this->epd(shredder, en_passant, promoted) + " " + to_string(this->halfmove_clock) + " " + to_string(this->fullmove_number);
            }

            string shredder_fen(_EnPassantSpec en_passant = "legal", optional<bool> promoted = nullopt) const {
                return this->epd(true, en_passant, promoted) + " " + to_string(this->halfmove_clock) + " " + to_string(this->fullmove_number);
            }

            void set_fen(const string &fen) {
                /*
                Parses a FEN and sets the position from it.

                :throws: :exc:`std::invalid_argument` if syntactically invalid. Use
                    :func:`~chess::Board::is_valid()` to detect invalid positions.
                */
                istringstream iss(fen);
                deque<string> parts = {istream_iterator<string>(iss), {}};

                // Board part.
                string board_part;
                if (!parts.empty()) {
                    board_part = parts.front();
                    parts.pop_front();
                } else {
                    throw invalid_argument("empty fen");
                }

                // Turn.
                string turn_part;
                Color turn;
                if (!parts.empty()) {
                    turn_part = parts.front();
                    parts.pop_front();

                    if (turn_part == "w") {
                        turn = WHITE;
                    } else if (turn_part == "b") {
                        turn = BLACK;
                    } else {
                        throw invalid_argument("expected 'w' or 'b' for turn part of fen: '" + fen + "'");
                    }
                } else {
                    turn = WHITE;
                }

                // Validate castling part.
                string castling_part;
                if (!parts.empty()) {
                    castling_part = parts.front();
                    parts.pop_front();

                    if (!regex_match(castling_part, FEN_CASTLING_REGEX)) {
                        throw invalid_argument("invalid castling part in fen: '" + fen + "'");
                    }
                } else {
                    castling_part = "-";
                }

                // En passant square.
                optional<string> ep_part;
                optional<Square> ep_square;
                if (!parts.empty()) {
                    ep_part = parts.front();
                    parts.pop_front();

                    if (ep_part != "-") {
                        auto it = find(begin(SQUARE_NAMES), end(SQUARE_NAMES), ep_part);
                        if (it == end(SQUARE_NAMES)) {
                            throw invalid_argument("invalid en passant part in fen: '" + fen + "'");
                        }
                        ep_square = distance(SQUARE_NAMES, it);
                    } else {
                        ep_square = nullopt;
                    }
                } else {
                    ep_square = nullopt;
                }

                // Check that the half-move part is valid.
                string halfmove_part;
                int halfmove_clock;
                if (!parts.empty()) {
                    halfmove_part = parts.front();
                    parts.pop_front();

                    try {
                        halfmove_clock = stoi(halfmove_part);
                    } catch (invalid_argument) {
                        throw invalid_argument("invalid half-move clock in fen: '" + fen + "'");
                    }

                    if (halfmove_clock < 0) {
                        throw invalid_argument("half-move clock cannot be negative: '" + fen + "'");
                    }
                } else {
                    halfmove_clock = 0;
                }

                // Check that the full-move number part is valid.
                // 0 is allowed for compatibility, but later replaced with 1.
                string fullmove_part;
                int fullmove_number;
                if (!parts.empty()) {
                    fullmove_part = parts.front();
                    parts.pop_front();

                    try {
                        fullmove_number = stoi(fullmove_part);
                    } catch (invalid_argument) {
                        throw invalid_argument("invalid fullmove number in fen: '" + fen + "'");
                    }

                    if (fullmove_number < 0) {
                        throw invalid_argument("fullmove number cannot be negative: '" + fen + "'");
                    }

                    fullmove_number = max(fullmove_number, 1);
                } else {
                    fullmove_number = 1;
                }

                // All parts should be consumed now.
                if (!parts.empty()) {
                    throw invalid_argument("fen string has more parts than expected: '" + fen + "'");
                }

                // Validate the board part and set it.
                this->_set_board_fen(board_part);

                // Apply.
                this->turn = turn;
                this->_set_castling_fen(castling_part);
                this->ep_square = ep_square;
                this->halfmove_clock = halfmove_clock;
                this->fullmove_number = fullmove_number;
                this->clear_stack();
            }


            void set_castling_fen(const string &castling_fen) const {
                /*
                Sets castling rights from a string in FEN notation like ``Qqk``.

                :throws: :exc:`std::invalid_argument` if the castling FEN is syntactically
                    invalid.
                */
                this->_set_castling_fen(castling_fen);
                this->clear_stack();
            }


            void set_board_fen(const string &fen) const {
                BaseBoard::set_board_fen(fen);
                this->clear_stack();
            }


            void set_piece_map(const unordered_map<Square, Piece> &pieces) const {
                BaseBoard::set_piece_map(pieces);
                this->clear_stack();
            }


            void set_chess960_pos(int scharnagl) const {
                BaseBoard::set_chess960_pos(scharnagl);
                this->chess960 = true;
                this->turn = WHITE;
                this->castling_rights = this->rooks;
                this->ep_square = nullopt;
                this->halfmove_clock = 0;
                this->fullmove_number = 1;

                this->clear_stack();
            }


            optional<int> chess960_pos(bool ignore_turn = false, bool ignore_castling = false, bool ignore_counters = true) const {
                /*
                Gets the Chess960 starting position index between 0 and 956,
                or ``std::nullopt`` if the current position is not a Chess960 starting
                position.

                By default, white to move (**ignore_turn**) and full castling rights
                (**ignore_castling**) are required, but move counters
                (**ignore_counters**) are ignored.
                */
                if (this->ep_square) {
                    return nullopt;
                }

                if (!ignore_turn) {
                    if (this->turn != WHITE) {
                        return nullopt;
                    }
                }

                if (!ignore_castling) {
                    if (this->clean_castling_rights() != this->rooks) {
                        return nullopt;
                    }
                }

                if (!ignore_counters) {
                    if (this->fullmove_number != 1 || this->halfmove_clock != 0) {
                        return nullopt;
                    }
                }

                return BaseBoard::chess960_pos();
            }


            string epd(bool shredder = false, const _EnPassantSpec &en_passant = "legal", optional<bool> promoted = nullopt, const unordered_map<string, optional<variant<string, int, float, Move , vector<Move>>>> &operations = {}) const {
                /*
                Gets an EPD representation of the current position.

                See :func:`~chess::Board::fen()` for FEN formatting options (*shredder*,
                *ep_square* and *promoted*).

                Supported operands for EPD operations
                are strings, integers, finite floats, legal moves and ``std::nullopt``.
                Additionally, the operation ``pv`` accepts a legal variation as
                a list of moves. The operations ``am`` and ``bm`` accept a list of
                legal moves in the current position.

                The name of the field cannot be a lone dash and cannot contain spaces,
                newlines, carriage returns or tabs.

                *hmvc* and *fmvn* are not included by default. You can use:

                >>> #include "chess.cpp"
                >>> #include <iostream>
                >>> #include <optional>
                >>>
                >>> chess::Board board;
                >>> std::cout << board.epd(false, "legal", std::nullopt, {{"hmvc", board.halfmove_clock}, {"fmvn", board.fullmove_number}});
                rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - hmvc 0; fmvn 1;
                */
                optional<Square> ep_square;
                if (en_passant == "fen") {
                    ep_square = this->ep_square;
                } else if (en_passant == "xfen") {
                    ep_square = this->has_pseudo_legal_en_passant() ? this->ep_square : nullopt;
                } else {
                    ep_square = this->has_legal_en_passant() ? this->ep_square : nullopt;
                }

                vector<string> epd = {this->board_fen(promoted),
                                      this->turn == WHITE ? "w" : "b",
                                      shredder ? this->castling_shredder_fen() : this->castling_xfen(),
                                      ep_square != nullopt ? SQUARE_NAMES[*ep_square] : "-"};

                if (!operations.empty()) {
                    epd.push_back(this->_epd_operations(operations));
                }

                string s;
                for (const string &s2 : epd) {
                    s += s2;
                    s += " ";
                }
                s.resize(s.size() - 1);
                return s;
            }


            unordered_map<string, optional<variant<string, int, float, Move , vector<Move>>>> set_epd(const string &epd) const {
                /*
                Parses the given EPD string and uses it to set the position.

                If present, ``hmvc`` and ``fmvn`` are used to set the half-move
                clock and the full-move number. Otherwise, ``0`` and ``1`` are used.

                Returns a dictionary of parsed operations. Values can be strings,
                integers, floats, move objects, or lists of moves.

                :throws: :exc:`std::invalid_argument` if the EPD string is invalid.
                */
                auto it = begin(epd);
                auto it2 = rbegin(epd);
                while (isspace(*it)) {
                    ++it;
                }
                while (isspace(*it2) || *it2 == ';') {
                    ++it2;
                }
                vector<string> parts;
                string s = string(it, it2.base());
                istringstream iss(s);
                string s2;
                for (int i = 0; getline(iss, s2, ' ') && i < 4; ++i) {
                    parts.push_back(s2);
                }
                int i;
                for (int i = 0, splits = 0; i < s.length() - 1 && splits < 4; ++i) {
                    if (isspace(s[i + 1])) {
                        ++splits;
                        ++i;
                        while (i < s.length() - 1 && isspace(s[i + 1])) {
                            ++i;
                        }
                    }
                }
                parts.push_back(s.substr(i));

                // Parse ops.
                if (parts.size() > 4) {
                    string joined;
                    for (string s : parts) {
                        joined += s;
                        joined += " ";
                    }
                    joined.resize(joined.size() - 1);
                    auto operations = this->_parse_epd_ops(parts.back(), [&]() -> Board {return Board(joined + " 0 1");});
                    parts.pop_back();
                    if (operations.find("hmvc") != end(operations)) {
                        if (holds_alternative<string>(*operations["hmvc"])) {
                            joined += " " + get<string>(*operations["hmvc"]);
                        } else if (holds_alternative<int>(*operations["hmvc"])) {
                            joined += " " + to_string(get<int>(*operations["hmvc"]));
                        } else {
                            joined += " " + to_string(int(get<float>(*operations["hmvc"])));
                        }
                    } else {
                        joined += " 0";
                    }
                    if (operations.find("fmvn") != end(operations)) {
                        if (holds_alternative<string>(*operations["fmvn"])) {
                            joined += " " + get<string>(*operations["fmvn"]);
                        } else if (holds_alternative<int>(*operations["fmvn"])) {
                            joined += " " + to_string(get<int>(*operations["fmvn"]));
                        } else {
                            joined += " " + to_string(int(get<float>(*operations["fmvn"])));
                        }
                    } else {
                        joined += " 1"
                    }
                    this->set_fen(joined);
                    return operations;
                } else {
                    this->set_fen(epd);
                    return {};
                }
            }


            string san(const Move &move) const {
                /*
                Gets the standard algebraic notation of the given move in the context
                of the current position.
                */
                return this->_algebraic(move);
            }


            string lan(const Move &move) const {
                /*
                Gets the long algebraic notation of the given move in the context of
                the current position.
                */
                return this->_algebraic(move, true);
            }


            string san_and_push(const Move &move) const {
                return this->_algebraic_and_push(move);
            }

            string variation_san(vector<Move> variation) {
                /*
                Given a sequence of moves, returns a string representing the sequence
                in standard algebraic notation (e.g., ``1. e4 e5 2. Nf3 Nc6`` or
                ``37...Bg6 38. fxg6``).

                The board will not be modified as a result of calling this.

                :throws: :exc:`std::invalid_argument` if any moves in the sequence are illegal.
                */
                BaseBoardT = this->copy(false);
                vector<string> san;

                for (const Move &move : variation) {
                    if (!board->is_legal(move))
                        throw invalid_argument("illegal move " + string(*move) + " in position " + board->fen());

                    if (board->turn == WHITE)
                        san.push_back(board->fullmove_number + ". " + board->san_and_push(move));
                    else if (san.empty())
                        san.push_back(board->fullmove_number + "..." + board->san_and_push(move));
                    else
                        san.push_back(board->san_and_push(move));
                }

                return string(begin(san), end(san));
            }

            Move parse_san(const string &san) {
                /*
                Uses the current position as the context to parse a move in standard
                algebraic notation and returns the corresponding move object.

                Ambiguous moves are rejected. Overspecified moves (including long
                algebraic notation) are accepted.

                The returned move is guaranteed to be either legal or a null move.

                :throws: :exc:`std::invalid_argument` if the SAN is invalid, illegal or ambiguous.
                */
                // Castling.
                try {
                    if (san == "O-O" || san == "O-O+" || san == "O-O#" || san == "0-0" || san == "0-0+" || san == "0-0#") {
                        for (const Move &move : this->generate_castling_moves()) {
                            if (this->is_kingside_castling(move))
                                return move;
                        }
                        throw out_of_range("");
                    } else if (san == "O-O-O" || san == "O-O-O+" || san == "O-O-O#" || san == "0-0-0" || san == "0-0-0+" || san == "0-0-0#") {
                        for (const Move &move : this->generate_castling_moves()) {
                            if (this->is_queenside_castling(move))
                                return move;
                        }
                        throw out_of_range("");
                    }
                } catch (out_of_range) {
                    throw invalid_argument("illegal san: '" + san + "' in" + this->fen());
                }

                // Match normal moves.
                smatch match;
                regex_search(san, match, SAN_REGEX);
                if (match.empty()) {
                    // Null moves.
                    if (san == "--" || san == "Z0" || san == "0000" || san == "@@@@")
                        return Move::null();
                    else if (san.find(',') != string::npos)
                        throw invalid_argument("unsupported multi-leg move: " + san);
                    else
                        throw invalid_argument("invalid san: " + san);
                }

                // Get target square. Mask our own pieces to exclude castling moves.
                auto it = find(begin(SQUARE_NAMES), end(SQUARE_NAMES), match[4].str());
                if (it == end(SQUARE_NAMES))
                    throw invalid_argument("");
                Square to_square = distance(SQUARE_NAMES, it);
                Bitboard to_mask = BB_SQUARES[to_square] & ~this->occupied_co[this->turn];

                // Get the promotion piece type.
                string p = match[5].str();
                optional<PieceType> promotion;
                if (!p.empty()) {
                    auto it = find(begin(PIECE_SYMBOLS), end(PIECE_SYMBOLS), tolower(p.back()));
                    if (it == end(PIECE_SYMBOLS))
                        throw invalid_argument("");
                    promotion = distance(PIECE_SYMBOLS, it);
                } else
                    promotion = nullopt;

                // Filter by original square.
                int from_file;
                Bitboard from_mask = BB_ALL;
                if (!match[2].str().empty()) {
                    auto it = find(begin(FILE_NAMES), end(FILE_NAMES), match[2].str());
                    if (it == end(FILE_NAMES))
                        throw invalid_argument("");
                    from_file = distance(FILE_NAMES, it);
                    from_mask &= BB_FILES[from_file];
                }
                int from_rank;
                if (!match[3].str().empty()) {
                    from_rank = stoi(match[3].str()) - 1;
                    from_mask &= BB_RANKS[from_rank];
                }

                // Filter by piece type.
                if (!match[1].str().empty()) {
                    auto it = find(begin(PIECE_SYMBOLS), end(PIECE_SYMBOLS), tolower(match[1].str().front()));
                    if (it == end(PIECE_SYMBOLS))
                        throw invalid_argument("");
                    PieceType piece_type = distance(PIECE_SYMBOLS, it);
                    from_mask &= this->pieces_mask(piece_type, this->turn);
                } else if (!match[2].str().empty() && !match[3].str().empty()) {
                    // Allow fully specified moves, even if they are not pawn moves,
                    // including castling moves.
                    Move move = this->find_move(square(from_file, from_rank), to_square, promotion);
                    if (!move.promotion && !promotion || *move.promotion == *promotion)
                        return move;
                    else
                        throw invalid_argument("missing promotion piece type: '" + san + "' in " + this->fen());
                } else
                    from_mask &= this->pawns;

                // Match legal moves.
                optional<Move> matched_move = nullopt;
                for (const Move &move : this->generate_legal_moves(from_mask, to_mask)) {
                    if (bool(move.promotion) != bool(promotion) || *move.promotion != *promotion)
                        continue;

                    if (matched_move)
                        throw invalid_argument("ambiguous san: '" + san + "' in " + this->fen());

                    matched_move = move;
                }

                if (!matched_move)
                    throw invalid_argument("illegal san: '" + san + "' in " + this->fen());

                return *matched_move;
            }

            Move push_san(const string &san) {
                /*
                Parses a move in standard algebraic notation, makes the move and puts
                it onto the move stack.

                Returns the move.

                :throws: :exc:`std::invalid_argument` if neither legal nor a null move.
                */
                Move move = this->parse_san(san);
                this->push(move);
                return move;
            }

            string uci(Move move, optional<bool> chess960 = nullopt) const {
                /*
                Gets the UCI notation of the move.

                *chess960* defaults to the mode of the board. Pass ``True`` to force
                Chess960 mode.
                */
                if (chess960 == nullopt)
                    chess960 = this->chess960;

                move = this->_to_chess960(move);
                move = this->_from_chess960(*chess960, move.from_square, move.to_square, move.promotion, move.drop);
                return move.uci();
            }

            Move parse_uci(const string &uci) {
                /*
                Parses the given move in UCI notation.

                Supports both Chess960 and standard UCI notation.

                The returned move is guaranteed to be either legal or a null move.

                :throws: :exc:`std::invalid_argument` if the move is invalid or illegal in the
                    current position (but not a null move).
                */
                Move move = Move::from_uci(uci);

                if (!*move)
                    return move;

                move = this->_to_chess960(move);
                move = this->_from_chess960(this->chess960, move.from_square, move.to_square, move.promotion, move.drop);

                if (!this->is_legal(move))
                    throw invalid_argument("illegal uci: '" + uci + "' in " + this->fen());

                return move;
            }

            Move push_uci(const string &uci) {
                /*
                Parses a move in UCI notation and puts it on the move stack.

                Returns the move.

                :throws: :exc:`std::invalid_argument` if the move is invalid or illegal in the
                    current position (but not a null move).
                */
                Move move = this->parse_uci(uci);
                this->push(move);
                return move;
            }

            string xboard(Move move, optional<bool> chess960 = nullopt) {
                if (chess960 == nullopt)
                    chess960 = this->chess960;

                if (!*chess960 || !this->is_castling(move))
                    return move.xboard();
                else if (this->is_kingside_castling(move))
                    return "O-O";
                else
                    return "O-O-O";
            }

            Move parse_xboard(const string &xboard) {
                return this->parse_san(xboard);
            }

            Move push_xboard(const string &san) {
                /*
                Parses a move in standard algebraic notation, makes the move and puts
                it onto the move stack.

                Returns the move.

                :throws: :exc:`std::invalid_argument` if neither legal nor a null move.
                */
                Move move = this->parse_san(san);
                this->push(move);
                return move;
            }

            bool is_en_passant(const Move &move) const {
                // Checks if the given pseudo-legal move is an en passant capture.
                return (this->ep_square == move.to_square &&
                        bool(this->pawns & BB_SQUARES[move.from_square]) &&
                        (abs(move.to_square - move.from_square) == 7 || abs(move.to_square - move.from_square) == 9) &&
                        !(this->occupied & BB_SQUARES[move.to_square]));
            }

            bool is_capture(const Move &move) {
                // Checks if the given pseudo-legal move is a capture.
                Bitboard touched = BB_SQUARES[move.from_square] ^ BB_SQUARES[move.to_square];
                return bool(touched & this->occupied_co[!this->turn]) || this->is_en_passant(move);
            }

            bool is_zeroing(const Move &move) {
                // Checks if the given pseudo-legal move is a capture or pawn move.
                Bitboard touched = BB_SQUARES[move.from_square] ^ BB_SQUARES[move.to_square];
                return bool(touched & this->pawns || touched & this->occupied_co[!this->turn] || move.drop == PAWN);
            }

            bool is_irreversible(const Move &move) {
                /*
                Checks if the given pseudo-legal move is irreversible.

                In standard chess, pawn moves, captures, moves that destroy castling
                rights and moves that cede en passant are irreversible.

                This method has false-negatives with forced lines. For example, a check
                that will force the king to lose castling rights is not considered
                irreversible. Only the actual king move is.
                */
                return this->is_zeroing(move) || this->_reduces_castling_rights(move) || this->has_legal_en_passant();
            }

            bool is_castling(const Move &move) const {
                // Checks if the given pseudo-legal move is a castling move.
                if (this->kings & BB_SQUARES[move.from_square]) {
                    int diff = square_file(move.from_square) - square_file(move.to_square);
                    return abs(diff) > 1 || bool(this->rooks & this->occupied_co[this->turn] & BB_SQUARES[move.to_square]);
                }
                return false;
            }

            bool is_kingside_castling(const Move &move) {
                /*
                Checks if the given pseudo-legal move is a kingside castling move.
                */
                return this->is_castling(move) && square_file(move.to_square) > square_file(move.from_square);
            }

            bool is_queenside_castling(const Move &move) {
                /*
                Checks if the given pseudo-legal move is a queenside castling move.
                */
                return this->is_castling(move) && square_file(move.to_square) < square_file(move.from_square);
            }

            Bitboard clean_castling_rights() const {
                /*
                Returns valid castling rights filtered from
                :data:`~chess::Board::castling_rights`.
                */
                if (!this->_stack.empty())
                    // No new castling rights are assigned in a game, so we can assume
                    // they were filtered already.
                    return this->castling_rights;

                Bitboard castling = this->castling_rights & this->rooks;
                Bitboard white_castling = castling & BB_RANK_1 & this->occupied_co[WHITE];
                Bitboard black_castling = castling & BB_RANK_8 & this->occupied_co[BLACK];

                if (!this->chess960) {
                    // The rooks must be on a1, h1, a8 or h8.
                    white_castling &= (BB_A1 | BB_H1);
                    black_castling &= (BB_A8 | BB_H8);

                    // The kings must be on e1 or e8.
                    if (!(this->occupied_co[WHITE] & this->kings & ~this->promoted & BB_E1))
                        white_castling = 0;
                    if (!(this->occupied_co[BLACK] & this->kings & ~this->promoted & BB_E8))
                        black_castling = 0;

                    return white_castling | black_castling;
                } else {
                    // The kings must be on the back rank.
                    Bitboard white_king_mask = this->occupied_co[WHITE] & this->kings & BB_RANK_1 & ~this->promoted;
                    Bitboard black_king_mask = this->occupied_co[BLACK] & this->kings & BB_RANK_8 & ~this->promoted;
                    if (!white_king_mask)
                        white_castling = 0;
                    if (!black_king_mask)
                        black_castling = 0;

                    // There are only two ways of castling, a-side and h-side, and the
                    // king must be between the rooks.
                    Bitboard white_a_side = white_castling & -white_castling;
                    Bitboard white_h_side = white_castling ? BB_SQUARES[msb(white_castling)] : 0;

                    if (white_a_side && msb(white_a_side) > msb(white_king_mask))
                        white_a_side = 0;
                    if (white_h_side && msb(white_h_side) < msb(white_king_mask))
                        white_h_side = 0;

                    Bitboard black_a_side = (black_castling & -black_castling);
                    Bitboard black_h_side = black_castling ? BB_SQUARES[msb(black_castling)] : BB_EMPTY;

                    if (black_a_side && msb(black_a_side) > msb(black_king_mask))
                        black_a_side = 0;
                    if (black_h_side && msb(black_h_side) < msb(black_king_mask))
                        black_h_side = 0;

                    // Done.
                    return black_a_side | black_h_side | white_a_side | white_h_side;
                }
            }

            bool has_castling_rights(Color color) {
                // Checks if the given side has castling rights.
                Bitboard backrank = color == WHITE ? BB_RANK_1 : BB_RANK_8;
                return bool(this->clean_castling_rights() & backrank);
            }

            bool has_kingside_castling_rights(Color color) {
                /*
                Checks if the given side has kingside (that is h-side in Chess960)
                castling rights.
                */
                Bitboard backrank = color == WHITE ? BB_RANK_1 : BB_RANK_8;
                Bitboard king_mask = this->kings & this->occupied_co[color] & backrank & ~this->promoted;
                if (!king_mask)
                    return false;

                Bitboard castling_rights = this->clean_castling_rights() & backrank;
                while (castling_rights) {
                    Bitboard rook = castling_rights & -castling_rights;

                    if (rook > king_mask)
                        return true;

                    castling_rights = castling_rights & (castling_rights - 1);
                }

                return false;
            }

            bool has_queenside_castling_rights(Color color) {
                /*
                Checks if the given side has queenside (that is h-side in Chess960)
                castling rights.
                */
                Bitboard backrank = color == WHITE ? BB_RANK_1 : BB_RANK_8;
                Bitboard king_mask = this->kings & this->occupied_co[color] & backrank & ~this->promoted;
                if (!king_mask)
                    return false;

                Bitboard castling_rights = this->clean_castling_rights() & backrank;
                while (castling_rights) {
                    Bitboard rook = castling_rights & -castling_rights;

                    if (rook < king_mask)
                        return true;

                    castling_rights = castling_rights & (castling_rights - 1);
                }

                return false;
            }

            bool has_chess960_castling_rights() {
                /*
                Checks if there are castling rights that are only possible in Chess960.
                */
                // Get valid Chess960 castling rights.
                bool chess960 = this->chess960;
                this->chess960 = true;
                castling_rights = this->clean_castling_rights();
                this->chess960 = chess960;

                // Standard chess castling rights can only be on the standard
                // starting rook squares.
                if (castling_rights & ~BB_CORNERS)
                    return true;

                // If there are any castling rights in standard chess, the king must be
                // on e1 or e8.
                if (castling_rights & BB_RANK_1 && !(this->occupied_co[WHITE] & this->kings & BB_E1))
                    return true;
                if (castling_rights & BB_RANK_8 && !(this->occupied_co[BLACK] & this->kings & BB_E8))
                    return true;

                return false;
            }

            Status status() {
                /*
                Gets a bitmask of possible problems with the position.

                :data:`~chess::STATUS_VALID` if all basic validity requirements are met.
                This does not imply that the position is actually reachable with a
                series of legal moves from the starting position.

                Otherwise, bitwise combinations of:
                :data:`~chess::STATUS_NO_WHITE_KING`,
                :data:`~chess::STATUS_NO_BLACK_KING`,
                :data:`~chess::STATUS_TOO_MANY_KINGS`,
                :data:`~chess::STATUS_TOO_MANY_WHITE_PAWNS`,
                :data:`~chess::STATUS_TOO_MANY_BLACK_PAWNS`,
                :data:`~chess::STATUS_PAWNS_ON_BACKRANK`,
                :data:`~chess::STATUS_TOO_MANY_WHITE_PIECES`,
                :data:`~chess::STATUS_TOO_MANY_BLACK_PIECES`,
                :data:`~chess::STATUS_BAD_CASTLING_RIGHTS`,
                :data:`~chess::STATUS_INVALID_EP_SQUARE`,
                :data:`~chess::STATUS_OPPOSITE_CHECK`,
                :data:`~chess::STATUS_EMPTY`,
                :data:`~chess::STATUS_RACE_CHECK`,
                :data:`~chess::STATUS_RACE_OVER`,
                :data:`~chess::STATUS_RACE_MATERIAL`,
                :data:`~chess::STATUS_TOO_MANY_CHECKERS`,
                :data:`~chess::STATUS_IMPOSSIBLE_CHECK`.
                */
                int errors = int(STATUS_VALID);

                // There must be at least one piece.
                if (!this->occupied)
                    errors |= int(STATUS_EMPTY);

                // There must be exactly one king of each color.
                if (!(this->occupied_co[WHITE] & this->kings))
                    errors |= int(STATUS_NO_WHITE_KING);
                if (!(this->occupied_co[BLACK] & this->kings))
                    errors |= int(STATUS_NO_BLACK_KING);
                if (popcount(this->occupied & this->kings) > 2)
                    errors |= int(STATUS_TOO_MANY_KINGS);

                // There can not be more than 16 pieces of any color.
                if (popcount(this->occupied_co[WHITE]) > 16)
                    errors |= int(STATUS_TOO_MANY_WHITE_PIECES);
                if (popcount(this->occupied_co[BLACK]) > 16)
                    errors |= int(STATUS_TOO_MANY_BLACK_PIECES);

                // There can not be more than 8 pawns of any color.
                if (popcount(this->occupied_co[WHITE] & this->pawns) > 8)
                    errors |= int(STATUS_TOO_MANY_WHITE_PAWNS);
                if (popcount(this->occupied_co[BLACK] & this->pawns) > 8)
                    errors |= int(STATUS_TOO_MANY_BLACK_PAWNS);

                // Pawns can not be on the back rank.
                if (this->pawns & BB_BACKRANKS)
                    errors |= int(STATUS_PAWNS_ON_BACKRANK);

                // Castling rights.
                if (this->castling_rights != this->clean_castling_rights())
                    errors |= int(STATUS_BAD_CASTLING_RIGHTS);

                // En passant.
                optional<Square> valid_ep_square = this->_valid_ep_square();
                if (bool(this->ep_square) != bool(valid_ep_square) || *this->ep_square != *valid_ep_square)
                    errors |= int(STATUS_INVALID_EP_SQUARE);

                // Side to move giving check.
                if (this->was_into_check())
                    errors |= int(STATUS_OPPOSITE_CHECK);

                // More than the maximum number of possible checkers in the variant.
                Bitboard checkers = this->checkers_mask();
                Bitboard our_kings = this->kings & this->occupied_co[this->turn] & ~this->promoted;
                if (popcount(checkers) > 2)
                    errors |= int(STATUS_TOO_MANY_CHECKERS);
                else if (popcount(checkers) == 2 && ray(lsb(checkers), msb(checkers)) & our_kings)
                    errors |= int(STATUS_IMPOSSIBLE_CHECK);
                else if (valid_ep_square != nullopt) {
                    for (Square checker : scan_reversed(checkers)) {
                        if (ray(checker, *valid_ep_square) & our_kings) {
                            errors |= int(STATUS_IMPOSSIBLE_CHECK);
                            break;
                        }
                    }
                }

                return Status(errors);
            }

            bool is_valid() {
                /*
                Checks some basic validity requirements.

                See :func:`~chess.Board.status()` for details.
                */
                return this->status() == STATUS_VALID;
            }

            vector<Move> generate_legal_moves(Bitboard from_mask = BB_ALL, Bitboard to_mask = BB_ALL) const {
                vector<Move> iter;
                if (this->is_variant_end())
                    return iter;

                Bitboard king_mask = this->kings & this->occupied_co[this->turn];
                if (king_mask) {
                    Square king = msb(king_mask);
                    Bitboard blockers = this->_slider_blockers(king);
                    Bitboard checkers = this->attackers_mask(!this->turn, king);
                    if (checkers) {
                        for (const Move &move : this->_generate_evasions(king, checkers, from_mask, to_mask)) {
                            if (this->_is_safe(king, blockers, move))
                                iter.push_back(move);
                            else
                            {
                                for (const Move &move : this->generate_pseudo_legal_moves(from_mask, to_mask)) {
                                    if (this->_is_safe(king, blockers, move))
                                        iter.push_back(move);
                                }
                            }
                        }
                    }
                } else {
                    for (const Move &move : this->generate_pseudo_legal_moves(from_mask, to_mask))
                        iter.push_back(move);
                }
                return iter;
            }

            vector<Move> generate_legal_ep(Bitboard from_mask = BB_ALL, Bitboard to_mask = BB_ALL) const {
                vector<Move> iter;
                if (this->is_variant_end())
                    return iter;

                for (const Move &move : this->generate_pseudo_legal_ep(from_mask, to_mask)) {
                    if (!this->is_into_check(move))
                        iter.push_back(move);
                }
                return iter;
            }

            vector<Move> generate_legal_captures(Bitboard from_mask = BB_ALL, Bitboard to_mask = BB_ALL) {
                vector<Move> iter;
                for (const Move &move : this->generate_legal_moves(from_mask, to_mask & this->occupied_co[!this->turn]))
                    iter.push_back(move);
                for (const Move &move : this->generate_legal_ep(from_mask, to_mask))
                    iter.push_back(move);
                return iter;
            }

            vector<Move> generate_castling_moves(Bitboard from_mask = BB_ALL, Bitboard to_mask = BB_ALL) {
                vector<Move> iter;
                if (this->is_variant_end())
                    return iter;

                Bitboard backrank = this->turn == WHITE ? BB_RANK_1 : BB_RANK_8;
                Bitboard king = this->occupied_co[this->turn] & this->kings & ~this->promoted & backrank & from_mask;
                king = king & -king;
                if (!king)
                    return iter;

                Bitboard bb_c = BB_FILE_C & backrank;
                Bitboard bb_d = BB_FILE_D & backrank;
                Bitboard bb_f = BB_FILE_F & backrank;
                Bitboard bb_g = BB_FILE_G & backrank;

                for (Square candidate : scan_reversed(this->clean_castling_rights() & backrank & to_mask)) {
                    Bitboard rook = BB_SQUARES[candidate];

                    bool a_side = rook < king;
                    Bitboard king_to = a_side ? bb_c : bb_g;
                    Bitboard rook_to = a_side ? bb_d : bb_f;

                    Bitboard king_path = between(msb(king), msb(king_to));
                    Bitboard rook_path = between(candidate, msb(rook_to));

                    if (!((this->occupied ^ king ^ rook) & (king_path | rook_path | king_to | rook_to) ||
                        this->_attacked_for_king(king_path | king, this->occupied ^ king) ||
                        this->_attacked_for_king(king_to, this->occupied ^ king ^ rook ^ rook_to)))
                        iter.push_back(this->_from_chess960(this->chess960, msb(king), candidate));
                }
                return iter;
            }

            void apply_transform(function<Bitboard(Bitboard)> f) {
                BaseBoard::apply_transform(f);
                this->clear_stack();
                this->ep_square = this->ep_square == nullopt ? nullopt : optional(msb(f(BB_SQUARES[*this->ep_square])));
                this->castling_rights = f(this->castling_rights);
            }

            BoardTtransform(function<Bitboard(Bitboard)> f) {
                BoardTboard = this->copy(false);
                board->apply_transform(f);
                return board;
            }

            void apply_mirror() {
                BaseBoard::apply_mirror();
                this->turn = !this->turn;
            }

            BoardTmirror() {
                /*
                Returns a mirrored copy of the board.

                The board is mirrored vertically and piece colors are swapped, so that
                the position is equivalent modulo color. Also swap the "en passant"
                square, castling rights and turn.

                Alternatively, :func:`~chess::Board::apply_mirror()` can be used
                to mirror the board.
                */
                BoardTboard = this->copy();
                board->apply_mirror();
                return board;
            }

            BoardTcopy(variant<bool, int> stack = true) {
                /*
                Creates a copy of the board.

                Defaults to copying the entire move stack. Alternatively, *stack* can
                be ``false``, or an integer to copy a limited number of moves.
                */
                BoardTboard = BaseBoard::copy();

                board->chess960 = this->chess960;

                board->ep_square = this->ep_square;
                board->castling_rights = this->castling_rights;
                board->turn = this->turn;
                board->fullmove_number = this->fullmove_number;
                board->halfmove_clock = this->halfmove_clock;

                if (holds_alternative<bool>(stack) && get<bool>(stack) || holds_alternative<int>(stack) && get<int>(stack)) {
                    stack = int(holds_alternative<bool>(stack) && get<bool>(stack) == true ? this->move_stack.size() : get<int>(stack));
                    vector<Move> move_stack;
                    for (auto it = thisend(->move_stack) - get<int>(stack); it != this->end(move_stack); ++it) {
                        Move copy = **it;
                        move_stack.push_back(copy);
                    }
                    board->move_stack = move_stack;
                    board->_stack = vector<Move>(thisend(->_stack) - get<int>(stack), this->end(_stack));
                }

                return board;
            }

            static BoardTempty(bool chess960 = false, ...) {
                // Creates a new empty board. Also see :func:`~chess::Board::clear()`.
                return BoardT(nullopt, chess960);
            }

            static tuple<Board , unordered_map<string, optional<variant<string, int, float, Move , vector<Move>>>>> from_epd(const string &epd, bool chess960 = false, ...) {
                /*
                Creates a new board from an EPD string. See
                :func:`~chess::Board::set_epd()`.

                Returns the board and the dictionary of parsed operations as a tuple.
                */
                BoardTboard = Board::empty(chess960);
                return {board, board->set_epd(epd)};
            }

            static BoardTfrom_chess960_pos(int scharnagl) {
                BoardTboard = Board::empty(true);
                board->set_chess960_pos(scharnagl);
                return board;
            }

        private:
            vector<_BoardState> _stack;

            bool _is_halfmoves(int n) const {
                return this->halfmove_clock >= n && !this->generate_legal_moves().empty();
            }

            _BoardState _board_state() const {
                return _BoardState(*this);
            }

            void _push_capture(const Move &move, Square capture_square, PieceType piece_type, bool was_promoted) const {

            }

            void _set_castling_fen(const string &castling_fen) const {
                if (castling_fen.empty() || castling_fen == "-") {
                    this->castling_rights = BB_EMPTY;
                    return;
                }

                if (!regex_match(castling_fen, FEN_CASTLING_REGEX)) {
                    throw invalid_argument("invalid castling fen: '" + castling_fen + "'");
                }

                this->castling_rights = BB_EMPTY;

                for (char flag : castling_fen) {
                    Color color = isupper(flag) ? WHITE : BLACK;
                    flag = tolower(flag);
                    Bitboard backrank = color == WHITE ? BB_RANK_1 : BB_RANK_8;
                    Bitboard rooks = this->occupied_co[color] & this->rooks & backrank;
                    optional<Square> king = this->king(color);

                    if (flag == 'q') {
                        // Select the leftmost rook.
                        if (king != nullopt && lsb(rooks) < *king) {
                            this->castling_rights |= rooks & -rooks;
                        } else {
                            this->castling_rights |= BB_FILE_A & backrank;
                        }
                    } else if (flag == 'k') {
                        // Select the rightmost rook.
                        int rook = msb(rooks);
                        if (king != nullopt && *king < rook) {
                            this->castling_rights |= BB_SQUARES[rook];
                        } else {
                            this->castling_rights |= BB_FILE_H & backrank;
                        }
                    } else {
                        auto it = find(begin(FILE_NAMES), end(FILE_NAMES), flag);
                        if (it == end(FILE_NAMES)) {
                            throw invalid_argument("");
                        }
                        this->castling_rights |= BB_FILES[distance(FILE_NAMES, it)] & backrank;
                    }
                }
            }

            string _epd_operations(const unordered_map<string, optional<variant<string, int, float, Move , vector<Move>>>> &operations) const {
                vector<char> epd;
                bool first_op = true;

                for (auto [opcode, operand] : operations) {
                    if (opcode == "-") {
                        throw "dash (-) is not a valid epd opcode";
                    }
                    for (char blacklisted : {' ', '\n', '\t', '\r'}) {
                        if (opcode.find(blacklisted) != string::npos) {
                            throw "invalid character ' ' in epd opcode: '" + opcode + "'";
                        }
                    }

                    if (!first_op) {
                        epd.push_back(' ');
                    }
                    first_op = false;
                    epd.insert(end(epd), begin(opcode), end(opcode));

                    if (operand == nullopt) {
                        epd.push_back(';');
                    } else if (holds_alternative<Move>(*operand)) {
                        epd.push_back(' ');
                        string san = this->san(get<Move>(*operand));
                        epd.insert(end(epd), begin(san), end(san));
                        epd.push_back(';');
                    } else if (holds_alternative<int>(*operand)) {
                        string s = " " + to_string(get<int>(*operand)) + ";";
                        epd.insert(end(epd), begin(s), end(s));
                    } else if (holds_alternative<float>(*operand)) {
                        if (!isfinite(get<float>(*operand))) {
                            throw "expected numeric epd operand to be finite, got: " + to_string(get<float>(*operand));
                        }
                        string s = " " + to_string(get<float>(*operand)) + ";";
                        epd.insert(end(epd), begin(s), end(s));
                    } else if (opcode == "pv" && holds_alternative<vector<Move>>(*operand)) {
                        BoardT position = this->copy(false);
                        for (const Move &move : get<vector<Move>>(*operand)) {
                            epd.push_back(' ');
                            string s = position->san_and_push(move);
                            epd.insert(end(epd), begin(s), end(s));
                        }
                        epd.push_back(';');
                    } else if ((opcode == "am" || opcode == "bm") && holds_alternative<vector<Move>>(*operand)) {
                        vector<string> v;
                        for (const Move &move : get<vector<Move>>(*operand)) {
                            v.push_back(this->san(move));
                        }
                        sort(begin(v), end(v));
                        for (const string &san : v) {
                            epd.push_back(' ');
                            epd.insert(end(epd), begin(san), end(san));
                        }
                        epd.push_back(';');
                    } else {
                        // Append as escaped string.
                        string s = " \"";
                        epd.insert(end(epd), begin(s), end(s));
                        s = regex_replace(regex_replace(regex_replace(regex_replace(regex_replace(get<string>(*operand), regex("\\"), "\\\\"), regex("\t"), "\\t"), regex("\r"), "\\r"), regex("\n"), "\\n"), regex("\""), "\\\"");
                        epd.insert(end(epd), begin(s), end(s));
                        s = "\";";
                        epd.insert(end(epd), begin(s), end(s));
                    }
                }

                return string(begin(epd), end(epd));
            }

            unordered_map<string, optional<variant<string, int, float, Move , vector<Move>>>> _parse_epd_ops(const string &operation_part, function<Board()> make_board) const {
                unordered_map<string, optional<variant<string, int, float, Move , vector<Move>>>> operations;
                string state = "opcode";
                string opcode = "";
                string operand = "";
                optional<BoardT> position = nullopt;

                vector<optional<char>> v(begin(operation_part), end(operation_part));
                v.push_back(nullopt);
                for (optional<char> ch : operation_part) {
                    if (state == "opcode") {
                        if (ch && (*ch == ' ' || *ch == '\t' || *ch == '\r' || *ch == '\n')) {
                            if (opcode == "-") {
                                opcode = "";
                            } else if (!opcode.empty()) {
                                state = "after_opcode";
                            }
                        } else if (ch == nullopt || *ch == ';') {
                            if (opcode == "-") {
                                opcode = "";
                             } else if (!opcode.empty()) {
                                operations[opcode] = opcode == "pv" || opcode == "am" || opcode == "bm" ? optional<vector<Move>>() : nullopt;
                                opcode = "";
                            }
                        } else {
                            opcode += *ch;
                        }
                    } else if (state == "after_opcode") {
                        if (ch && (*ch == ' ' || *ch == '\t' || *ch == '\r' || *ch == '\n')) {

                        } else if (*ch == '\"') {
                            state = "string";
                        } else if (ch == nullopt || *ch == ';') {
                            if (!opcode.empty()) {
                                operations[opcode] = opcode == "pv" || opcode == "am" || opcode == "bm" ? optional<vector<Move>>() : nullopt;
                                opcode = "";
                            }
                            state = "opcode";
                        } else if (*ch == '+' || *ch == '-' || *ch == '.' || isdigit(*ch)) {
                            operand = *ch;
                            state = "numeric";
                        } else {
                            operand = *ch;
                            state = "san";
                        }
                    } else if (state == "numeric") {
                        if (ch == nullopt || *ch == ';') {
                            if (operand.find('.') != string::npos || operand.find('e') != string::npos || operand.find('E') != string::npos) {
                                float parsed = stof(operand);
                                if (!isfinite(parsed)) {
                                    throw invalid_argument("invalid numeric operand for epd operation '" + opcode + "': '" + operand + "'");
                                }
                                operations[opcode] = parsed;
                            } else {
                                operations[opcode] = stoi(operand);
                            }
                            opcode = "";
                            operand = "";
                            state = "opcode";
                        } else {
                            operand += *ch;
                        }
                    } else if (state == "string") {
                        if (ch == nullopt || *ch == '\"') {
                            operations[opcode] = operand;
                            opcode = "";
                            operand = "";
                            state = "opcode";
                        } else if (*ch == '\\') {
                            state = "string_escape";
                        } else {
                            operand += *ch;
                        }
                    } else if (state == "string_escape") {
                        if (ch == nullopt) {
                            operations[opcode] = operand;
                            opcode = "";
                            operand = "";
                            state = "opcode";
                        } else if (*ch == 'r') {
                            operand += "\r";
                            state = "string";
                        } else if (*ch == 'n') {
                            operand += "\n";
                            state = "string";
                        } else if (*ch == 't') {
                            operand += "\t";
                            state = "string";
                        } else {
                            operand += *ch;
                            state = "string";
                        }
                    } else if (state == "san") {
                        if (ch == nullopt || *ch == ';') {
                            if (position == nullopt) {
                                position = make_board();
                            }

                            if (opcode == "pv") {
                                // A variation.
                                vector<Move> variation;
                                istringstream iss(operand);
                                vector<string> split = {istream_iterator<string>(iss), {}};
                                for (const string &token : split) {
                                    Move move = position->parse_xboard(token);
                                    variation.push_back(move);
                                    position->push(move);
                                }

                                // Reset the position.
                                while (!position->move_stack.empty()) {
                                    position->pop();
                                }

                                operations[opcode] = variation;
                            } else if (opcode == "bm" || opcode == "am") {
                                // A set of moves.
                                istringstream iss(operand);
                                vector<string> split = {istream_iterator<string>(iss), {}};
                                vector<Move> parsed;
                                for (const string &token : split) {
                                    parsed.push_back(position->parse_xboard(token));
                                }
                                operations[opcode] = parsed;
                            } else {
                                // A single move.
                                operations[opcode] = position->parse_xboard(operand);
                            }

                            opcode = "";
                            operand = "";
                            state = "opcode";
                        } else {
                            operand += *ch;
                        }
                    }
                }

                if (state != "opcode") {
                    throw;
                }
                return operations;
            }

            string _algebraic(const Move &move, bool long_ = false) const {
                string san = this->_algebraic_and_push(move, long_);
                this->pop();
                return san;
            }

            string _algebraic_and_push(const Move &move, bool long_ = false) const {
                string san = this->_algebraic_without_suffix(move, long_);

                // Look ahead for check or checkmate.
                this->push(move);
                bool is_check = this->is_check();
                bool is_checkmate = (is_check && this->is_checkmate()) || this->is_variant_loss() || this->is_variant_win();

                // Add check or checkmate suffix.
                if (is_checkmate && move) {
                    return san + "#";
                } else if (is_check && move) {
                    return san + "+";
                } else {
                    return san;
                }
            }

            string _algebraic_without_suffix(Move move, bool long_ = false) const {
                // Null move.
                if (!move) {
                    return "--";
                }

                // Drops.
                if (move.drop) {
                    string san = "";
                    if (*move.drop != PAWN) {
                        san = toupper(piece_symbol(*move.drop));
                    }
                    san += "@" + SQUARE_NAMES[move.to_square];
                    return san;
                }

                // Castling.
                if (this->is_castling(move)) {
                    if (square_file(move.to_square) < square_file(move.from_square)) {
                        return "O-O-O";
                    } else {
                        return "O-O";
                    }
                }

                optional<PieceType> piece_type = this->piece_type_at(move.from_square);
                if (!piece_type) {
                    throw "san() and lan() expect move to be legal or null, but got " + string(move) + " in " + this->fen();
                }
                bool capture = this->is_capture(move);

                string san;
                if (*piece_type != PAWN) {
                    san = toupper(piece_symbol(*piece_type));
                }

                if (long_) {
                    san += SQUARE_NAMES[move.from_square];
                } else if (*piece_type != PAWN) {
                    // Get ambiguous move candidates.
                    // Relevant candidates: not exactly the current move,
                    // but to the same square.
                    Bitboard others = 0;
                    Bitboard from_mask = this->pieces_mask(*piece_type, this->turn);
                    from_mask &= ~BB_SQUARES[move.from_square];
                    Bitboard to_mask = BB_SQUARES[move.to_square];
                    for (const Move &candidate : this->generate_legal_moves(from_mask, to_mask)) {
                        others |= BB_SQUARES[candidate.from_square];
                    }

                    // Disambiguate.
                    if (others) {
                        bool row = false, column = false;

                        if (others & BB_RANKS[square_rank(move.from_square)]) {
                            column = true;
                        }

                        if (others & BB_FILES[square_file(move.from_square)]) {
                            row = true;
                        } else {
                            column = true;
                        }

                        if (column) {
                            san += FILE_NAMES[square_file(move.from_square)];
                        }
                        if (row) {
                            san += RANK_NAMES[square_rank(move.from_square)];
                        }
                    }
                } else if (capture) {
                    san += FILE_NAMES[square_file(move.from_square)];
                }

                // Captures.
                if (capture) {
                    san += "x";
                } else if (long_) {
                    san += "-";
                }

                // Destination square.
                san += SQUARE_NAMES[move.to_square];

                // Promotion.
                if (move.promotion) {
                    san += "=" + toupper(piece_symbol(*move.promotion));
                }

                return san;
            }

            bool _reduces_castling_rights(const Move &move) {
                Bitboard cr = this->clean_castling_rights();
                Bitboard touched = BB_SQUARES[move.from_square] ^ BB_SQUARES[move.to_square];
                return bool(touched & cr ||
                            cr & BB_RANK_1 && touched & this->kings & this->occupied_co[WHITE] & ~this->promoted ||
                            cr & BB_RANK_8 && touched & this->kings & this->occupied_co[BLACK] & ~this->promoted);
            }

            optional<Square> _valid_ep_square() {
                if (!this->ep_square)
                    return nullopt;

                int ep_rank;
                Bitboard pawn_mask, seventh_rank_mask;
                if (this->turn == WHITE) {
                    ep_rank = 5;
                    pawn_mask = shift_down(BB_SQUARES[*this->ep_square]);
                    seventh_rank_mask = shift_up(BB_SQUARES[*this->ep_square]);
                } else {
                    ep_rank = 2;
                    pawn_mask = shift_up(BB_SQUARES[*this->ep_square]);
                    seventh_rank_mask = shift_down(BB_SQUARES[*this->ep_square]);
                }

                // The en passant square must be on the third or sixth rank.
                if (square_rank(*this->ep_square) != ep_rank)
                    return nullopt;

                // The last move must have been a double pawn push, so there must
                // be a pawn of the correct color on the fourth or fifth rank.
                if (!(this->pawns & this->occupied_co[!this->turn] & pawn_mask))
                    return nullopt;

                // And the en passant square must be empty.
                if (this->occupied & BB_SQUARES[*this->ep_square])
                    return nullopt;

                // And the second rank must be empty.
                if (this->occupied & seventh_rank_mask)
                    return nullopt;

                return this->ep_square;
            }

            bool _ep_skewered(Square king, Square capturer) const {
                // Handle the special case where the king would be in check if the
                // pawn and its capturer disappear from the rank.

                // Vertical skewers of the captured pawn are not possible. (Pins on
                // the capturer are not handled here.)
                if (this->ep_square == nullopt)
                    throw;

                Square last_double = *this->ep_square + (this->turn == WHITE ? -8 : 8);

                Bitboard occupancy = (this->occupied & ~BB_SQUARES[last_double] &
                                        ~BB_SQUARES[capturer] |
                                    BB_SQUARES[*this->ep_square]);

                // Horizontal attack on the fifth or fourth rank.
                Bitboard horizontal_attackers = this->occupied_co[!this->turn] & (this->rooks | this->queens);
                if (BB_RANK_ATTACKS[king].at(BB_RANK_MASKS[king] & occupancy) & horizontal_attackers)
                    return true;

                // Diagonal skewers. These are not actually possible in a real game,
                // because if the latest double pawn move covers a diagonal attack,
                // then the other side would have been in check already.
                Bitboard diagonal_attackers = this->occupied_co[!this->turn] & (this->bishops | this->queens);
                if (BB_DIAG_ATTACKS[king].at(BB_DIAG_MASKS[king] & occupancy) & diagonal_attackers)
                    return true;

                return false;
            }

            Bitboard _slider_blockers(Square king) const {
                Bitboard rooks_and_queens = this->rooks | this->queens;
                Bitboard bishops_and_queens = this->bishops | this->queens;

                Bitboard snipers = ((BB_RANK_ATTACKS[king].at(0) & rooks_and_queens) |
                                    (BB_FILE_ATTACKS[king].at(0) & rooks_and_queens) |
                                    (BB_DIAG_ATTACKS[king].at(0) & bishops_and_queens));

                Bitboard blockers = 0;

                for (Square sniper : scan_reversed(snipers & this->occupied_co[!this->turn])) {
                    Bitboard b = between(king, sniper) & this->occupied;

                    // Add to blockers if exactly one piece in-between.
                    if (b && BB_SQUARES[msb(b)] == b)
                        blockers |= b;
                }

                return blockers & this->occupied_co[this->turn];
            }

            bool _is_safe(Square king, Bitboard blockers, const Move &move) const {
                if (move.from_square == king) {
                    if (this->is_castling(move))
                        return true;
                    else
                        return !this->is_attacked_by(!this->turn, move.to_square);
                } else if (this->is_en_passant(move))
                    return bool(this->pin_mask(this->turn, move.from_square) & BB_SQUARES[move.to_square] &&
                                !this->_ep_skewered(king, move.from_square));
                else
                    return bool(!blockers & BB_SQUARES[move.from_square] ||
                                ray(move.from_square, move.to_square) & BB_SQUARES[king]);
            }

            vector<Move> _generate_evasions(Square king, Bitboard checkers, Bitboard from_mask = BB_ALL, Bitboard to_mask = BB_ALL) const {
                vector<Move> iter;
                Bitboard sliders = checkers & (this->bishops | this->rooks | this->queens);

                Bitboard attacked = 0;
                for (Square checker : scan_reversed(sliders))
                    attacked |= ray(king, checker) & ~BB_SQUARES[checker];

                if (BB_SQUARES[king] & from_mask) {
                    for (Square to_square : scan_reversed(BB_KING_ATTACKS[king] & ~this->occupied_co[this->turn] & ~attacked & to_mask))
                        iter.push_back(Move(king, to_square));
                }

                Square checker = msb(checkers);
                if (BB_SQUARES[checker] == checkers) {
                    // Capture or block a single checker.
                    Bitboard target = between(king, checker) | checkers;

                    for (const Move &move : this->generate_pseudo_legal_moves(~this->kings & from_mask, target & to_mask))
                        iter.push_back(move);

                    // Capture the checking pawn en passant (but avoid yielding
                    // duplicate moves).
                    if (this->ep_square && !(BB_SQUARES[*this->ep_square] & target)) {
                        Square last_double = *this->ep_square + (this->turn == WHITE ? -8 : 8);
                        if (last_double == checker) {
                            for (const Move &move : this->generate_pseudo_legal_ep(from_mask, to_mask))
                                iter.push_back(move);
                        }
                    }
                }
                return iter;
            }

            bool _attacked_for_king(Bitboard path, Bitboard occupied) {
                for (Square sq : scan_reversed(path))
                    if (this->_attackers_mask(!this->turn, sq, occupied))
                        return true;
                return false;
            }

            Move _from_chess960(bool chess960, Square from_square, Square to_square, optional<PieceType> promotion = nullopt, optional<PieceType> drop = nullopt) {
                if (!chess960 && promotion == nullopt && drop == nullopt) {
                    if (from_square == E1 && this->kings & BB_E1) {
                        if (to_square == H1)
                            return Move(E1, G1);
                        else if (to_square == A1)
                            return Move(E1, C1);
                    } else if (from_square == E8 && this->kings & BB_E8) {
                        if (to_square == H8)
                            return Move(E8, G8);
                        else if (to_square == A8)
                            return Move(E8, C8);
                    }
                }

                return Move(from_square, to_square, promotion, drop);
            }

            Move _to_chess960(const Move &move) {
                if (move.from_square == E1 && this->kings & BB_E1) {
                    if (move.to_square == G1 && !(this->rooks & BB_G1))
                        return Move(E1, H1);
                    else if (move.to_square == C1 && !(this->rooks & BB_C1))
                        return Move(E1, A1);
                } else if (move.from_square == E8 && this->kings & BB_E8) {
                    if (move.to_square == G8 && !(this->rooks & BB_G8))
                        return Move(E8, H8);
                    else if (move.to_square == C8 && !(this->rooks & BB_C8))
                        return Move(E8, A8);
                }

                return move;
            }

            tuple<Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Color, Bitboard, optional<Square>> _transposition_key() const {
                return {this->pawns, this->knights, this->bishops, this->rooks,
                        this->queens, this->kings,
                        this->occupied_co[WHITE], this->occupied_co[BLACK],
                        this->turn, this->clean_castling_rights(),
                        this->has_legal_en_passant() ? this->ep_square : nullopt};
            }

            bool operator==(const Board &board) {
                return (
                    this->halfmove_clock == board.halfmove_clock &&
                    this->fullmove_number == board.fullmove_number &&
                    this->_transposition_key() == board._transposition_key());
            }
        };

        ostream &operator<<(ostream &os, const Board &board) {
            if (!board.chess960) {
                os << "Board('" << board.fen() << "')";
            } else {
                os << "Board('" << board.fen() << "', chess960=true)";
            }
            return os;
        }

        class PseudoLegalMoveGenerator {

        public:
            Board board;

            PseudoLegalMoveGenerator(Board board) {
                this->board = board;
            }

            operator bool() {
                return !this->board.generate_pseudo_legal_moves().empty();
            }

            int count() {
                return this->board.generate_pseudo_legal_moves().size();
            }

            auto begin() const {
                return std::begin(this->board.generate_pseudo_legal_moves());
            }

            auto end() const {
                return std::end(this->board.generate_pseudo_legal_moves());
            }
        };

        ostream &operator<<(ostream &os, const PseudoLegalMoveGenerator &pseudo_legal_move_generator) {
            vector<string> builder;

            for (const Move &move : pseudo_legal_move_generator) {
                if (pseudo_legal_move_generator.board.is_legal(move)) {
                    builder.push_back(pseudo_legal_move_generator.board.san(move));
                } else {
                    builder.push_back(pseudo_legal_move_generator.board.uci(move));
                }
            }

            string sans;
            for (const string &s : builder) {
                sans += s + ", ";
            }
            sans.resize(sans.size() - 2);
            os << "<PseudoLegalMoveGenerator at " << &pseudo_legal_move_generator << " (" << sans << ")>";
            return os;
        }

        class LegalMoveGenerator {

        public:
            Board board;

            LegalMoveGenerator(Board board) {
                this->board = board;
            }

            operator bool() {
                return !this->board->generate_legal_moves().empty();
            }

            int count() {
                return this->board->generate_legal_moves().size();
            }

            auto begin() const {
                return std::begin(this->board.generate_legal_moves());
            }

            auto end() const {
                return std::end(this->board.generate_legal_moves());
            }
        };

        ostream &operator<<(ostream &os, const LegalMoveGenerator &legal_move_generator) {
            string sans;
            for (const Move &move : legal_move_generator) {
                sans += legal_move_generator.board.san(move) + ", ";
            }
            sans.resize(sans.size() - 2);
            os << "<LegalMoveGenerator at " << &legal_move_generator << " (" << sans << ")>";
            return os;
        }

        typedef variant<long, vector<Square>> IntoSquareSet;

        class SquareSet {
            /*
            A set of squares.

            Square sets are internally represented by 64-bit integer masks of the
            included squares. Bitwise operations can be used to compute unions,
            intersections and shifts.

            >>> #include "chess.cpp"
            >>>
            >>> chess::SquareSet squares = chess::SquareSet({chess::A8, chess::A1});
            >>> std::cout << squares;
            SquareSet(0x0100_0000_0000_0001)

            >>> squares = chess::SquareSet(chess::BB_A8 | chess::BB_RANK_1);
            >>> std::cout << squares;
            SquareSet(0x0100_0000_0000_00ff)

            >>> std::cout << string(squares);
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

            >>> std::cout << std::find(begin(squares), end(squares), chess::B1) != end(squares);;
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
            ...     std::cout << string(square);
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

            >>> std::cout << long(squares);
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
            Bitboard mask;

            SquareSet(IntoSquareSet squares = BB_EMPTY) {
                try {
                    this->mask = get<int>(squares) & BB_ALL; // type: ignore
                    return;
                } catch (bad_variant_access) {
                    this->mask = 0;
                }

                // Try squares as an iterable. Not under except clause for nicer
                // backtraces.
                for (Square square : get<vector<Square>>(squares)) // type: ignore
                    this->add(square);
            }

            // Set

            vector<Square> iter() {
                return scan_forward(this->mask);
            }

            // MutableSet

            void add(Square square) {
                // Adds a square to the set.
                this->mask |= BB_SQUARES[square];
            }

            void discard(Square square) {
                // Discards a square from the set.
                this->mask &= ~BB_SQUARES[square];
            }

            // frozenset

            bool isdisjoint(IntoSquareSet other) {
                // Tests if the square sets are disjoint.
                return !bool(*this & other);
            }

            bool issubset(IntoSquareSet other) {
                // Tests if this square set is a subset of another.
                return !bool(~*this & other);
            }

            bool issuperset(IntoSquareSet other) {
                // Tests if this square set is a superset of another.
                return !bool(*this & ~SquareSet(other));
            }

            SquareSet *union_(IntoSquareSet other) {
                return *this | other;
            }

            SquareSet operator|(IntoSquareSet other) {
                SquareSet r = SquareSet(other);
                r.mask |= this->mask;
                return r;
            }

            SquareSet intersection(IntoSquareSet other) {
                return *this & other;
            }
        };

        ostream &operator<<(ostream &os, const SquareSet &square_set) {
            stringstream ss;
            ss << hex << square_set.mask;
            string s = ss.str();
            s = string(16 - s.length(), '0') + s;
            for (int i = 4; i < 16; i += 5) {
                s.insert(i, "'");
            }
            os << "SquareSet(" << "0x" << s << ")";
            return os;
        }
    }

    template <> struct hash<chess::Piece> {
        int operator()(const chess::Piece &piece) const {
            return piece.piece_type + (piece.color ? -1 : 5);
        }
    };
}

int main() {
}