#ifndef MOLYBDENUM_PSQT_H
#define MOLYBDENUM_PSQT_H

#include <array>
#include "Constants.h"
#include "BitStuff.h"


constexpr int PawnValueMG   =   80;
constexpr int KnightValueMG =  280;
constexpr int BishopValueMG =  320;
constexpr int RookValueMG   =  480;
constexpr int QueenValueMG  = 1050;

constexpr int PawnValueEG   =  120;
constexpr int KnightValueEG =  280;
constexpr int BishopValueEG =  320;
constexpr int RookValueEG   =  520;
constexpr int QueenValueEG  = 1050;

constexpr std::array<int, 6> PieceValuesMG = {PawnValueMG, KnightValueMG, BishopValueMG, RookValueMG, QueenValueMG, 0};
constexpr std::array<int, 6> PieceValuesEG = {PawnValueMG, KnightValueMG, BishopValueMG, RookValueMG, QueenValueMG, 0};


constexpr std::array<std::array<int, 64>, 6> PieceSquareBonusesMG {{
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

constexpr std::array<std::array<int, 64>, 6> PieceSquareBonusesEG {{
     //Pawn
        //A8
        {  0,   0,   0,   0,   0,   0,   0,   0,
         100, 100, 100, 100, 100, 100, 100, 100,
          40,  40,  40,  40,  40,  40,  40,  40,
          20,  20,  20,  20,  20,  20,  20,  20,
          10,  10,  10,  10,   0,   0,   0,   0,
           5,   5,   5,   5,   0,   0,   0,   0,
           2,   2,   2,   2,   0,   0,   0,   0,
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
        {-50, -20, -20, -20, -20, -20, -20, -50,
         -20, -15, -20, -20, -20, -20, -15, -20,
         -20,   0,  20,  20,  20,  30,   0,   0,
           0,  20,  25,  40,  40,  25,  20,   0,
          15,   5,  20,  10,  10,  20,   5,  15,
           0,  15,  10,  10,   0,  10,  15,   0,
           5,  20,  10,   5,   5,   5,  10,   5,
           0,   0, -10,   0,   0, -10,   0,   0},

           //Rook
         {  10,  10,  10,  15,  15,  10,  10,  10,
            35,  35,  35,  50,  50,  35,  35,  35,
            20,  20,  20,  20,  20,  20,  20,  20,
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
         {-30, -30, -30, -30, -30, -30, -30, -30,
          -25, -25, -20, -20, -20, -25, -25, -25,
          -20,  -5,   0,   0,   0,   0,  -5, -20,
          -10,  15,  30,  30,  30,  30,  15, -10,
          -10,  15,  30,  30,  30,  30,  15, -10,
          -20,  -5,   0,   0,   0,   0,  -5, -20,
          -25, -25, -20, -20, -20, -25, -25, -25,
          -30, -30, -30, -30, -30, -30, -30, -30},
}};

constexpr std::array<std::array<std::array<int, 64>, 13>, 2> initPSQT() {
    std::array<std::array<std::array<int, 64>, 13>, 2> PSQT{};

    for (int piece = WHITE_PAWN; piece != NO_PIECE + 1; piece++) {
        for (int square = 0; square != 64; square++) {
            bool white = piece < BLACK_PAWN;
            int sq = square ^ (piece < BLACK_PAWN ? 63 : 7);
            int pt = typeOf(piece);
            int pvm = white ? -PieceValuesMG[pt] : PieceValuesMG[pt];
            int pve = white ? -PieceValuesEG[pt] : PieceValuesEG[pt];
            int pbm = white ? -PieceSquareBonusesMG[pt][sq] : PieceSquareBonusesMG[pt][sq];
            int pbe = white ? -PieceSquareBonusesEG[pt][sq] : PieceSquareBonusesEG[pt][sq];

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
