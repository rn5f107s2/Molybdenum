#ifndef MOLYBDENUM_PSQT_H
#define MOLYBDENUM_PSQT_H

#include <array>
#include "Constants.h"
#include "BitStuff.h"


constexpr int PawnValueMG   =  106;
constexpr int KnightValueMG =  295;
constexpr int BishopValueMG =  373;
constexpr int RookValueMG   =  558;
constexpr int QueenValueMG  = 1239;

constexpr int PawnValueEG   =   80;
constexpr int KnightValueEG =  299;
constexpr int BishopValueEG =  317;
constexpr int RookValueEG   =  604;
constexpr int QueenValueEG  = 1169;

constexpr std::array<int, 6> PieceValuesMG = {PawnValueMG, KnightValueMG, BishopValueMG, RookValueMG, QueenValueMG, 0};
constexpr std::array<int, 6> PieceValuesEG = {PawnValueEG, KnightValueEG, BishopValueEG, RookValueEG, QueenValueEG, 0};


constexpr std::array<std::array<int, 64>, 6> PieceSquareBonusesMG {{
    //Pawn
         //A8
        {0, 0, 0, 0, 0, 0, 0, 0,
         310, 146, 198, 150, 121, 125, 73, 1,
         45, 50, 82, 76, 113, 105, 99, 29,
         20, 17, 39, 35, 48, 33, 42, 17,
         6, 11, 16, 34, 11, 2, 12, -5,
         5, 3, 6, -5, -3, -24, 32, 14,
         3, 11, 1, -22, -13, 11, 53, 9,
         0, 0, 0, 0, 0, 0, 0, 0},

     //Knight
        {-60, -24, 23, 28, 64, 13, 14, -35,
         -17, 33, 78, 100, 81, 140, 61, 53,
         6, 87, 109, 115, 149, 167, 108, 53,
         10, 35, 79, 120, 100, 118, 51, 52,
         -6, 28, 49, 63, 70, 57, 58, 7,
         -27, 13, 41, 37, 43, 46, 41, -5,
         -42, -22, 0, 25, 28, 28, 10, 17,
         -83, -14, -47, -19, -16, -5, -24, -70},

      //Bishop
        {-37, 20, -10, -1, 9, 16, 24, -1,
         -6, 31, 52, 54, 58, 63, 58, 20,
         0, 53, 70, 80, 76, 116, 108, 50,
         1, 26, 85, 82, 95, 76, 31, 18,
         3, 23, 34, 54, 44, 33, 31, 5,
         19, 23, 36, 30, 25, 21, 18, 34,
         -5, 28, 35, 11, 19, 33, 55, 26,
         -8, 20, -1, -7, -7, -13, 11, 0},

       //Rook
        {73, 63, 65, 73, 88, 87, 64, 91,
         67, 60, 75, 103, 95, 109, 95, 109,
         40, 54, 55, 62, 82, 77, 93, 84,
         26, 19, 26, 38, 44, 36, 39, 50,
         8, 0, 0, 18, 14, -5, 21, 14,
         -10, -9, -3, 2, 3, -3, 31, 16,
         -21, -6, 5, 6, 10, 12, 31, 3,
         -3, 0, 9, 24, 23, 19, 24, -2},

       //Queen
        {-15, 45, 69, 78, 90, 69, 30, 40,
         -52, 11, 23, 40, 44, 49, 60, 63,
         15, 28, -9, 40, 94, 60, 84, 42,
         36, 4, 0, 16, 81, 23, 40, 25,
         -15, 8, 19, 10, 20, 0, 23, 9,
         -13, 0, -3, 1, 0, 2, -7, 7,
         -76, -3, -4, 12, 8, 18, -37, 28,
         -39, -25, -77, -1, -15, -15, 3, -11},

       //King
        {-53, -88, -46, -111, -61, -60, -21, 65,
         -77, -43, -77, -39, -48, -59, -10, -22,
         -99, -15, -67, -84, -81, -29, -35, -53,
         -88, -64, -82, -132, -126, -97, -91, -101,
         -70, -64, -95, -120, -109, -96, -85, -106,
         -42, -29, -70, -75, -72, -79, -46, -53,
         13, -12, -17, -55, -56, -32, 5, 7,
         -4, 29, 7, -73, -19, -59, 27, 24},
}};

constexpr std::array<std::array<int, 64>, 6> PieceSquareBonusesEG {{
     //Pawn
        //A8
        {0, 0, 0, 0, 0, 0, 0, 0,
         91, 190, 131, 155, 153, 145, 160, 180,
         120, 146, 115, 101, 59, 89, 120, 121,
         36, 40, 16, 1, 3, 18, 21, 40,
         3, 19, -7, -2, 12, -17, 19, 7,
         8, 24, 1, 21, 41, 13, 18, 10,
         31, 52, 14, 9, 78, 35, 24, 5,
         0, 0, 0, 0, 0, 0, 0, 0},

         //Knight
        {-73, -5, 34, 27, 36, 6, 0, -49,
         12, 32, 36, 45, 26, 48, 32, 12,
         41, 37, 62, 87, 86, 68, 52, 41,
         39, 50, 82, 80, 63, 106, 74, 75,
         41, 44, 77, 63, 76, 81, 60, 48,
         3, -7, 34, 63, 63, 34, 27, 45,
         2, 6, 5, 10, 15, 20, -39, -26,
         -53, -62, -23, -13, -16, -5, -20, -24},

          //Bishop
        {100, 25, 58, 36, 32, 23, 22, 30,
         6, 53, 18, 44, 41, 46, 45, 20,
         61, 43, 52, 55, 44, 77, 19, 53,
         59, 45, 25, 71, 38, 67, 32, 33,
         -5, 47, 53, 76, 88, 52, 48, -9,
         26, 27, 25, 62, 79, 46, 57, 35,
         96, 20, 21, 23, 41, 27, 28, 4,
         -14, 14, -27, -10, -10, 15, 4, -44},

           //Rook
         {51, 54, 74, 66, 62, 55, 51, 49,
          47, 62, 79, 67, 60, 67, 53, 52,
          54, 69, 77, 66, 73, 76, 76, 57,
          56, 61, 80, 74, 62, 63, 59, 57,
          26, 33, 51, 49, 45, 30, 32, 20,
          9, 10, 24, 28, 14, 5, 4, -5,
          -4, 11, 22, 16, 1, 1, 7, -5,
          -7, 2, 36, 42, 20, 15, 24, -1},

               //Queen
        {46, 24, 60, 74, 68, 87, 105, 62,
        110, 7, 77, 97, 115, 129, 41, 119,
        14, 29, 173, 123, 45, 216, 97, 151,
        -59, 35, 133, 122, 29, 158, 50, 106,
        47, 38, 1, 116, 65, 92, 57, 66,
        16, 15, 53, 21, 38, 55, 113, 21,
        192, 6, 87, 16, 6, -1, 161, 0,
        34, -65, 201, 25, 3, -45, -28, -25},

                  //King
         {-95, -54, -41, -15, -26, -2, 18, -87,
          -31, 16, 13, 12, 30, 43, 48, 6,
          -9, 33, 39, 49, 62, 71, 67, 19,
          -15, 27, 46, 55, 58, 63, 47, 12,
          -36, 3, 30, 43, 45, 27, 17, -12,
          -48, -14, -2, 13, 18, 5, -8, -27,
          -31, -22, -19, -19, -9, -8, -12, -36,
          -72, -23, -11, -67, -56, -56, -34, -85},
}};

constexpr std::array<std::array<std::array<int, 64>, 13>, 2> initPSQT() {
    std::array<std::array<std::array<int, 64>, 13>, 2> PSQT{};

    for (int piece = WHITE_PAWN; piece != NO_PIECE + 1; piece++) {
        for (int square = 0; square != 64; square++) {
            bool white = piece < BLACK_PAWN;
            int sq = square ^ (piece < BLACK_PAWN ? 63 : 7);
            int pt = typeOf(piece);
            int pvm = white ? PieceValuesMG[pt] : -PieceValuesMG[pt];
            int pve = white ? PieceValuesEG[pt] : -PieceValuesEG[pt];
            int pbm = white ? PieceSquareBonusesMG[pt][sq] : -PieceSquareBonusesMG[pt][sq];
            int pbe = white ? PieceSquareBonusesEG[pt][sq] : -PieceSquareBonusesEG[pt][sq];

            if (piece != 12) {
                PSQT[0][piece][square] = pvm + pbm;
                PSQT[1][piece][square] = pve + pbe;
            } else {
                PSQT[0][piece][square] = 0;
                PSQT[1][piece][square] = 0;
            }
        }
    }

    return PSQT;
}

constexpr std::array<std::array<std::array<int, 64>, 13>, 2> PSQT = initPSQT();

#endif //MOLYBDENUM_PSQT_H
