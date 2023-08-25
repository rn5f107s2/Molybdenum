#include "Position.h"
#include "Movegen.h"
#include "search.h"
#include "eval.h"
#include "Constants.h"
#include "Movepicker.h"
#include <chrono>

template<bool ROOT>
int search(int alpha, int beta, Position &pos, int depth, SearchInfo &si, int plysInSearch = 0);

int startSearch(Position &pos, searchTime &st) {
    return iterativeDeepening(pos, st);
}

int iterativeDeepening(Position  &pos, searchTime &st) {
    int score;
    SearchInfo si;
    si.st = st;

    for (int depth = 1; depth != 100; depth++) {
        score = search<true>(-INFINITE, INFINITE, pos, depth, si);
        std::cout << "info depth " << depth << " currmove " << moveToString(si.bestRootMove) << " score cp " << score <<
        " nodes: " << si.nodeCount << "\n";

        if (std::chrono::steady_clock::now() > (si.st.searchStart + si.st.thinkingTime))
            break;
    }

    std::cout << "bestmove " << moveToString(si.bestRootMove) << "\n";
    return score;
}

template<bool ROOT>
int search(int alpha, int beta, Position &pos, int depth, SearchInfo &si, int plysInSearch) {
    if (depth <= 0)
        return evaluate(pos);

    MoveList ml;
    Move bestMove;
    Move currentMove = 0;
    int bestScore = -INFINITE;
    //bool exact = false;
    bool check = generateMoves(pos, ml);

    if (!(si.nodeCount & 1023) && (std::chrono::steady_clock::now() > (si.st.searchStart + si.st.thinkingTime)))
        si.stop = true;

    if constexpr (!ROOT) {
        if (pos.hasRepeated(plysInSearch) || pos.plys50moveRule > 99)
            return 0;
    }

    //int ttScore;
    //int ttBound;
    //int ttDepth;
    //Move ttMove = 0;
    //bool ttHit = false;
    //u64 key = pos.key();
    //TTEntry* tte = TT.probe(key);

    //if (tte->key == key) {
    //    ttScore = tte->score;
    //    ttBound = tte->bound;
    //    ttMove  = tte->move;
    //    ttDepth = tte->depth;
    //    ttHit   = true;
    //}

    Movepicker mp;
    while ((currentMove = pickNextMove(mp, 0, pos)) != 0) {
        pos.makeMove(currentMove);
        si.nodeCount++;

        int score = -search<false>(-beta, -alpha, pos, depth - 1, si, plysInSearch + 1);

        pos.unmakeMove(currentMove);

        if (si.stop)
            return DRAW;

        if (score > bestScore) {
            bestScore = score;
            bestMove = currentMove;

            if(score > alpha) {
                alpha = score;
                //exact = true;

                if (score > beta) {
                    //TT.save(tte, key, bestScore, LOWER, bestMove, depth);
                    return bestScore;
                }
            }
        }
    }

    if (bestScore == -INFINITE) {
        return check ? (-MATE + plysInSearch) : DRAW;
    }

    if constexpr (ROOT)
        si.bestRootMove = bestMove;

    //TT.save(tte, key, bestScore, exact ? EXACT : UPPER, bestMove, depth);
    return bestScore;
}
