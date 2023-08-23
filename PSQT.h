#ifndef MOLYBDENUM_PSQT_H
#define MOLYBDENUM_PSQT_H

#include <array>
#include "Constants.h"
#include "BitStuff.h"


constexpr int PawnValue   = 100;
constexpr int KnightValue = 300;
constexpr int BishopValue = 300;
constexpr int RookValue   = 500;
constexpr int QueenValue  = 900;

constexpr std::array<int, 6> PieceValues = {PawnValue, KnightValue, BishopValue, RookValue, QueenValue, 0};

constexpr std::array<std::array<int, 64>, 6> PieceSquareBonuses {{
    //Pawn
         //A8
        {  0,   0,   0,   0,   0,   0,   0,   0,
         100, 100, 100, 100, 100,  90,  75,  75,
          60,  60,  60,  60,  60,  45,  30,  30,
          10,  10,  10,  40,  40,  25,   0,   0,
           0,   0,  25,  40,  40,  25, -20, -20,
          10,   0, -10,   5,   0, -25, -30,   5,
           0,   0,   0, -10, -30,   0,  20,  20,
           0,   0,   0,   0,   0,   0,   0,   0},

     //Knight
        {-70, -70, -70, -70, -70, -70, -70, -70,
         -70, -50,  10,   0,   0,  10, -50, -70,
         -60,  20,  40,  20,  20,  40,  20, -60,
         -50,  10,  25,  75,  75,  25,  10, -50,
         -40,   0,  10,  40,  40,  10,   0, -40,
         -50,  15,  30,   0,   0,  30,  15, -50,
         -60, -30, -10,  15,  15, -10,  20,  20,
         -70, -20, -40, -30, -30, -40, -10, -70},

      //Bishop
        {-50, -50, -50, -50, -50, -50, -50, -50,
         -40, -35, -20, -20, -20, -20, -35, -40,
         -60,   0,  20,  20,  20,  30,   0,   0,
           0,  20,  25,  40,  40,  25,  20,   0,
          15,   5,  20,  10,  10,  20,   5,  15,
           0,  15,  10,  10,   0,  10,  15,   0,
           5,  20,  10,   5,   5,   5,  10,   5,
           0,   0, -10,   0,   0, -10,   0,  0},

       //Rook
        {  0,   0,   0,   5,   5,   0,   0,   0,
          35,  35,  35,  50,  50,  35,  35,  35,
           0,   0,   0,  20,  20,   0,   0,   0,
           0,   0,   0,   5,   5,   0,   0,   0,
           0,   0,   0,   5,   5,   0,   0,   0,
           0,   0,   0,   5,   5,   0,   0,   0,
           0,   0,   0,  10,  10,   5,   0,   0,
           0,   0,   0,  15,  15,   5,   0,   0},

       //Queen
        {  0,   0,   0,   0,   0,   0,   0,   0,
           0,   0,   0,   0,   0,   0,   0,   0,
           0,   0,   0,   0,   0,   0,   0,   0,
           0,   0,   0,   0,   0,   0,   0,   5,
           0,   0,   0,   0,   0,   0,   0,   0,
           0,   5,   0,  10,   0,   0,   5,   0,
           0,   0,  10,  10,  10,   5,   0,   0,
          -5,  -5,  -5,  -5,  -5,  -5,  -5,  -5},

       //King
        {-60, -60, -60, -60, -60, -60, -60, -60,
         -60, -60, -60, -60, -60, -60, -60, -60,
         -60, -60, -60, -60, -60, -60, -60, -60,
         -60, -60, -60, -60, -60, -60, -60, -60,
         -50, -50, -50, -50, -50, -50, -50, -50,
         -30, -30, -40, -40, -40, -40, -30, -30,
         -20, -20, -20, -30, -30, -15,   5,  10,
         -10, -10, -15, -20, -10, -15,  25,  20},
}};

constexpr std::array<std::array<int, 64>, 13> initPSQT() {
    std::array<std::array<int, 64>, 13> PSQT{};

    for (int piece = WHITE_PAWN; piece != NO_PIECE + 1; piece++) {
        for (int square = 0; square != 64; square++) {
            bool white = piece < BLACK_PAWN;
            int sq = square ^ (piece < BLACK_PAWN ? 63 : 7);
            int pt = typeOf(piece);
            int pv = white ? -PieceValues[pt] : PieceValues[pt];
            int pb = white ? -PieceSquareBonuses[pt][sq] : PieceSquareBonuses[pt][sq];

            if (piece != 12)
                PSQT[piece][square] = pv + pb;
            else
                PSQT[12][square] = 0;
        }
    }

    return PSQT;
}

constexpr std::array<std::array<int, 64>, 13> PSQT = initPSQT();

inline void printPSQT() {
    for (int pt = 0; pt != 12; pt++) {
        for (int square = 63; square != -1; square--) {
            int sq = square;
            if (pt < BLACK_PAWN) {
                sq ^= 56;
            }

            std::cout << PSQT[pt][sq] << " ";

            if (square % 8 == 0)
                std::cout << "\n";
        }
    }
}

#endif //MOLYBDENUM_PSQT_H
