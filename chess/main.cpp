#include "chess.cpp"
#include <iostream>

int main() {
    chess::Board board;

    while (!board.is_game_over(true)) {
        while (true) {
            std::cout << board.unicode(false, true) << std::endl;

            std::string san;
            std::cout << board.ply() + 1 << ". " << (board.turn ? "[WHITE] " : "[BLACK] ") << "Enter Move: ";
            std::cin >> san;
            std::cout << std::endl;

            try {
                chess::Move move = board.parse_san(san);
                if (!move) {
                    throw std::invalid_argument("");
                }
                board.push(move);
                break;
            } catch (std::invalid_argument) {
                std::cout << "Invalid Move, Try Again..." << std::endl;
            }
        }
    }

    std::cout << "Game Over! Result: " << board.result(true);
}