#include "chess.cpp"
#include <iostream>

int main() {
    chess::Board board;
    // board.set_chess960_pos(275);
    board.set_epd("bnrknrqb/pppppppp/8/8/8/8/PPPPPPPP/BNRKNRQB w KQkq -");
    std::cout << board;
}