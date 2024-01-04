#ifndef MOLYBDENUM_SEARCH_H
#define MOLYBDENUM_SEARCH_H

#include "Position.h"
#include "timemanagement.h"
#include "Movepicker.h"
#include "PSQT.h"
#include "Movegen.h"
#include <chrono>
#include <cmath>

//#define DATAGEN

struct SearchInfo {
    u64 nodeCount = 0;
    bool stop = false;
    Move bestRootMove = 0;
    searchTime st;
};

struct SearchStack {
    int plysInSearch = 0;
    int staticEval = INFINITE;
    Move currMove = 0;
    PieceToHist *contHist = nullptr;
};

extern u64 benchNodes;
static Move emptyMove = 0;

int startSearch(Position &pos, searchTime &st, int maxDepth = MAXDEPTH, Move &bm = emptyMove);
int iterativeDeepening(Position  &pos, searchTime &st, int maxDepth = MAXDEPTH, Move &bm = emptyMove);
int aspirationWindow(int prevScore, Position &pos, SearchInfo &si, int depth);
void clearHistory();

inline std::array<double, 256> initReductions() {
    std::array<double, 256> R{};

    for (int i = 0; i < 256; i++) R[i] = std::log(i);

    return R;
}

static std::array<double, 256> Log = initReductions();

inline int lmrReduction(int depth, int movecount, bool improving) {
    return int(0.65 + !improving * 0.45 + Log[depth] * Log[movecount] / 2.39);
}

inline int mateInPlies(int score) {
    bool mating = score > MAXMATE;
    int  plies  = (mating ? MATE - score : MATE + score) / 2 + (score < 0 ? 0 : 1);
    return plies * (mating ? 1 : -1);
}

inline bool stop(searchTime &st, SearchInfo &si) {
    return    (std::chrono::steady_clock::now() > (si.st.searchStart + si.st.thinkingTime) && st.limit == Time)
              || (st.limit == Nodes && si.nodeCount >= st.nodeLimit);
}

#endif //MOLYBDENUM_SEARCH_H
