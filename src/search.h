#ifndef MOLYBDENUM_SEARCH_H
#define MOLYBDENUM_SEARCH_H

#include "Position.h"
#include "timemanagement.h"
#include "PSQT.h"
#include "searchUtil.h"
#include "Movegen.h"
#include <chrono>
#include <cmath>

#include "tune.h"

#ifdef TUNE
extern Tune tune;
#endif

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
    Move excluded = NO_MOVE;
    PieceToHist *contHist = nullptr;
};

extern u64 benchNodes;
static Move emptyMove = NO_MOVE;

int startSearch(Position &pos, searchTime &st, int maxDepth = MAXDEPTH, Move &bm = emptyMove);
int iterativeDeepening(Position  &pos, searchTime &st, int maxDepth = MAXDEPTH, Move &bm = emptyMove);
int aspirationWindow(int prevScore, Position &pos, SearchInfo &si, int depth);
void clearHistory();

inline std::array<double, 256> initReductions() {
    std::array<double, 256> R{};

    for (int i = 1; i < 256; i++) R[i] = std::log(i);
    R[0] = 0;

    return R;
}

static std::array<double, 256> Log = initReductions();

inline double lmrReduction(int depth, int movecount, bool improving) {
    return 0.66 + !improving * 0.49 + Log[depth] * Log[movecount] / 2.02;
}

inline int mateInPlies(int score) {
    bool mating = score > MAXMATE;
    int  plies  = (mating ? MATE - score : MATE + score) / 2 + (score < 0 ? 0 : 1);
    return plies * (mating ? 1 : -1);
}

template<LimitType LT> inline
bool stop(searchTime &st, SearchInfo &si) {
    return    (std::chrono::steady_clock::now() > (si.st.searchStart + si.st.thinkingTime[LT]) && st.limit == Time)
              || (st.limit == Nodes && si.nodeCount >= st.nodeLimit);
}

#endif //MOLYBDENUM_SEARCH_H
