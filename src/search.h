#ifndef MOLYBDENUM_SEARCH_H
#define MOLYBDENUM_SEARCH_H

#include "Position.h"
#include "timemanagement.h"
#include "PSQT.h"
#include "searchUtil.h"
#include "Movegen.h"

#include <atomic>
#include <chrono>
#include <cmath>

#include "tune.h"

#ifdef TUNE
extern Tune tune;
#endif

extern bool prettyprint;

class Thread;

struct SearchInfo {
    Move bestRootMove = 0;
    SearchTime st;
    int rootMoveCount = 0;
    int selDepth = 0;

public:
    std::atomic_uint64_t nodeCount = 0;
    std::atomic_bool stop = false;

    SearchInfo([[maybe_unused]] const SearchInfo &si) {
        clear();
    }

    SearchInfo() {}

    void clear() {
        bestRootMove  = NO_MOVE;
        rootMoveCount = 0;
        selDepth      = 0;

        nodeCount.store(0, std::memory_order_relaxed);
        stop.store(false, std::memory_order_relaxed);
    }
};

struct SearchStack {
    int plysInSearch = 0;
    int staticEval = INFINITE;
    int quarterRed = 0;
    Move currMove = 0;
    Move excluded = NO_MOVE;
    PieceToHist *contHist = nullptr;
};

enum NodeType {
    Root, PVNode, NonPvNode
};

static Move emptyMove = NO_MOVE;

class SearchState {
    std::array<std::array<Move, 2>, STACKSIZE> killers;
    SideFromToHist mainHistory;
    ContHist continuationHistory;
    std::array<std::array<Move, MAXDEPTH>, MAXDEPTH> pvMoves;
    std::array<int, MAXDEPTH> pvLength;

    int currentPvDepth;

public:

    void clearHistory();
    std::string outputWDL(Position &pos);
    int startSearch(Position &pos, SearchTime &st, int maxDepth, Move &bestMove = emptyMove);
    int iterativeDeepening(Position  &pos, SearchTime &st, int maxDepth, [[maybe_unused]] Move &bestMove);
    int aspirationWindow(int prevScore, Position &pos, SearchInfo &si, int depth);

    template<NodeType nt>
    int qsearch(int alpha, int beta, Position &pos, SearchInfo &si, SearchStack *stack);

    template<NodeType nt>
    int search(int alpha, int beta, Position &pos, int depth, SearchInfo &si, SearchStack *stack);

    Thread* thread;
    SearchInfo si;

    void prettyPrint(Position &pos, SearchInfo &si, int score, int depth);
};

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
bool stop(SearchTime &st, SearchInfo &si) {
    return    (std::chrono::steady_clock::now() > (si.st.searchStart + si.st.thinkingTime[LT]) && st.limit == Time)
              || (st.limit == Nodes && si.nodeCount >= st.nodeLimit);
}

void prettyInitial();

#endif //MOLYBDENUM_SEARCH_H
