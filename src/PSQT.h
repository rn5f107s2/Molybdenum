#ifndef MOLYBDENUM_PSQT_H
#define MOLYBDENUM_PSQT_H

#include <array>
#include "Constants.h"
#include "BitStuff.h"


constexpr int PawnValueMG   = 0;
constexpr int KnightValueMG = 0;
constexpr int BishopValueMG = 0;
constexpr int RookValueMG   = 0;
constexpr int QueenValueMG  = 0;

constexpr int PawnValueEG   = 0;
constexpr int KnightValueEG = 0;
constexpr int BishopValueEG = 0;
constexpr int RookValueEG   = 0;
constexpr int QueenValueEG  = 0;

constexpr int TempoMG = 25;
constexpr int TempoEG = 20;


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
        {0,    0,    0,    0,    0,    0,    0,    0,
         209,  168,  174,  192,  144,  113,   97,  128,
         73,  106,   82,  122,  107,  120,   99,   71,
         48,   75,   72,   88,   95,   79,   81,   51,
         44,   62,   70,   83,   81,   73,   75,   45,
         46,   74,   70,   62,   68,   67,   94,   52,
         34,   73,   56,   49,   43,   79,   80,   46,
         0,    0,    0,    0,    0,    0,    0,    0, },

     //Knight
        { 158,    7,   77,   85,  170,   73,   41,  175,
          257,  244,  317,  311,  318,  319,  231,  259,
          265,  317,  336,  364,  377,  377,  335,  281,
          296,  297,  333,  327,  322,  343,  306,  321,
          280,  293,  300,  307,  309,  307,  291,  287,
          271,  286,  292,  297,  290,  297,  303,  277,
          266,  255,  279,  284,  285,  283,  271,  257,
          0,  269,  226,  242,  251,  261,  272,  148,},

      //Bishop
        { 287,  151,  115,  124,  136,   77,  142,  282,
          293,  311,  318,  279,  289,  320,  294,  299,
          317,  344,  338,  369,  368,  375,  350,  337,
          315,  321,  343,  363,  350,  352,  320,  318,
          307,  312,  327,  343,  346,  319,  312,  310,
          308,  318,  320,  321,  310,  327,  323,  309,
          302,  316,  319,  305,  318,  320,  321,  311,
          273,  299,  296,  272,  279,  289,  266,  297,},

       //Rook
        {
                466,  441,  469,  453,  453,  400,  420,  458,
                433,  437,  470,  465,  458,  460,  436,  419,
                410,  423,  435,  438,  446,  449,  436,  423,
                389,  390,  408,  420,  415,  407,  392,  395,
                373,  365,  384,  389,  386,  373,  382,  371,
                355,  373,  363,  357,  368,  362,  383,  362,
                357,  369,  381,  367,  374,  388,  372,  333,
                370,  375,  379,  392,  391,  388,  352,  362, },

       //Queen
        { 687,  678,  691,  697,  699,  666,  668,  684,
          682,  655,  679,  664,  689,  695,  690,  687,
          680,  681,  681,  719,  708,  727,  716,  705,
          680,  670,  684,  682,  686,  685,  673,  682,
          675,  671,  669,  673,  676,  667,  673,  661,
          665,  681,  666,  667,  661,  667,  673,  664,
          665,  662,  683,  677,  677,  684,  677,  669,
          681,  656,  667,  676,  674,  637,  671,  663, },

       //King
        {
                0,    0,    0,    0,    0,    2,    3,    0,
                0,    0,    0,    4,    5,    7,    0,    0,
                0,    6,    8,    2,    1,    7,    6,    0,
                0,    0,    7,    2,  -13,   -1,    5,    0,
                -11,   -6,    3,    0,  -39,  -22,  -13,  -10,
                -16,  -10,  -28,  -56,  -52,  -51,  -11,  -24,
                19,    4,  -10,  -18,  -21,    4,   22,   41,
                12,   56,   38,  -26,   33,  -13,   57,   57, },
}};

constexpr std::array<std::array<int, 64>, 6> PieceSquareBonusesEG {{
     //Pawn
        //A8
        {   0,    0,    0,    0,    0,    0,    0,    0,
            178,  211,  192,  178,  183,  190,  221,  194,
            185,  181,  165,  153,  147,  139,  167,  172,
            148,  134,  114,  104,   98,  109,  122,  131,
            126,  122,  102,  100,   94,  101,  115,  114,
            123,  121,  105,  106,  107,  103,  110,  110,
            131,  125,  120,  106,  119,  114,  120,  118,
            0,    0,    0,    0,    0,    0,    0,    0, },

         //Knight
        {
                105,  193,  245,  246,  223,  213,  181,   46,
                185,  209,  202,  219,  209,  219,  208,  159,
                195,  205,  225,  219,  212,  207,  203,  201,
                186,  222,  229,  232,  238,  215,  222,  189,
                184,  207,  230,  230,  227,  226,  215,  189,
                156,  187,  211,  225,  219,  206,  187,  166,
                127,  165,  186,  197,  196,  190,  162,  155,
                107,  131,  166,  188,  177,  152,  148,   64, },

          //Bishop
        { 212,  268,  270,  260,  267,  257,  250,  183,
          217,  231,  227,  248,  245,  229,  230,  195,
          219,  231,  237,  229,  224,  225,  226,  216,
          221,  240,  240,  226,  237,  230,  240,  215,
          213,  236,  240,  238,  231,  239,  232,  202,
          214,  222,  231,  241,  245,  225,  213,  219,
          215,  209,  217,  211,  213,  211,  210,  185,
          196,  205,  174,  212,  197,  181,  198,  177, },

           //Rook
         {
                385,  391,  389,  395,  402,  408,  399,  385,
                405,  407,  396,  400,  405,  394,  401,  402,
                410,  410,  407,  405,  406,  401,  402,  401,
                410,  412,  410,  401,  404,  404,  407,  399,
                399,  404,  399,  401,  399,  399,  393,  385,
                393,  386,  393,  396,  390,  388,  376,  380,
                371,  378,  376,  381,  373,  368,  368,  377,
                376,  381,  387,  382,  377,  377,  387,  367, },

               //Queen
        { 719,  769,  776,  786,  783,  787,  752,  750,
          738,  793,  790,  823,  809,  790,  754,  741,
          722,  766,  789,  776,  804,  789,  766,  745,
          714,  787,  786,  817,  820,  809,  813,  767,
          712,  754,  782,  808,  795,  790,  770,  762,
          686,  717,  760,  756,  765,  766,  739,  683,
          639,  691,  698,  708,  714,  672,  639,  631,
          600,  627,  652,  670,  640,  649,  565,  521, },

                  //King
         { -20,   31,   25,   24,   34,   46,   50,    0,
           0,   38,   36,   32,   31,   37,   30,   -4,
           3,   28,   21,   11,   15,   19,   19,   11,
           -13,   16,   13,    7,    6,   10,   15,   -2,
           -22,   -6,    4,    4,   10,    8,    1,  -20,
           -26,  -16,   -9,    5,    6,    3,  -14,  -20,
           -35,  -25,  -16,  -17,  -12,  -20,  -27,  -48,
           -71,  -60,  -50,  -45,  -74,  -47,  -59,  -83, },
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
