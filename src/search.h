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
    int rootMoveCount = 0;
    int selDepth = 0;
};

struct SearchStack {
    int plysInSearch = 0;
    int staticEval = INFINITE;
    int quarterRed = 0;
    Move currMove = 0;
    Move excluded = NO_MOVE;
    PieceToHist *contHist = nullptr;
};

extern u64 benchNodes;
static Move emptyMove = NO_MOVE;

int startSearch(Position &pos, searchTime &st, int maxDepth = MAXDEPTH, Move &bm = emptyMove);
int iterativeDeepening(Position &pos, searchTime &st, int maxDepth = MAXDEPTH, Move &bm = emptyMove);
int start(Position &pos, SearchInfo &si, int depth);
void clearHistory();

inline std::array<double, 256> initReductions() {
    std::array<double, 256> R{};

    for (int i = 1; i < 256; i++) R[i] = std::log(i);
    R[0] = 0;

    return R;
}

static std::array<double, 256> Log = initReductions();

inline double lmrReduction(int depth, int movecount, bool improving) {
    return 0.66 + !improving * 0.46 + Log[depth] * Log[movecount] / 2.10;
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

inline float sigmoid(int val) {
    return 1 / (1 + std::exp((1.0f / 133.0f) * float(-val)));
}

inline Move selectMove(MoveList &ml, std::array<int, 218> &depths, std::array<int, 218> &scores, float cpuct, float fpu, uint64_t nodeCount) {
    int   bestIdx  = 0;
    float best     = -1.0f;

    const int d0Visits = 20;
    const float base   = 2.0f;

    for (int i = 0; i < ml.length; i++) {
        const int approxVisits = d0Visits * pow(base, depths[i]);

        const float q = depths[i] ? sigmoid(scores[i]) : fpu;
        
        float thisVal = q + (float(ml.moves[i].score) / 16384.0f) * cpuct * std::sqrt(nodeCount + 1) / (approxVisits);

        if (thisVal <= best)
            continue;

        best    = thisVal;
        bestIdx = i; 
    }

    return bestIdx;
}

#endif //MOLYBDENUM_SEARCH_H
