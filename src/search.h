#ifndef MOLYBDENUM_SEARCH_H
#define MOLYBDENUM_SEARCH_H

#include "Position.h"
#include "timemanagement.h"
#include "Movepicker.h"
#include <chrono>

struct SearchInfo {
    int nodeCount = 0;
    Move bestRootMove = 0;
    bool stop = false;
    searchTime st;
};

int startSearch(Position &pos, searchTime &st);
int iterativeDeepening(Position  &pos, searchTime &st);
int searchRoot(Position &pos, SearchInfo &si, int depth, int alpha = -INFINITE, int beta = INFINITE);
int aspirationWindow(Position &pos, int depth, SearchInfo &si, int prevScore);

inline int mateInPlies(int score) {
    bool mating = score > MAXMATE;
    int  plies  = (mating ? MATE - score : MATE + score) / 2 + (score < 0 ? 0 : 1);
    return plies;
}

#endif //MOLYBDENUM_SEARCH_H
