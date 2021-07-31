#include <string>
#include <optional>
#include <variant>

namespace std {
    typedef unsigned long Bitboard;
    class BaseBoard {
    public:
        BaseBoard(const optional<string> &);
        void apply_transform(const function<Bitboard(Bitboard)> &);
        void apply_mirror();
        Bitboard pawns, knights, bishops, rooks, queens, kings, occupied_co[2], occupied, promoted;
        void set_chess960_pos(int);
    };
    class SquareSet;
    typedef int Square;
    typedef variant<Bitboard, SquareSet, vector<Square>> IntoSquareSet;
    class SquareSet {
    public:
        SquareSet(const IntoSquareSet &);
    };
    typedef bool Color;
    class Board {
    public:
        Bitboard pawns, knights, bishops, rooks, queens, kings, occupied_co[2], occupied, promoted, castling_rights;
        Color turn;
        optional<Square> ep_square;
        int halfmove_clock, fullmove_number;
        Board(const optional<string> &, bool);
    };
}