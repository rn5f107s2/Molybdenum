#ifndef MOLYBDENUM_SEARCH_H
#define MOLYBDENUM_SEARCH_H

#include "Position.h"
#include "timemanagement.h"
#include "Movepicker.h"
#include <chrono>
#include <cmath>

struct SearchInfo {
    int nodeCount = 0;
    Move bestRootMove = 0;
    bool stop = false;
    searchTime st;
};

int startSearch(Position &pos, searchTime &st);
int iterativeDeepening(Position  &pos, searchTime &st);
int searchRoot(Position &pos, SearchInfo &si, int depth, int alpha = -INFINITE, int beta = INFINITE);
void clearHistory();
int aspirationWindow(Position &pos, int depth, SearchInfo &si, int prevScore);

inline int mateInPlies(int score) {
    bool mating = score > MAXMATE;
    int  plies  = (mating ? MATE - score : MATE + score) / 2 + (score < 0 ? 0 : 1);
    return plies;
}

inline std::array<double, 256> initReductions() {
    std::array<double, 256> R{};

    for (int i = 0; i < 256; i++) R[i] = std::log(i);

    return R;
}

static std::array<double, 256> Log = initReductions();
inline int lmrReduction(int depth, int movecount) {
    return int(1 + Log[depth] * Log[movecount] / 2);
}


#endif //MOLYBDENUM_SEARCH_H
