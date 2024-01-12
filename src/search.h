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
//#define TUNE

#ifdef TUNE

#include "tune.h"
#include "UCIOptions.h"

#define RANGE(x, y, z) int(y), int(z), int(x), reinterpret_cast<void (*)(int)>(x)

#define UPDATEFLOAT(x) tune.x = float(tuneOptions.getValue(#x)) / 100;
#define UPDATEINT(x) tune.x = tuneOptions.getValue(#x);
#define TUNEFLOAT(w, x, y, z) if (!initialized) {spinOptions.push(UCIOptionSpin(#w, RANGE(int(x * 100), int(y * 100), int(z * 100))));} UPDATEFLOAT(w)
#define TUNEINT(w, x, y, z) if (!initialized) {spinOptions.push(UCIOptionSpin(#w, RANGE(x, y, z)));} UPDATEINT(w)

static TuneOptions tuneOptions;

inline void TuneOptions::init() {
    TUNEFLOAT(LMRBase, 0.75, 0.01, 4)
    TUNEFLOAT(LMRDiv, 2.19, 0.01, 8)
    TUNEFLOAT(LMRImproving, 0.55, 0, 2)
    TUNEINT(LMRDepth, 2, 1, 4)
    TUNEINT(LMRMovecount, 2, 1, 4)

    TUNEINT(AspiBase, 83, 1, 160)
    TUNEINT(AspiLo, 24, 1, 200)
    TUNEINT(AspiHi, 49, 1, 300)
    TUNEINT(AspiDepth, 2, 2, 10)
    TUNEINT(AspiWide, 3, 1, 5)

    TUNEINT(RFPBase, 115, 1, 300)
    TUNEINT(RFPImproving, 203, 1, 400)
    TUNEINT(RFPDepth, 9, 3, 15)

    TUNEINT(NMPDepth, 2, 1, 10)
    TUNEINT(NMPSeThreshold, 274, 1, 500)
    TUNEINT(NMPDepthThreshold, 6, 1, 12)
    TUNEINT(NMPBaseRed, 3, 1, 6)

    TUNEINT(MCPDepth, 4, 1, 24)
    TUNEINT(MCPMultiplier, 12, 1, 24)

    TUNEINT(FPDepth, 7, 1, 24)
    TUNEINT(FPMult, 209, 1, 400)
    TUNEINT(FPBase, 179, 1, 350)

    TUNEINT(HistDepth, 5, 1, 20)
    TUNEINT(HistMult, -5460, -9000, -1)

    TUNEINT(QsSEEMargin, -94, -200, -1)
    TUNEINT(QsDeltaMargin, 159, 1, 300)

    TUNEINT(HistDepthMult, 16, 1, 32)
    TUNEINT(HistMax, 1638, 1, 3000)
    TUNEINT(HistLimit, 15, 1, 20)

    TUNEINT(SEEPawn, 100, 1, 1200)
    TUNEINT(SEEKnight, 299, 1, 1200)
    TUNEINT(SEEBishop, 281, 1, 1200)
    TUNEINT(SEERook, 538, 1, 1200)
    TUNEINT(SEEQueen, 972, 1, 1200)

    initialized = true;
}

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

    for (int i = 0; i < 256; i++) R[i] = std::log(i);

    return R;
}

static std::array<double, 256> Log = initReductions();

inline int lmrReduction(int depth, int movecount, bool improving) {
    return int(0.64 + !improving * 0.43 + Log[depth] * Log[movecount] / 2.06);
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
