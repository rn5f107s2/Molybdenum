#ifndef MOLYBDENUM_SEARCH_H
#define MOLYBDENUM_SEARCH_H

#include "Position.h"
#include "timemanagement.h"
#include <chrono>

int startSearch(Position &pos, searchTime &st);
int iterativeDeepening(Position  &pos, searchTime &st);

struct SearchInfo {
    int nodeCount = 0;
    Move bestRootMove = 0;
    bool stop = false;
    searchTime st;
};

inline int mateInPlies(int score) {
    bool mating = score > MAXMATE;
    int  plies  = (mating ? MATE - score : MATE + score) / 2 + (score < 0 ? 0 : 1);
    return plies;
}

#endif //MOLYBDENUM_SEARCH_H
