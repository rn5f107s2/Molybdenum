#include "Position.h"
#include "Movegen.h"
#include "search.h"
#include "eval.h"
#include "Constants.h"
#include "Movepicker.h"
#include <chrono>
#include <algorithm>

std::array<std::array<Move, 2>, 100> killers;
std::array<std::array<std::array<int, 64>, 64>, 2> history;

template<bool ROOT>
int search(int alpha, int beta, Position &pos, int depth, SearchInfo &si, int plysInSearch = 0, bool doNull = true);
int qsearch(int alpha, int beta, Position &pos, SearchInfo &si);

int startSearch(Position &pos, searchTime &st) {
    return iterativeDeepening(pos, st);
}

void clearHistory() {
    memset(&history, 0, sizeof(history[0]) * history.size());
}

int searchRoot(Position &pos, SearchInfo &si, int depth, int alpha, int beta) {
    int score = search<true>(alpha, beta, pos, depth, si);
    return score;
}

int iterativeDeepening(Position  &pos, searchTime &st) {
    int score;
    SearchInfo si;
    si.st = st;

    for (int depth = 1; depth != 100; depth++) {
        score = aspirationWindow(pos, depth, si, score);

        if (std::chrono::steady_clock::now() > (si.st.searchStart + si.st.thinkingTime))
            break;

        std::string uciOutput;
        auto searchTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - si.st.searchStart).count();
        uciOutput += "info depth ";
        uciOutput += std::to_string(depth);

        uciOutput += " currmove ";
        uciOutput += moveToString(si.bestRootMove);

        uciOutput += " score ";
        uciOutput += abs(score) > MAXMATE ? "mate " : "cp ";
        uciOutput += std::to_string(abs(score) > MAXMATE ? mateInPlies(score) : score);

        uciOutput += " nodes ";
        uciOutput += std::to_string(si.nodeCount);

        uciOutput += " time ";
        uciOutput += std::to_string(searchTime);

        uciOutput += " nps ";
        uciOutput += std::to_string((si.nodeCount / std::max(int(searchTime), 1)) * 1000);

        std::cout << uciOutput << "\n";
    }

    std::cout << "bestmove " << moveToString(si.bestRootMove) << "\n";
    return score;
}

int aspirationWindow(Position &pos, int depth, SearchInfo &si, int prevScore) {
    int alpha = -INFINITE;
    int beta  =  INFINITE;
    int score =  INFINITE;
    int failCount = 0;

    if (depth >= 6) {
        alpha = prevScore - 35;
        beta  = prevScore + 35;
    }

    search:
    score = search<true>(alpha, beta, pos, depth, si);

    if (std::chrono::steady_clock::now() > (si.st.searchStart + si.st.thinkingTime))
        return score;

    if (score <= alpha) {
        failCount++;
        alpha -= (alpha - score) + 15 * failCount * failCount;
        goto search;
    } else if (score >= beta) {
        failCount++;
        beta += (score - beta) + 15 * failCount * failCount;
        goto search;
    }

    return score;
}

template<bool ROOT>
int search(int alpha, int beta, Position &pos, int depth, SearchInfo &si, int plysInSearch, bool doNull) {
    u64 checkers = attackersTo<false, false>(lsb(pos.bitBoards[pos.sideToMove ? WHITE_KING : BLACK_KING]),getOccupied<WHITE>(pos) | getOccupied<BLACK>(pos), pos.sideToMove ? BLACK_PAWN : WHITE_PAWN, pos);
    Move bestMove = 0, currentMove = 0;
    int bestScore = -INFINITE, score = -INFINITE, moveCount, staticEval = evaluate(pos);
    bool exact = false, check = checkers, pvNode = (beta - alpha) > 1, ttHit = false;

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
        ttBound = tte->bound;
        ttMove  = tte->move;
        ttDepth = tte->depth;
        ttHit   = true;
        ttScore = tte->score;

        if (ttScore > MAXMATE)
            ttScore -= plysInSearch;
        else if (ttScore < -MAXMATE)
            ttScore += plysInSearch;
    }

    if (!pvNode && ttHit && ttDepth >= depth && (ttBound == EXACT || (ttBound == LOWER && ttScore >= beta) || (ttBound == UPPER && ttScore <= alpha)))
        return ttScore;

    if (!pvNode && !check && staticEval - 100 * depth >= beta)
        return staticEval;

    if (!pvNode && !check && depth >= 2 && doNull && staticEval >= beta) {
        pos.makeNullMove();
        int reduction = std::min(depth, (3 + (staticEval >= beta + 250) + (depth > 6)));
        int nullScore = -search<false>(-beta, -alpha, pos, depth - reduction, si, plysInSearch + 1, false);
        pos.unmakeNullMove();

        if (nullScore >= beta)
            return nullScore;
    }

    Movepicker mp;
    while ((currentMove = pickNextMove<false>(mp, ttMove, pos, checkers, killers[plysInSearch], history[pos.sideToMove])) != 0) {
        int from = extract<FROM>(currentMove);
        int to   = extract<TO>(currentMove);
        pos.makeMove(currentMove);
        si.nodeCount++;
        moveCount++;

        int reductions = lmrReduction(depth, moveCount);
        reductions = std::max(reductions, 1);

        if (depth > 2 && moveCount > 2 && !pvNode) {
            score = -search<false>(-alpha - 1, -alpha, pos, depth - reductions, si, plysInSearch + 1);

            if (score > alpha && reductions > 1)
                score = -search<false>(-alpha - 1, -alpha, pos, depth - 1, si, plysInSearch + 1);
        }else {
            if (!pvNode || moveCount > 1)
                score = -search<false>(-alpha - 1, -alpha, pos, depth - 1, si, plysInSearch + 1);

            if (pvNode && ((score > alpha && score < beta) || moveCount == 1))
                score = -search<false>(-beta, -alpha, pos, depth - 1, si, plysInSearch + 1);
        }

        pos.unmakeMove(currentMove);

        if (si.stop && !(ROOT && depth == (1 + check)))
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

                    if (!pos.isCapture(bestMove))
                        history[pos.sideToMove][from][to] += depth * depth;

                    TT.save(tte, key, bestScore, LOWER, bestMove, depth, plysInSearch);
                    return bestScore;
                }
            }
        }
    }

    if (ROOT && exact)
        si.bestRootMove = bestMove;

    if (bestScore == -INFINITE) {
        return checkers ? (-MATE + plysInSearch) : DRAW;
    }

    TT.save(tte, key, bestScore, exact ? EXACT : UPPER, bestMove, depth, plysInSearch);
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
