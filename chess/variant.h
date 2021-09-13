/*
This is a line-for-line remake of niklasf's 'python-chess' in C++
All credit for the original code and algorithms go to niklasf and his credits
The original source code can be found here: https://github.com/niklasf/python-chess
*/

#include "chess.cpp"

namespace chess
{
    class SuicideBoard : public chess::Board
    {
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

            chess::Bitboard pin_mask(chess::Color, chess::Square) const;

            bool _attacked_for_king(chess::Bitboard, chess::Bitboard) const;

            chess::Bitboard checkers_mask() const;

            bool gives_check(chess::Move) const;

            bool is_into_check(chess::Move) const;

            bool was_into_check() const;

            int _material_balance() const;

            bool is_variant_end() const;

            bool is_variant_win() const;

            bool is_variant_loss() const;
    };
}