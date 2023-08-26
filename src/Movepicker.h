#ifndef MOLYBDENUM_MOVEPICKER_H
#define MOLYBDENUM_MOVEPICKER_H

#include "Movegen.h"

struct Movepicker {
    MoveList ml;
    int moveIndex = 0;
    bool scored = false;
    bool moveListInitialized = false;
};

std::array<Move, 2> empty = {0, 0};

const std::array<std::array<int, 13>, 13> MVVLVA =
         {{       //Pawn  Knight  Bishop  Rook    Queen   King    Pawn    Knight  Bishop  Rook    Queen   King    NONE
                 {105000, 205000, 305000, 405000, 505000, 605000, 105000, 205000, 305000, 405000, 505000, 605000, 0},
                 {104000, 204000, 304000, 404000, 504000, 604000, 104000, 204000, 304000, 404000, 504000, 604000, 0},
                 {103000, 203000, 303000, 403000, 503000, 603000, 103000, 203000, 303000, 403000, 503000, 603000, 0},
                 {102000, 202000, 302000, 402000, 502000, 602000, 102000, 202000, 302000, 402000, 502000, 602000, 0},
                 {101000, 201000, 301000, 401000, 501000, 601000, 101000, 201000, 301000, 401000, 501000, 601000, 0},
                 {100000, 200000, 300000, 400000, 500000, 600000, 100000, 200000, 300000, 400000, 500000, 600000, 0},
                 {105000, 205000, 305000, 405000, 505000, 605000, 105000, 205000, 305000, 405000, 505000, 605000, 0},
                 {104000, 204000, 304000, 404000, 504000, 604000, 104000, 204000, 304000, 404000, 504000, 604000, 0},
                 {103000, 203000, 303000, 403000, 503000, 603000, 103000, 203000, 303000, 403000, 503000, 603000, 0},
                 {102000, 202000, 302000, 402000, 502000, 602000, 102000, 202000, 302000, 402000, 502000, 602000, 0},
                 {101000, 201000, 301000, 401000, 501000, 601000, 101000, 201000, 301000, 401000, 501000, 601000, 0},
                 {100000, 200000, 300000, 400000, 500000, 600000, 100000, 200000, 300000, 400000, 500000, 600000, 0},
                 {     0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0, 0}
         }};

//This also returns the best move
inline Move scoreMoves(Movepicker &mp, Move ttMove, Position &pos, const std::array<Move, 2> &killers) {
    int bestScore = -1000000;
    int bestIndex = 0;

    for (int i = mp.moveIndex; i < mp.ml.length; i++) {
        if (mp.ml.moves[i].move == ttMove)
            mp.ml.moves[i].score = 1000000;
        else if (mp.ml.moves[i].move == killers[0])
            mp.ml.moves[i].score = 90000;
        else if (mp.ml.moves[i].move == killers[1])
            mp.ml.moves[i].score = 80000;

        int from = extract<FROM>(mp.ml.moves[i].move);
        int to   = extract<TO  >(mp.ml.moves[i].move);
        int movingPiece   = pos.pieceLocations[from];
        int capturedPiece = pos.pieceLocations[to];

        mp.ml.moves[i].score += MVVLVA[movingPiece][capturedPiece];

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
Move pickNextMove(Movepicker &mp, Move ttMove, Position &pos, u64 check = 0ULL, const std::array<Move, 2> &killers = empty) {
    if (!mp.moveListInitialized)
        generateMoves<qsearch>(pos, mp.ml, check);

    mp.moveListInitialized = true;

    if (!mp.scored) {
        mp.scored = true;
        return scoreMoves(mp, ttMove, pos, killers);
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
