/*
This is a line-for-line remake of niklasf's 'python-chess' in C++
All credit for the original code and algorithms go to niklasf and his credits
The original source code can be found here: https://github.com/niklasf/python-chess
*/

/*
A chess library with move generation and validation,
and XBoard/UCI engine communication.
*/

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
#include <stack>
#include <variant>
#include <array>
#include <iterator>

namespace chess {

    typedef std::string _EnPassantSpec;


    typedef bool Color;
    typedef int PieceType;
    char piece_symbol(PieceType);


    std::string piece_name(PieceType);

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

        std::optional<Color> winner;
        /* The winning color or ``std::nullopt`` if drawn. */

        Outcome(Termination, std::optional<Color>);

        std::string result() const;
    };



    typedef int Square;
    Square parse_square(const std::string &);


    std::string square_name(Square);


    Square square(int, int);


    int square_file(Square);


    int square_rank(Square);


    int square_distance(Square, Square);


    Square square_mirror(Square);


    typedef unsigned long Bitboard;
    int lsb(Bitboard);

    std::vector<Square> scan_forward(Bitboard);

    int msb(Bitboard);

    std::vector<Square> scan_reversed(Bitboard);

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

    Bitboard _edges(Square);

    std::vector<Bitboard> _carry_rippler(Bitboard);

    std::tuple<std::vector<Bitboard>, std::vector<std::unordered_map<Bitboard, Bitboard>>> _attack_table(const std::vector<int> &);

    std::vector<std::vector<Bitboard>> _rays();

    Bitboard ray(Square, Square);

    Bitboard between(Square, Square);


    class Piece {
        /* A piece with type and color. */

    public:
        PieceType piece_type;
        /* The piece type. */

        Color color;
        /* The piece color. */

        Piece(PieceType, Color);

        char symbol() const;


        std::string unicode_symbol(bool) const;


        operator std::string() const;

        static Piece from_symbol(char);
    };
    std::ostream &operator<<(std::ostream &, const Piece &);



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

        std::optional<PieceType> promotion;
        /* The promotion piece type or ``std::nullopt``. */

        std::optional<PieceType> drop;
        /* The drop piece type or ``std::nullopt``. */

        Move(Square, Square, std::optional<PieceType>, std::optional<PieceType>);

        std::string uci() const;


        std::string xboard() const;

        operator bool() const;

        operator std::string() const;

        static Move from_uci(const std::string &);


        static Move null();
    };
    std::ostream &operator<<(std::ostream &, const Move &);


    class SquareSet;
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
        BaseBoard(const std::optional<std::string> &);

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


        void set_piece_at(Square, const std::optional<Piece> &, bool);


        std::string board_fen(std::optional<bool>) const;


        void set_board_fen(const std::string &);


        std::unordered_map<Square, Piece> piece_map(Bitboard) const;


        void set_piece_map(const std::unordered_map<Square, Piece> &);


        void set_chess960_pos(int);


        std::optional<int> chess960_pos() const;


        operator std::string() const;

        std::string unicode(bool, bool, const std::string &) const;


        bool operator==(const BaseBoard &) const;

        void apply_transform(const std::function<Bitboard(Bitboard)> &f);

        BaseBoard transform(const std::function<Bitboard(Bitboard)> &f) const;


        void apply_mirror();

        BaseBoard mirror() const;


        BaseBoard copy() const;


        static BaseBoard BaseBoard::empty();


        static BaseBoard BaseBoard::from_chess960_pos(int);
    protected:
        void _reset_board();


        void _clear_board();


        Bitboard _attackers_mask(Color, Square, Bitboard) const;

        std::optional<PieceType> _remove_piece_at(Square);

        void _set_piece_at(Square, PieceType, Color, bool);

        void _set_board_fen(std::string);

        void _set_piece_map(const std::unordered_map<Square, Piece> &);

        void _set_chess960_pos(int);
    };
    std::ostream &operator<<(std::ostream &, const BaseBoard &);



    class Board;
    class _BoardState {

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
        static std::string aliases[6];
        static std::optional<std::string> uci_variant;
        static std::optional<std::string> xboard_variant;
        static std::string starting_fen;

        static std::optional<std::string> tbw_suffix;
        static std::optional<std::string> tbz_suffix;
        static std::optional<unsigned char> tbw_magic[4];
        static std::optional<unsigned char> tbz_magic[4];
        static std::optional<std::string> pawnless_tbw_suffix;
        static std::optional<std::string> pawnless_tbz_suffix;
        static std::optional<unsigned char> pawnless_tbw_magic;
        static std::optional<unsigned char> pawnless_tbz_magic;
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

        Bitboard promoted;
        /* A bitmask of pieces that have been promoted. */

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

        Board(const std::optional<std::string> &, bool);

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


        void set_piece_at(Square, const std::optional<Piece> &, bool);


        std::vector<Move> generate_pseudo_legal_moves(Bitboard, Bitboard) const;

        std::vector<Move> generate_pseudo_legal_ep(Bitboard, Bitboard) const;

        std::vector<Move> generate_pseudo_legal_captures(Bitboard, Bitboard) const;

        Bitboard checkers_mask() const;

        SquareSet checkers() const;


        bool is_check() const;


        bool gives_check(const Move &);


        bool is_into_check(const Move &) const;

        bool was_into_check() const;

        bool is_pseudo_legal(const Move &) const;

        bool is_legal(const Move &) const;

        bool is_variant_end() const;


        bool is_variant_loss() const;


        bool is_variant_win() const;


        bool is_variant_draw() const;


        bool is_game_over(bool);

        std::string result(bool);

        std::optional<Outcome> outcome(bool);


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


        bool is_repetition(int);


        void push(Move);


        Move pop();


        Move peek() const;


        Move find_move(Square, Square, std::optional<PieceType>);


        std::string castling_shredder_fen() const;

        std::string castling_xfen() const;

        bool has_pseudo_legal_en_passant() const;

        bool has_legal_en_passant() const;

        std::string fen(bool, _EnPassantSpec, std::optional<bool>);

        std::string shredder_fen(_EnPassantSpec, std::optional<bool>);

        void set_fen(const std::string &);


        void set_castling_fen(const std::string &);


        void set_board_fen(const std::string &);


        void set_piece_map(const std::unordered_map<Square, Piece> &);


        void set_chess960_pos(int);


        std::optional<int> chess960_pos(bool, bool, bool) const;


        std::string epd(bool, const _EnPassantSpec &, std::optional<bool>, const std::unordered_map<std::string, std::optional<std::variant<std::string, int, float, Move, std::vector<Move>>>> &);


        std::unordered_map<std::string, std::optional<std::variant<std::string, int, float, Move, std::vector<Move>>>> set_epd(const std::string &);


        std::string san(const Move &);


        std::string lan(const Move &);


        std::string san_and_push(const Move &);

        std::string variation_san(const std::vector<Move> &) const;


        Move parse_san(const std::string &);


        Move push_san(const std::string &);


        std::string uci(Move, std::optional<bool>) const;


        Move parse_uci(const std::string &);


        Move push_uci(const std::string &);


        std::string xboard(const Move &, std::optional<bool>) const;

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


        std::vector<Move> generate_legal_moves(Bitboard, Bitboard) const;

        std::vector<Move> generate_legal_ep(Bitboard, Bitboard) const;

        std::vector<Move> generate_legal_captures(Bitboard, Bitboard) const;

        std::vector<Move> generate_castling_moves(Bitboard, Bitboard) const;

        bool operator==(const Board &) const;

        void apply_transform(const std::function<Bitboard(Bitboard)> &);

        Board transform(const std::function<Bitboard(Bitboard)> &) const;


        void apply_mirror();

        Board mirror() const;


        Board copy(std::variant<bool, int>) const;


        static Board empty(bool);


        static std::tuple<Board, std::unordered_map<std::string, std::optional<std::variant<std::string, int, float, Move, std::vector<Move>>>>> from_epd(const std::string &, bool);


        static Board from_chess960_pos(int);
    private:
        std::vector<_BoardState> _stack;

        bool _is_halfmoves(int) const;

        _BoardState _board_state() const;

        void _push_capture(const Move &, Square, PieceType, bool) const;

        void _set_castling_fen(const std::string &);

        std::string _epd_operations(const std::unordered_map<std::string, std::optional<std::variant<std::string, int, float, Move, std::vector<Move>>>> &);

        std::unordered_map<std::string, std::optional<std::variant<std::string, int, float, Move, std::vector<Move>>>> _parse_epd_ops(const std::string &, const std::function<Board()> &) const;

        std::string _algebraic(const Move &, bool);

        std::string _algebraic_and_push(const Move &, bool);

        std::string _algebraic_without_suffix(const Move &, bool);

        bool _reduces_castling_rights(const Move &) const;

        std::optional<Square> _valid_ep_square() const;

        bool _ep_skewered(Square, Square) const;

        Bitboard _slider_blockers(Square) const;

        bool _is_safe(Square, Bitboard, const Move &) const;

        std::vector<Move> _generate_evasions(Square, Bitboard, Bitboard, Bitboard) const;

        bool _attacked_for_king(Bitboard, Bitboard) const;

        Move _from_chess960(bool, Square, Square, std::optional<PieceType>, std::optional<PieceType>) const;

        Move _to_chess960(const Move &) const;

        std::tuple<Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Bitboard, Color, Bitboard, std::optional<Square>> _transposition_key() const;
    };
    std::ostream &operator<<(std::ostream &, Board);



    class PseudoLegalMoveGenerator {

    public:
        Board board;
        PseudoLegalMoveGenerator(const Board &);

        operator bool() const;

        int count() const;

        auto begin() const;

        auto end() const;
    };
    std::ostream &operator<<(std::ostream &, PseudoLegalMoveGenerator);


    class LegalMoveGenerator {
    
    public:
        Board board;
        LegalMoveGenerator(const Board &);

        operator bool() const;

        int count() const;

        auto begin() const;

        auto end() const;
    };
    std::ostream &operator<<(std::ostream &, LegalMoveGenerator);


    typedef std::variant<Bitboard, SquareSet, std::vector<Square>> IntoSquareSet;

    class SquareSet {
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
        Bitboard mask;
        std::vector<Square> iter;
        SquareSet(const IntoSquareSet &);

        // Set

        auto begin() const;

        auto end() const;

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

        bool operator==(const IntoSquareSet &) const;

        SquareSet operator<<(int) const;

        SquareSet operator>>(int) const;

        SquareSet operator<<=(int);

        SquareSet operator>>=(int);

        SquareSet operator~() const;

        operator unsigned long int() const;

        operator std::string() const;

        static SquareSet ray(Square, Square);

        static SquareSet between(Square, Square);

        static SquareSet from_square(Square);
    };
    std::ostream &operator<<(std::ostream &, const SquareSet &);
}