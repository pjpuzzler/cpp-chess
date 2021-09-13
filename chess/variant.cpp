/*
This is a line-for-line remake of niklasf's 'python-chess' in C++
All credit for the original code and algorithms go to niklasf and his credits
The original source code can be found here: https://github.com/niklasf/python-chess
*/

#include "variant.h"
#include <iostream>

namespace chess
{
    std::string SuicideBoard::aliases[] = {"Suicide", "Suicide chess"};
    std::optional<std::string> SuicideBoard::uci_variant = "suicide";
    std::optional<std::string> SuicideBoard::xboard_variant = "suicide";

    std::optional<std::string> SuicideBoard::tbw_suffix = ".stbw";
    std::optional<std::string> SuicideBoard::tbz_suffix = ".stbz";
    std::optional<std::array<unsigned char, 4>> SuicideBoard::tbw_magic = {{0x7b, 0xf6, 0x93, 0x15}};
    std::optional<std::array<unsigned char, 4>> SuicideBoard::tbz_magic = {{0xe4, 0xcf, 0xe7, 0x23}};
    std::optional<std::string> SuicideBoard::pawnless_tbw_suffix = ".gtbw";
    std::optional<std::string> SuicideBoard::pawnless_tbz_suffix = ".gtbz";
    std::optional<std::array<unsigned char, 4>> SuicideBoard::pawnless_tbw_magic = {{0xbc, 0x55, 0xbc, 0x21}};
    std::optional<std::array<unsigned char, 4>> SuicideBoard::pawnless_tbz_magic = {{0xd6, 0xf5, 0x1b, 0x50}};
    bool SuicideBoard::connected_kings = true;
    bool SuicideBoard::one_king = false;
    bool SuicideBoard::captures_compulsory = true;

    chess::Bitboard SuicideBoard::pin_mask(chess::Color color, chess::Square square) const
    {
        return chess::BB_ALL;
    }

    bool SuicideBoard::_attacked_for_king(chess::Bitboard path, chess::Bitboard occupied) const
    {
        return false;
    }

    chess::Bitboard SuicideBoard::checkers_mask() const
    {
        return chess::BB_EMPTY;
    }

    bool SuicideBoard::gives_check(chess::Move move) const
    {
        return false;
    }

    bool SuicideBoard::is_into_check(chess::Move move) const
    {
        return false;
    }

    bool SuicideBoard::was_into_check() const
    {
        return false;
    }

    int SuicideBoard::_material_balance() const
    {
        return (chess::popcount(this->occupied_co[this->turn]) -
                chess::popcount(this->occupied_co[!this->turn]));
    }

    bool SuicideBoard::is_variant_end() const
    {
        for (chess::Bitboard has_pieces : this->occupied_co) {
            if (!has_pieces) {
                return true;
            }
        }
        return false;
    }

    bool SuicideBoard::is_variant_win() const
    {
        if (!this->occupied_co[this->turn]) {
            return true;
        } else {
            return this->is_stalemate() && this->_material_balance() < 0;
        }
    }

    bool SuicideBoard::is_variant_loss() const
    {
        if (!this->occupied_co[this->turn]) {
            return false;
        } else {
            return this->is_stalemate() && this->_material_balance() > 0;
        }
    }
}