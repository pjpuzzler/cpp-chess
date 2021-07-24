/*
This is a complete remake of niklasf's python-chess in C++
The original version can be found here: https://github.com/niklasf/python-chess
*/

#include <string>
#include <unordered_map>

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
    Color winner;

    Outcome(Termination termination, Color winner)
    {
        termination = termination;
        // The reason for the game to have ended.
        winner = winner;
        // The winning color or ``None`` if drawn.
    }

    string result()
    {
        // Returns ``1-0``, ``0-1`` or ``1/2-1/2``.
        return winner == NULL ? "1/2-1/2" : (winner ? "1-0" : "0-1");
    }
};

int main()
{
}