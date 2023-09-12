#ifndef MOLYBDENUM_PSQT_H
#define MOLYBDENUM_PSQT_H

#include <array>
#include "Constants.h"
#include "BitStuff.h"


constexpr int PawnValueMG   =   95;
constexpr int KnightValueMG =  264;
constexpr int BishopValueMG =  331;
constexpr int RookValueMG   =  469;
constexpr int QueenValueMG  =  833;

constexpr int PawnValueEG   =   80;
constexpr int KnightValueEG =  259;
constexpr int BishopValueEG =  278;
constexpr int RookValueEG   =  480;
constexpr int QueenValueEG  =  969;

constexpr std::array<int, 6> PieceValuesMG = {PawnValueMG, KnightValueMG, BishopValueMG, RookValueMG, QueenValueMG, 0};
constexpr std::array<int, 6> PieceValuesEG = {PawnValueEG, KnightValueEG, BishopValueEG, RookValueEG, QueenValueEG, 0};


constexpr std::array<std::array<int, 64>, 6> PieceSquareBonusesMG {{
    //Pawn
         //A8
        {  0,   0,   0,   0,   0,   0,   0,   0,
         559, 100, 289, 100, 97,  90,  161,  75,
          66,  60,  74,  60, 163,  45, 101,  30,
          35,  10,  48,  40,  37,  25,  45,   0,
          17,   0,  29,  40,   6,  25,  -9, -20,
           6,   0,  21,   5, -18, -25,  27,   5,
           9,   0,  12, -10, -38,   0,  49,  20,
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
        {-173, -50, -117, -50, -26, -50, -40, -50,
         -41 , -35,  38, -20, -22, -20,  -6, -40,
         -53 ,   0,  20,  20,  26,  30,  97,   0,
         -59 ,  20,  95,  40,  99,  25,  34,   0,
          24 ,   5,  31,  10,  -4,  20,   5,  15,
          -7 ,  15,  48,  10,  -9,  10, -26,   0,
          -86,  20,  10,   5,   6,   5,  39,   5,
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
        {-61,   0,   9,   0,  20,   0, -55,   0,
        -125,   0,  -7,   0,   2,   0,  59,   0,
          -7,   0, -64,   0, 101,   0,  52,   0,
          46,   0, -33,   0, 112,   0,  55,   5,
         -23,   0,  39,   0,  20,   0,  18,   0,
         -15,   5,   3,  10,   0,   0, -28,   0,
        -129,   0, -19,  10,  10,   5, -93,   0,
         -42,  -5,-132,  -5, -18,  -5,  -2,  -5},

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

constexpr std::array<std::array<int, 64>, 6> PieceSquareBonusesEG {{
     //Pawn
        //A8
        {  0,   0,   0,   0,   0,   0,   0,   0,
          68, 235, 246, 214, 151, 201, 197, 176,
         110, 117, 115,  40,  40,  40,  40,  40,
          20,  20,  20,  20,  20,  20,  20,  20,
          10,  10,  10,  10,   0,   0,   0,   0,
           5,   5,   5,   5,   0,   0,   0,   0,
           2,   2,   2,   2,   0,   0,   0,   0,
           0,   0,   0,   0,   0,   0,   0,   0},

         //Knight
        {-70,  23, -70, -64, -70, -62, -70, -77,
         -70,  96,  10, -32,   0,  18, -50,-106,
         -60,-103,  40, 113,  20, -44,  20, -39,
         -50,  20,  25, -60,  75,  14,  10,  39,
         -40,-128,  10,   1,  40,  10,   0,-123,
         -50,  11,  30, -99,   0,  -5,  15, -50,
         -60, -30, -10, -29,  15, -43,  20,  20,
         -70,-149, -40,-159, -30,-285, -10, -63},

          //Bishop
        {-50, -20, -20, -20, -20, -20, -20, -50,
         -20, -15, -20, -20, -20, -20, -15, -20,
         -20,   0,  20,  20,  20,  30,   0,   0,
           0,  20,  25,  40,  40,  25,  20,   0,
          15,   5,  20,  10,  10,  20,   5,  15,
           0,  15,  10,  10,   0,  10,  15,   0,
           5,  20,  10,   5,   5,   5,  10,   5,
           0,   0, -10,   0,   0, -10,   0,   0},

           //Rook
         {  10,  64,  10,  99,  15,   9,  10,  90,
            35,  40,  35,  36,  50,  48,  35,  59,
            20,  76,  20,  58,  20,  75,  20,  67,
             0,  37,   0, 112,   5,  29,   0,  57,
             0, -24,   0,   5,   5,  15,   0,  47,
             0,  27,   0,   9,   5,   0,   0,  15,
             0, -53,   0, -14,  10, -32,   0, -18,
             0, -48,   0,  15,  15,  15,   0, -95},

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
         {-30, 165, -30,  75, -30,  92, -30, -25,
          -25,  81, -20,  61, -20,  88, -25,  16,
          -20, 109,   0,  59,   0,  47,  -5,  50,
          -10,  28,  30, -29,  30,  -9,  15,  36,
          -10,  13,  30,  -2,  30,  -1,  15,  -7,
          -20,   0,   0,  20,   0,  11,  -5, -20,
          -25,  11, -20,  17, -20, -18, -25, -43,
          -30, -75, -30,  17, -30, -38, -30,-103},
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
