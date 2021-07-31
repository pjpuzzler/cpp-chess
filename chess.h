#include <string>
#include <optional>
#include <variant>
#include <functional>
#include <vector>

namespace chess {
    typedef unsigned long Bitboard;
    class BaseBoard {
    public:
        BaseBoard(const std::optional<std::string> &);
        void apply_transform(const std::function<Bitboard(Bitboard)> &);
        void apply_mirror();
        Bitboard pawns, knights, bishops, rooks, queens, kings, occupied_co[2], occupied, promoted;
        void set_chess960_pos(int);
    };
    class SquareSet;
    typedef int Square;
    typedef std::variant<Bitboard, SquareSet, std::vector<Square>> IntoSquareSet;
    class SquareSet {
    public:
        SquareSet(const IntoSquareSet &);
    };
    typedef bool Color;
    class Board;
    class LegalMoveGenerator {
    public:
        LegalMoveGenerator(const Board &board);
    };
    class PseudoLegalMoveGenerator {
    public:
        PseudoLegalMoveGenerator(const Board &);
    };
    class Board {
    public:
        Bitboard pawns, knights, bishops, rooks, queens, kings, occupied_co[2], occupied, promoted, castling_rights;
        Color turn;
        std::optional<Square> ep_square;
        int halfmove_clock, fullmove_number;
        Board(const std::optional<std::string> &, bool);
    };
}