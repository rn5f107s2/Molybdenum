#ifndef MOLYBDENUM_MOVEPICKER_H
#define MOLYBDENUM_MOVEPICKER_H

#include "Movegen.h"
#include "searchUtil.h"

using FromToHist = std::array<std::array<int, 64>, 64>;
using Killers    = std::array<Move, 2>;

static std::array<Move, 2> empty = {0, 0};
static std::array<std::array<int, 64>, 64> empty2 = {{{0}}};

struct Movepicker {
    MoveList ml;
    int moveIndex = 0;
    bool scored = false;
    Move ttMove = NO_MOVE;
    FromToHist mainHistory{};
    Killers killers{};
};

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
inline Move scoreMoves(Movepicker &mp, Position &pos) {
    int bestScore = -1000000;
    int bestIndex = 0;

    for (int i = mp.moveIndex; i < mp.ml.length; i++) {
        Move &move = mp.ml.moves[i].move;
        int &score = mp.ml.moves[i].score;

        if (move == mp.ttMove)
            score = 10000000;
        else if (move == mp.killers[0])
            score = 900000;
        else if (move == mp.killers[1])
            score = 800000;

        int from = extract<FROM>(move);
        int to   = extract<TO  >(move);
        Piece movingPiece   = pos.pieceOn(from);
        Piece capturedPiece = pos.pieceOn(to);

        score += MVVLVA[movingPiece][capturedPiece];
        score += mp.mainHistory[from][to];

        if (score > bestScore) {
            bestScore = score;
            bestIndex = i;
        }
    }

    ScoredMove temp = mp.ml.moves[mp.moveIndex];
    mp.ml.moves[mp.moveIndex] = mp.ml.moves[bestIndex];
    mp.ml.moves[bestIndex] = temp;

    return mp.ml.moves[mp.moveIndex++].move;
}

template<bool qsearch>
inline void initMp(Movepicker &mp,
                   Position &pos,
                   Move ttMove,
                   u64 checkers = 0ULL,
                   const Killers &killers = empty,
                   const FromToHist &history = empty2) {
    mp.mainHistory = history;
    mp.killers = killers;
    mp.ttMove = ttMove;
    generateMoves<qsearch>(pos, mp.ml, checkers);
}

Move pickNextMove(Movepicker &mp, Position &pos) {
    if (!mp.scored) {
        mp.scored = true;
        return scoreMoves(mp, pos);
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
