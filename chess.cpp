#include <iostream>
#include <vector>
#include <utility>

using namespace std;

typedef int Board[8][8], PieceType, Square, Rank, File;
typedef bool Color;

const Color WHITE = true, BLACK = false;
const PieceType PAWN = 1, KNIGHT = 2, BISHOP = 3, ROOK = 4, QUEEN = 5, KING = 6;
const Square A1 = 0, A2 = 1, A3 = 2, A4 = 3, A5 = 4, A6 = 5, A7 = 6, A8 = 7, B1 = 8, B2 = 9, B3 = 10, B4 = 11, B5 = 12, B6 = 13, B7 = 14, B8 = 15, C1 = 16, C2 = 17, C3 = 18, C4 = 19, C5 = 20, C6 = 21, C7 = 22, C8 = 23, D1 = 24, D2 = 25, D3 = 26, D4 = 27, D5 = 28, D6 = 29, D7 = 30, D8 = 31, E1 = 32, E2 = 33, E3 = 34, E4 = 35, E5 = 36, E6 = 37, E7 = 38, E8 = 39, F1 = 40, F2 = 41, F3 = 42, F4 = 43, F5 = 44, F6 = 45, F7 = 46, F8 = 47, G1 = 48, G2 = 49, G3 = 50, G4 = 51, G5 = 52, G6 = 53, G7 = 54, G8 = 55, H1 = 56, H2 = 57, H3 = 58, H4 = 59, H5 = 60, H6 = 61, H7 = 62, H8 = 63;

class Piece
{
public:
    PieceType pieceType;
    Color color;

    Piece(PieceType pieceType, Color color)
    {
        pieceType = pieceType;
        color = color;
    }
};

class Chess
{
public:
    Board board;
    Color turn;

    Chess()
    {
    }

    bool isCheck()
    {
        // Tests if the current side to move is in check.
    }

    bool isAttackedBy(Color color, Square square)
    {
        // Checks if the given side attacks the given square.
        // Pinned pieces still count as attackers. Pawns that can be captured en passant are not considered attacked.
    }
};

int main()
{
    Chess chess;
}