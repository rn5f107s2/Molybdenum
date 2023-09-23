#ifndef MOLYBDENUM_MOVEPICKER_H
#define MOLYBDENUM_MOVEPICKER_H

#include "Movegen.h"
#include "searchUtil.h"

struct Movepicker {
    MoveList ml;
    int moveIndex = 0;
    bool scored = false;
    bool moveListInitialized = false;
};

static std::array<Move, 2> empty = {0, 0};
static std::array<std::array<int, 64>, 64> empty2 = {{{0}}};

const std::array<std::array<int, 13>, 13> MVVLVA =
         {{
                 {1050000, 2050000, 3050000, 4050000, 5050000, 6050000, 1050000, 2050000, 3050000, 4050000, 5050000, 6050000, 0},
                 {1040000, 2040000, 3040000, 4040000, 5040000, 6040000, 1040000, 2040000, 3040000, 4040000, 5040000, 6040000, 0},
                 {1030000, 2030000, 3030000, 4030000, 5030000, 6030000, 1030000, 2030000, 3030000, 4030000, 5030000, 6030000, 0},
                 {1020000, 2020000, 3020000, 4020000, 5020000, 6020000, 1020000, 2020000, 3020000, 4020000, 5020000, 6020000, 0},
                 {1010000, 2010000, 3010000, 4010000, 5010000, 6010000, 1010000, 2010000, 3010000, 4010000, 5010000, 6010000, 0},
                 {1000000, 2000000, 3000000, 4000000, 5000000, 6000000, 1000000, 2000000, 3000000, 4000000, 5000000, 6000000, 0},
                 {1050000, 2050000, 3050000, 4050000, 5050000, 6050000, 1050000, 2050000, 3050000, 4050000, 5050000, 6050000, 0},
                 {1040000, 2040000, 3040000, 4040000, 5040000, 6040000, 1040000, 2040000, 3040000, 4040000, 5040000, 6040000, 0},
                 {1030000, 2030000, 3030000, 4030000, 5030000, 6030000, 1030000, 2030000, 3030000, 4030000, 5030000, 6030000, 0},
                 {1020000, 2020000, 3020000, 4020000, 5020000, 6020000, 1020000, 2020000, 3020000, 4020000, 5020000, 6020000, 0},
                 {1010000, 2010000, 3010000, 4010000, 5010000, 6010000, 1010000, 2010000, 3010000, 4010000, 5010000, 6010000, 0},
                 {1000000, 2000000, 3000000, 4000000, 5000000, 6000000, 1000000, 2000000, 3000000, 4000000, 5000000, 6000000, 0},
                 {     0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0, 0}
         }};

//This also returns the best move
inline Move scoreMoves(Movepicker &mp, Move ttMove, Position &pos, const std::array<Move, 2> &killers, const std::array<std::array<int, 64>, 64> &history) {
    int bestScore = -1000000;
    int bestIndex = 0;

    for (int i = mp.moveIndex; i < mp.ml.length; i++) {
        if (mp.ml.moves[i].move == ttMove)
            mp.ml.moves[i].score = 10000000;
        else if (mp.ml.moves[i].move == killers[0])
            mp.ml.moves[i].score = 900000;
        else if (mp.ml.moves[i].move == killers[1])
            mp.ml.moves[i].score = 800000;

        int from = extract<FROM>(mp.ml.moves[i].move);
        int to   = extract<TO  >(mp.ml.moves[i].move);
        int movingPiece   = pos.pieceLocations[from];
        int capturedPiece = pos.pieceLocations[to];

        mp.ml.moves[i].score += MVVLVA[movingPiece][capturedPiece];
        mp.ml.moves[i].score += history[from][to];

        if (capturedPiece != NO_PIECE) {
            if (!see(pos, -50, mp.ml.moves[i].move))
                mp.ml.moves[i].score = -100000;

            pos.printBoard();
            std::cout << moveToString(mp.ml.moves[i].move) << "\n";
        }

        if (mp.ml.moves[i].score > bestScore) {
            bestScore = mp.ml.moves[i].score;
            bestIndex = i;
        }
    }

    ScoredMove temp = mp.ml.moves[mp.moveIndex];
    mp.ml.moves[mp.moveIndex] = mp.ml.moves[bestIndex];
    mp.ml.moves[bestIndex] = temp;

    return mp.ml.moves[mp.moveIndex++].move;
}

template<bool qsearch> inline
Move pickNextMove(Movepicker &mp, Move ttMove, Position &pos, u64 check = 0ULL, const std::array<Move, 2> &killers = empty, const std::array<std::array<int, 64>, 64> &history = empty2) {
    if (!mp.moveListInitialized)
        generateMoves<qsearch>(pos, mp.ml, check);

    mp.moveListInitialized = true;

    if (!mp.scored) {
        mp.scored = true;
        return scoreMoves(mp, ttMove, pos, killers, history);
    }


    int bestScore = -1000000;
    int bestIndex = 0;

    for (int i = mp.moveIndex; i < mp.ml.length; i++) {
        if (mp.ml.moves[i].score > bestScore) {
            bestScore = mp.ml.moves[i].score;
            bestIndex = i;
        }
    }

    ScoredMove temp = mp.ml.moves[mp.moveIndex];
    mp.ml.moves[mp.moveIndex] = mp.ml.moves[bestIndex];
    mp.ml.moves[bestIndex] = temp;

    return mp.ml.moves[mp.moveIndex++].move;
}

#endif //MOLYBDENUM_MOVEPICKER_H
