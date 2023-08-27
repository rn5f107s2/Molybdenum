#include "Position.h"
#include "Movegen.h"
#include "search.h"
#include "eval.h"
#include "Constants.h"
#include "Movepicker.h"
#include <chrono>

std::array<std::array<Move, 2>, 100> killers;

template<bool ROOT>
int search(int alpha, int beta, Position &pos, int depth, SearchInfo &si, int plysInSearch = 0, bool doNull = true);
int qsearch(int alpha, int beta, Position &pos, SearchInfo &si);

int startSearch(Position &pos, searchTime &st) {
    return iterativeDeepening(pos, st);
}

int iterativeDeepening(Position  &pos, searchTime &st) {
    int score;
    SearchInfo si;
    si.st = st;

    for (int depth = 1; depth != 100; depth++) {
        score = search<true>(-INFINITE, INFINITE, pos, depth, si);

        if (std::chrono::steady_clock::now() > (si.st.searchStart + si.st.thinkingTime))
            break;

        std::cout << "info depth " << depth << " currmove " << moveToString(si.bestRootMove) << " score cp " << score <<
        " nodes: " << si.nodeCount << "\n";
    }

    std::cout << "bestmove " << moveToString(si.bestRootMove) << "\n";
    return score;
}

template<bool ROOT>
int search(int alpha, int beta, Position &pos, int depth, SearchInfo &si, int plysInSearch, bool doNull) {
    u64 checkers = attackersTo<false, false>(lsb(pos.bitBoards[pos.sideToMove ? WHITE_KING : BLACK_KING]),getOccupied<WHITE>(pos) | getOccupied<BLACK>(pos), pos.sideToMove ? BLACK_PAWN : WHITE_PAWN, pos);
    Move bestMove, currentMove;
    int bestScore = -INFINITE, score = -INFINITE, moveCount, staticEval = evaluate(pos);
    bool exact = false, check = checkers, pvNode = (beta - alpha) > 1, ttHit = false;;

    depth += check;

    if (depth <= 0)
        return qsearch(alpha, beta, pos, si);

    MoveList ml;

    if (!(si.nodeCount & 1023) && (std::chrono::steady_clock::now() > (si.st.searchStart + si.st.thinkingTime)))
        si.stop = true;

    if constexpr (!ROOT) {
        if (pos.hasRepeated(plysInSearch) || pos.plys50moveRule > 99)
            return 0;
    }

    int ttScore, ttBound, ttDepth;
    Move ttMove = 0;
    u64 key = pos.key();
    TTEntry* tte = TT.probe(key);

    if (tte->key == key) {
        ttScore = tte->score;
        ttBound = tte->bound;
        ttMove  = tte->move;
        ttDepth = tte->depth;
        ttHit   = true;
    }

    if (!pvNode && ttHit && ttDepth >= depth && (ttBound == EXACT || (ttBound == LOWER && ttScore >= beta)))
        return ttScore;

    if (!pvNode && !check && depth >= 2 && doNull && staticEval >= beta) {
        pos.makeNullMove();
        int reduction = std::min(depth, (3 + (staticEval >= beta + 250) + (depth > 6)));
        int nullScore = -search<false>(-beta, -alpha, pos, depth - reduction, si, plysInSearch + 1, false);
        pos.unmakeNullMove();

        if (nullScore >= beta)
            return nullScore;
    }

    Movepicker mp;
    while ((currentMove = pickNextMove< false>(mp, ttMove, pos, checkers, killers[plysInSearch])) != 0) {
        pos.makeMove(currentMove);
        si.nodeCount++;
        moveCount++;

        if (exact)
            score = -search<false>(-alpha - 1, -alpha, pos, depth - 1, si, plysInSearch + 1);

        if (!exact || (score > alpha && score < beta))
            score = -search<false>(-beta, -alpha, pos, depth - 1, si, plysInSearch + 1);

        pos.unmakeMove(currentMove);

        if (si.stop)
            return DRAW;

        if (score > bestScore) {
            bestScore = score;
            bestMove = currentMove;

            if(score > alpha) {
                alpha = score;
                exact = true;

                if (score >= beta) {
                    if (!pos.isCapture(bestMove) && bestMove != killers[plysInSearch][0]) {
                        killers[plysInSearch][1] = killers[plysInSearch][0];
                        killers[plysInSearch][0] = bestMove;
                    }
                    TT.save(tte, key, bestScore, LOWER, bestMove, depth);
                    return bestScore;
                }
            }
        }
    }

    if (bestScore == -INFINITE) {
        return checkers ? (-MATE + plysInSearch) : DRAW;
    }

    if constexpr (ROOT)
        si.bestRootMove = bestMove;

    TT.save(tte, key, bestScore, exact ? EXACT : UPPER, bestMove, depth);
    return bestScore;
}

int qsearch(int alpha, int beta, Position &pos, SearchInfo &si) {
    Move bestMove = 0;
    Move currentMove;
    bool check;
    int bestScore = evaluate(pos);

    if (bestScore >= beta)
        return bestScore;

    Movepicker mp;
    while ((currentMove = pickNextMove<true>(mp, 0, pos, check)) != 0) {
        pos.makeMove(currentMove);
        si.nodeCount++;

        int score = -qsearch(-beta, -alpha, pos, si);

        pos.unmakeMove(currentMove);

        if (score > bestScore) {
            bestScore = score;

            if (score > alpha) {
                alpha = score;
                bestMove = currentMove;

                if (score >= beta)
                    return score;
            }
        }
    }

    return bestScore;
}
