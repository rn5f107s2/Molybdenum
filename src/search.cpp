#include "Position.h"
#include "Movegen.h"
#include "search.h"
#include "eval.h"
#include "Constants.h"
#include "Movepicker.h"
#include "searchUtil.h"
#include <chrono>
#include <algorithm>

std::array<std::array<Move, 2>, 100> killers;
std::array<std::array<std::array<int, 64>, 64>, 2> mainHistory;
std::array<std::array<std::array<std::array<int, 13>, 64>, 13>, 64> contHist;

template<bool ROOT>
int search(int alpha, int beta, Position &pos, int depth, SearchInfo &si, SearchStack *stack);
int qsearch(int alpha, int beta, Position &pos, SearchInfo &si);

int startSearch(Position &pos, searchTime &st) {
    return iterativeDeepening(pos, st);
}

void clearHistory() {
    memset(&mainHistory, 0, sizeof(mainHistory[0]) * mainHistory.size());
    memset(&contHist, 0, sizeof(contHist[0]) * contHist.size());
}

int searchRoot(Position &pos, SearchInfo &si, int depth, int alpha, int beta) {
    std::array<SearchStack, MAXDEPTH + 2> stack;
    int score = search<true>(alpha, beta, pos, depth, si, &stack[2]);

    for (int i = 0; i != 64; i++)
        for (int j = 0; j != 64; j++)
            for (int k = 0; k != 2; k++)
                mainHistory[k][j][i] /= 4;

    for (int i = 0; i != 13; i++)
        for (int j = 0; j != 64; j++)
            for (int k = 0; k != 13; k++)
                for (int l = 0; l != 64; l++)
                    contHist[i][j][k][l] /= 4;

    return score;
}

int iterativeDeepening(Position  &pos, searchTime &st) {
    int score;
    SearchInfo si;
    si.st = st;

    for (int depth = 1; depth != MAXDEPTH; depth++) {
        score = aspirationWindow(score, pos, si, depth);

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

int aspirationWindow(int prevScore, Position &pos, SearchInfo &si, int depth) {
    int delta = std::clamp(80 - depth * depth, 25, 50);
    int alpha = -INFINITE;
    int beta  =  INFINITE;

    if (depth >= 2) {
        alpha = std::max(-INFINITE, prevScore - delta);
        beta  = std::min( INFINITE, prevScore + delta);
    }

    std::array<SearchStack, MAXDEPTH + 3> stack;

    stack[0].contHist = &contHist[NO_PIECE][0];
    stack[1].contHist = &contHist[NO_PIECE][0];
    int score = search<true>(alpha, beta, pos, depth, si, &stack[2]);

    while ((score >= beta || score <= alpha) && (std::chrono::steady_clock::now() < (si.st.searchStart + si.st.thinkingTime))) {
        delta += delta / 3;

        if (score >= beta)
            beta = std::max(score + delta, INFINITE);
        else
            alpha = std::max(score - delta, -INFINITE);

        score = search<true>(alpha, beta, pos, depth, si, &stack[2]);
    }

    return score;
}

template<bool ROOT>
int search(int alpha, int beta, Position &pos, int depth, SearchInfo &si, SearchStack *stack) {
    u64 checkers = attackersTo<false, false>(lsb(pos.bitBoards[pos.sideToMove ? WHITE_KING : BLACK_KING]),getOccupied<WHITE>(pos) | getOccupied<BLACK>(pos), pos.sideToMove ? BLACK_PAWN : WHITE_PAWN, pos);
    Move bestMove = 0, currentMove = 0;
    int bestScore = -INFINITE, score = -INFINITE, moveCount = 0;
    bool exact = false, check = checkers, pvNode = (beta - alpha) > 1, ttHit = false;
    Stack<Move> historyUpdates;
    stack->staticEval = evaluate(pos);
    stack->plysInSearch = ROOT ? 0 : (stack-1)->plysInSearch + 1;

    depth += check;

    if (stack->plysInSearch >= MAXDEPTH)
        return stack->staticEval;

    if (depth <= 0)
        return qsearch(alpha, beta, pos, si);

    MoveList ml;

    if (!(si.nodeCount & 1023) && (std::chrono::steady_clock::now() > (si.st.searchStart + si.st.thinkingTime)))
        si.stop = true;

    if constexpr (!ROOT) {
        if (pos.hasRepeated(stack->plysInSearch) || pos.plys50moveRule > 99 || (pos.phase <= 3 && !(pos.bitBoards[WHITE_PAWN] | pos.bitBoards[BLACK_PAWN])))
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
            ttScore -= stack->plysInSearch;
        else if (ttScore < -MAXMATE)
            ttScore += stack->plysInSearch;
    }

    if (!pvNode && ttHit && ttDepth >= depth && (ttBound == EXACT || (ttBound == LOWER && ttScore >= beta) || (ttBound == UPPER && ttScore <= alpha)))
        return ttScore;

    if (!pvNode && !check && stack->staticEval - 100 * depth >= beta)
        return stack->staticEval;

    if (!pvNode && !check && depth >= 2 && (stack-1)->currMove != 65 && stack->staticEval >= beta) {
        pos.makeNullMove();
        int reduction = std::min(depth, (3 + (stack->staticEval >= beta + 250) + (depth > 6)));
        stack->currMove = 65;
        stack->contHist = &contHist[NO_PIECE][0];
        int nullScore = -search<false>(-beta, -alpha, pos, depth - reduction, si, stack+1);
        stack->currMove = 0;
        pos.unmakeNullMove();

        if (nullScore >= beta)
            return nullScore;
    }

    Movepicker mp;
    while ((currentMove = pickNextMove<false>(mp, ttMove, pos, checkers, killers[stack->plysInSearch], mainHistory[pos.sideToMove], *(stack-2)->contHist)) != 0) {
        int from = extract<FROM>(currentMove);
        int to   = extract<TO>(currentMove);
        stack->contHist = &contHist[pos.pieceLocations[from]][to];

        int reductions = lmrReduction(depth, moveCount);
        int expectedDepth = std::max(depth - reductions, 1);

        if (!pos.isCapture(currentMove) && depth <= 5 && moveCount > 12 * depth)
            continue;

        if (!pvNode && !pos.isCapture(currentMove) && bestScore > -MAXMATE && depth <= 5 && stack->staticEval + 175 + 200 * expectedDepth <= alpha)
            continue;

        pos.makeMove(currentMove);
        stack->currMove = currentMove;
        si.nodeCount++;
        moveCount++;

        reductions -= pvNode;
        reductions = std::max(reductions, 1);

        if (depth > 2 && moveCount > 2) {
            score = -search<false>(-alpha - 1, -alpha, pos, depth - reductions, si, stack+1);

            if (!pvNode && score > alpha && reductions > 1)
                score = -search<false>(-alpha - 1, -alpha, pos, depth - 1, si, stack+1);

            if (pvNode && score > alpha && score < beta)
                score = -search<false>(-beta, -alpha, pos, depth - 1, si, stack+1);
        }else {
            if (!pvNode || moveCount > 1)
                score = -search<false>(-alpha - 1, -alpha, pos, depth - 1, si, stack+1);

            if (pvNode && ((score > alpha && score < beta) || moveCount == 1))
                score = -search<false>(-beta, -alpha, pos, depth - 1, si, stack+1);
        }

        pos.unmakeMove(currentMove);
        stack->currMove = 0;

        if (si.stop && !(ROOT && depth == (1 + check)))
            return DRAW;

        if (score > bestScore) {
            bestScore = score;
            bestMove = currentMove;

            if(score > alpha) {
                alpha = score;
                exact = true;

                if (score >= beta) {
                    if (!pos.isCapture(bestMove) && bestMove != killers[stack->plysInSearch][0]) {
                        killers[stack->plysInSearch][1] = killers[stack->plysInSearch][0];
                        killers[stack->plysInSearch][0] = bestMove;
                    }

                    if (!pos.isCapture(bestMove))
                        updateHistory(mainHistory[pos.sideToMove], bestMove, historyUpdates, depth, *(stack-2)->contHist, pos);

                    TT.save(tte, key, bestScore, LOWER, bestMove, depth, stack->plysInSearch);
                    return bestScore;
                }
            }
        }

        if (!pos.isCapture(currentMove))
            historyUpdates.push(currentMove);
    }

    if (ROOT && exact)
        si.bestRootMove = bestMove;

    if (bestScore == -INFINITE) {
        return checkers ? (-MATE + stack->plysInSearch) : DRAW;
    }

    TT.save(tte, key, bestScore, exact ? EXACT : UPPER, bestMove, depth, stack->plysInSearch);
    return bestScore;
}

int qsearch(int alpha, int beta, Position &pos, SearchInfo &si) {
    Move bestMove = 0;
    Move currentMove;
    bool check;
    int staticEval;
    int bestScore = staticEval = evaluate(pos);

    if (bestScore >= beta)
        return bestScore;

    Movepicker mp;
    while ((currentMove = pickNextMove<true>(mp, 0, pos, check)) != 0) {
        if (pos.isCapture(currentMove) && staticEval + PieceValuesSEE[pos.pieceLocations[extract<TO>(currentMove)]] + 150 <= alpha)
            continue;

        if (pos.isCapture(currentMove) && !see(pos, -101, currentMove))
            continue;

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
