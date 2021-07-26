#include <string>
#include <stack>
#include <functional>
#include <vector>
#include <variant>

using namespace std;

typedef bool Color;
typedef uint8_t Square;
typedef unsigned long Bitboard;

class Move;

typedef BaseBoard BaseBoardT;

class BaseBoard
{
public:
    Bitboard occupied_co[2], pawns, knights, bishops, rooks, queens, kings, promoted, occupied;

    BaseBoard(const string &board_fen);

    void set_chess960_pos(uint16_t scharnagl);

    void apply_transform(function<Bitboard(Bitboard)> f);

    void apply_mirror();
};

class Board : public BaseBoard
{
public:
    Color turn;
    Bitboard castling_rights;
    int8_t ep_square;
    uint16_t fullmove_number;
    uint8_t halfmove_clock;
    Bitboard promoted;
    bool chess960;
    stack<Move> move_stack;
};

class LegalMoveGenerator
{
public:
    LegalMoveGenerator(const Board *board);
};

class PseudoLegalMoveGenerator
{
public:
    PseudoLegalMoveGenerator(const Board *board);
};

using IntoSquareSet = variant<Bitboard, vector<Square>>;

class SquareSet
{
public:
    SquareSet(IntoSquareSet squares);
};
