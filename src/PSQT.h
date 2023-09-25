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

constexpr std::array<int, 6> gamePhaseValues = {0, 2, 3, 5, 10, 0};
constexpr int maxPhase =   gamePhaseValues[KNIGHT] * 4
                         + gamePhaseValues[BISHOP] * 4
                         + gamePhaseValues[ROOK  ] * 4
                         + gamePhaseValues[QUEEN ] * 2;

constexpr std::array<std::array<int, 64>, 6> PieceSquareBonusesMG {{
    //Pawn
         //A8
        {   0,    0,    0,    0,    0,    0,    0,    0,
          118,  130,  118,  150,  129,  109,   17,  -15,
           21,   34,   66,   68,   81,   97,   75,   21,
           12,   25,   23,   19,   48,   33,   34,    9,
           -2,   11,    0,   26,   19,    2,   20,   -5,
          -11,    3,   -2,   -5,   13,   -8,   40,    6,
          -13,    3,   -7,  -22,   -5,   11,   53,   -7,
            0,    0,    0,    0,    0,    0,    0,    0},

     //Knight
        {-164, -104,  -17,   20,   64,  -19,  -66,  -83,
            7,   41,   78,  108,   81,  172,   53,   77,
           30,   87,  109,  131,  181,  175,  124,   85,
           34,   51,   87,  120,   92,  126,   75,   84,
           26,   44,   57,   63,   78,   65,   66,   31,
           -3,   21,   41,   53,   67,   46,   57,   19,
          -18,   -6,   24,   33,   28,   36,   26,   17,
          -75,   -6,  -23,   -3,    0,   19,   -8,  -38},

      //Bishop
        { -13,  -36,  -10,  -65,  -55,  -32,   16,  -41,
           -6,   39,   36,   14,   58,   55,   42,   20,
           16,   53,   46,   88,   68,  108,   84,   58,
            9,   26,   61,   74,   71,   68,   39,   18,
           11,   23,   26,   62,   52,   33,   23,   21,
           27,   31,   28,   30,   33,   29,   34,   42,
           27,   20,   43,   11,   19,   33,   47,   26,
           -8,   20,   -1,  -15,   -7,  -13,   27,    8},

       //Rook
        {  89,   87,   97,  105,  120,  143,  120,  139,
           59,   60,   91,  119,   95,  133,  119,  157,
           32,   62,   63,   70,  106,  101,  149,  132,
           18,   27,   42,   46,   52,   52,   71,   74,
            0,   -8,    8,   26,   22,   -5,   29,   22,
          -18,   -9,    5,    2,   11,    5,   47,   24,
          -21,   -6,   13,    6,   10,   12,   39,    3,
           -3,    0,   17,   24,   31,   19,   32,   -2},

       //Queen
        { -47,  -27,   21,   46,   58,   69,   94,   24,
          -12,  -29,   -9,  -24,  -12,   41,   12,   79,
           -1,    4,   -1,   24,   30,   92,  100,   90,
          -20,  -12,    0,    0,    9,   23,   16,   33,
           -7,   -8,  -13,    2,   -4,    0,   23,   17,
           -5,    0,   -3,   -7,    0,    2,   17,   15,
          -12,   -3,   12,   12,    8,   18,   19,   44,
           -7,  -25,  -21,   -1,   -7,  -23,    3,   -3},

       //King
        {  -37,  -56,   -6, -175,  -93,  -52,   35,  169,
          -165,  -99, -125,  -39,  -80,  -67,  -26,  -62,
          -187,  -71, -115, -132,  -81,   19,  -35, -109,
          -128, -144, -154, -212, -182, -153, -155, -197,
          -126, -120, -175, -192, -197, -144, -165, -194,
           -66,  -53, -110, -123, -128, -119,  -70, -101,
            29,  -20,  -33,  -79,  -80,  -64,    5,    7,
            20,   53,   23, -105,  -19,  -75,   27,   24},
}};

constexpr std::array<std::array<int, 64>, 6> PieceSquareBonusesEG {{
     //Pawn
        //A8
        {     0,    0,    0,    0,    0,    0,    0,    0,
            243,  238,  235,  179,  169,  185,  240,  260,
            176,  186,  147,  125,  107,   97,  152,  145,
             76,   80,   56,   49,   43,   42,   69,   64,
             51,   59,   41,   38,   36,   39,   51,   39,
             48,   64,   41,   61,   49,   45,   50,   34,
             63,   60,   54,   57,   70,   59,   48,   37,
              0,    0,    0,    0,    0,    0,    0,    0},

         //Knight
        {   15,   75,   90,   83,   84,   62,   80,  -17,
            76,   96,  100,  101,   90,   72,   88,   52,
            97,  101,  126,  119,  102,  100,   92,   73,
           103,  130,  138,  136,  143,  138,  122,   91,
            97,  116,  141,  143,  148,  137,  116,   88,
            75,  113,  122,  127,  127,  114,   91,   85,
            74,   86,   93,  106,  111,  100,   73,   86,
            67,   42,   81,   83,   88,   75,   60,   56},

          //Bishop
        {   76,   89,   82,  100,   96,   79,   70,   70,
            62,   85,   82,   92,   73,   78,   85,   60,
           101,   91,  108,   79,   92,   93,   83,   85,
            91,  109,   97,  119,  110,  107,  104,   89,
            83,  111,  117,  108,  112,  108,  104,   63,
            74,   99,  105,  102,  111,  102,   81,   67,
            80,   76,   69,   95,   97,   83,   76,   52,
            58,   78,   53,   86,   78,   87,   60,   36,},

           //Rook
         {  59,   62,   74,   66,   62,   47,   51,   49,
            63,   78,   79,   67,   68,   51,   45,   28,
            70,   69,   69,   66,   49,   44,   36,   25,
            64,   69,   72,   74,   54,   47,   43,   33,
            58,   65,   67,   65,   61,   62,   48,   36,
            57,   58,   56,   60,   54,   45,   20,   19,
            44,   51,   54,   56,   49,   41,   23,   35,
            49,   58,   60,   58,   44,   39,   40,   39},

               //Queen
        {  78,   88,  100,   98,   84,   71,   17,   54,
           38,  103,  125,  153,  171,  113,   97,   55,
           54,   61,  125,  123,  141,  112,   57,   39,
           61,   91,  101,  130,  141,  134,  114,   74,
           55,   78,   89,  124,  121,  108,   65,   66,
           32,   55,   77,   61,   78,   79,   57,   29,
           32,   38,   23,   32,   38,    7,  -15,  -72,
           10,   31,   41,   33,   19,   11,  -12,  -25},

                  //King
         {-119,  -70,  -57,    1,  -26,  -18,  -30, -159,
           -15,   16,   21,   12,   30,   43,   32,   -2,
             7,   25,   39,   49,   46,   39,   43,   19,
           -15,   27,   46,   63,   58,   55,   47,   20,
           -28,    3,   38,   51,   53,   35,   25,    4,
           -48,  -14,    6,   21,   26,   13,   -8,  -19,
           -63,  -38,  -19,   -3,   -1,   -8,  -36,  -52,
          -104,  -79,  -59,  -35,  -72,  -40,  -66, -101},
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
