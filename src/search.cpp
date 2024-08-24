#include "Position.h"
#include "Movegen.h"
#include "search.h"
#include "eval.h"
#include "Constants.h"
#include "Movepicker.h"
#include "searchUtil.h"
#include <chrono>
#include <algorithm>

std::array<std::array<Move, 2>, STACKSIZE> killers;
SideFromToHist mainHistory;
ContHist continuationHistory;
std::array<std::array<Move, MAXDEPTH>, MAXDEPTH> pvMoves;
std::array<int, MAXDEPTH> pvLength;

u64 benchNodes = 0;

#include "tune.h"

#ifdef TUNE
Tune tune;
#endif

enum NodeType {
    Root, PVNode, NonPvNode
};

template<NodeType NT>
int search(int alpha, int beta, Position &pos, int depth, SearchInfo &si, SearchStack *stack);
int qsearch(int alpha, int beta, Position &pos, SearchInfo &si, SearchStack *stack);
int rootSearch(Position &pos, SearchInfo &si, SearchStack *stack, int depth);

int startSearch(Position &pos, searchTime &st, int maxDepth, Move &bestMove) {
    return iterativeDeepening(pos, st, maxDepth,bestMove);
}

void clearHistory() {
    TT.clear();
    memset(&mainHistory, 0, sizeof(mainHistory[0]) * mainHistory.size());
    memset(&continuationHistory, 0, sizeof(continuationHistory[0]) * continuationHistory.size());
    memset(&pvMoves, 0, sizeof(pvMoves[0]) * pvMoves.size());
    memset(&pvLength, 0, sizeof(pvLength[0]) * pvLength.size());
    memset(&killers, 0, sizeof(killers[0]) * killers.size());
}

int iterativeDeepening(Position  &pos, searchTime &st, int maxDepth, [[maybe_unused]] Move &bestMove) {
    SearchInfo si;
    si.st = st;

    return start(pos, si, maxDepth);
}

int start(Position &pos, SearchInfo &si, int depth) {
    std::array<SearchStack, STACKSIZE> stack;
    stack[0].contHist = &continuationHistory[NO_PIECE][0];
    stack[1].contHist = &continuationHistory[NO_PIECE][0];

    return rootSearch(pos, si, &stack[2], depth);
}

int rootSearch(Position &pos, SearchInfo &si, SearchStack *stack, int maxDepth) {     
    u64 ksq = pos.getPieces(pos.sideToMove, KING);     
    u64 checkers = attackersTo<false, false>(lsb(ksq), pos.getOccupied(), pos.sideToMove ? BLACK_PAWN : WHITE_PAWN, pos);
    u64 threats  = getThreats(pos, !pos.sideToMove);

    MoveList ml;     
    generateMoves<false>(pos, ml, checkers);

    std::array<int, 218> depths;
    std::array<int, 218> scores;

    scores.fill(-INFINITE);
    depths.fill(0);

    const float cpuct       = 2.35f;
    const float temperature = 4.17f;
    const float fpu         = 1.0f;

    pos.policyNet.scoreMovesList(ml, pos.sideToMove, pos.bitBoards, threats, temperature);

    stack->plysInSearch = 0;
    stack->quarterRed   = 0;
    stack->staticEval   = evaluate(pos);

    while (!stop<Soft>(si.st, si)) {
        int bestScore = -INFINITE;

        if (si.st.limit == Depth) {
            int total = 0;

            for (int i = 0; i < ml.length; i++)
                total += depths[i];

            if ((total / ml.length) >= maxDepth)
                break;
        }

        for (int i = 0; i < ml.length; i++)
            bestScore = std::max(bestScore, scores[i]);

        int idx = selectMove(ml, depths, scores, cpuct, fpu, si.nodeCount);

        Move move = ml.moves[idx].move;
        int  from = extract<FROM>(move);
        int  to   = extract<TO  >(move);
        int  pc   = pos.pieceOn(from);

        stack->currMove = move;
        stack->contHist = &continuationHistory[pc][to];

        pos.makeMove(move);
        si.nodeCount++;

        int alpha = -INFINITE;
        int beta  =  INFINITE;
        int delta = std::max(79 - depths[idx] * depths[idx], 23);

        if (depths[idx] >= 2) {
            alpha = scores[idx] - delta;
            beta  = scores[idx] + delta;
        }

        int score = -search<PVNode>(-beta, -alpha, pos, depths[idx]++, si, stack+1);

        while ((score >= beta || score <= alpha) && !stop<Hard>(si.st, si)) {
            delta *= 1.23;

            if (score >= beta)
                beta = std::max(score + delta, INFINITE);
            else
                alpha = std::max(score - delta, -INFINITE);

            si.selDepth = 0;

            score = -search<PVNode>(-beta, -alpha, pos, depths[idx]++, si, stack+1);
        }

        pos.unmakeMove(move);

        if (stop<Hard>(si.st, si) && depths[idx] != 1)
            break;

        scores[idx] = score;
    }

    int  bestScore = -INFINITE;
    Move bestMove  = NO_MOVE;

    for (int i = 0; i < ml.length; i++) {
        if (scores[i] > bestScore) {
            bestScore = scores[i];
            bestMove  = ml.moves[i].move;
        }
    }

    std::cout << "info depth 1 score cp " << bestScore << " nodes " << si.nodeCount << "\n";
    std::cout << "bestmove " << moveToString(bestMove) << std::endl;

    benchNodes += si.nodeCount; 

    return bestScore;
}

template<NodeType nt>
int search(int alpha, int beta, Position &pos, int depth, SearchInfo &si, SearchStack *stack) {
    constexpr bool ROOT = nt == Root;
    constexpr bool PvNode = nt == PVNode || ROOT;

    u64 ksq = pos.getPieces(pos.sideToMove, KING);
    u64 checkers = attackersTo<false, false>(lsb(ksq),pos.getOccupied(), pos.sideToMove ? BLACK_PAWN : WHITE_PAWN, pos);
    Move bestMove = 0, currentMove = 0, excluded = NO_MOVE;
    int bestScore = -INFINITE, score = -INFINITE, moveCount = 0, extensions = 0;
    bool exact = false, check = checkers, ttHit = false, improving, whatAreYouDoing;
    Stack<Move> historyUpdates;

    excluded = stack->excluded;

    if (stack->plysInSearch > si.selDepth)
        si.selDepth = stack->plysInSearch;
    
    stack->plysInSearch = ROOT ? 0 : (stack-1)->plysInSearch + 1;
    stack->quarterRed   = 0;
    (stack+1)->excluded = NO_MOVE;

    pvLength[stack->plysInSearch] = stack->plysInSearch;

    depth += check;

    if (stack->plysInSearch >= MAXDEPTH)
        return stack->staticEval;

    if (depth <= 0)
        return qsearch(alpha, beta, pos, si, stack);

    if (   !(si.nodeCount & 1023)
        && stop<Hard>(si.st, si))
        si.stop = true;

    if (si.stop && !(ROOT && depth == (1 + check)))
        return DRAW;

    if constexpr (!ROOT) {
        if (   pos.hasRepeated(stack->plysInSearch)
            || pos.plys50moveRule > 99
            || (pos.phase <= 3 && !(pos.getPieces(PAWN))))
            return 0;
    }

    if (   alpha > MAXMATE
        && MATE - stack->plysInSearch <= alpha)
        return alpha;

    int ttScore = 0, ttBound = UPPER, ttDepth = 0;
    Move ttMove = NO_MOVE;
    u64 key = pos.key();
    TTEntry* tte = TT.probe(key);

    if (   tte->key == key 
        && !excluded) {
        ttBound = tte->bound;
        ttMove  = tte->move;
        ttDepth = tte->depth;
        ttScore = tte->score;
        ttHit   = true;

        if (ttScore > MAXMATE)
            ttScore -= stack->plysInSearch;
        else if (ttScore < -MAXMATE)
            ttScore += stack->plysInSearch;
    }

    if (   !PvNode
        && ttHit
        && !excluded
        && ttDepth >= depth
        && (    ttBound == EXACT
            || (ttBound == LOWER && ttScore >= beta)
            || (ttBound == UPPER && ttScore <= alpha)))
        return ttScore;

    if (!excluded)
        stack->staticEval = evaluate(pos);

    improving       = stack->staticEval > (stack-2)->staticEval;
    whatAreYouDoing = stack->staticEval + (stack-1)->staticEval > 0;

    if (   !PvNode
        && !check
        && !excluded
        && depth < 10
        && stack->staticEval - (100 * depth - 164 * improving - 43 * whatAreYouDoing) >= beta
        && stack->staticEval >= beta)
        return stack->staticEval;

    if (   !PvNode
        && !check
        && !excluded
        && depth >= 2
        && (stack-1)->currMove != NULL_MOVE
        && stack->staticEval >= beta
        && beta > -MAXMATE) {

        u64 prefetchKey = key;
        updateKey(prefetchKey);
        __builtin_prefetch(TT.probe(prefetchKey));

        int reduction = std::min(depth, (4 + (stack->staticEval >= beta + 290) + (depth > 6)));

        pos.makeNullMove();

        stack->currMove = NULL_MOVE;
        stack->contHist = &continuationHistory[NO_PIECE][0];

        int nullScore = -search<NonPvNode>(-beta, -alpha, pos, depth - reduction, si, stack+1);

        pos.unmakeNullMove();

        if (nullScore >= beta && nullScore < MAXMATE)
            return nullScore;
    }

    Movepicker mp = Movepicker<false>(&pos, ttMove, 
                                            &killers[stack->plysInSearch], 
                                            &mainHistory[pos.sideToMove], 
                                            &*(stack-1)->contHist, 
                                            &*(stack-2)->contHist, checkers, ROOT);
    while ((currentMove = mp.pickMove())) {
        extensions = depth + moveCount <= 13 ? extensions : 0;
    
        if (currentMove == excluded)
            continue;

        int  from    = extract<FROM>(currentMove);
        int  to      = extract<TO>(currentMove);
        bool capture = pos.isCapture(currentMove);
        Piece pc     = pos.pieceOn(from);

        double red = lmrReduction(depth, moveCount, improving);
        int reductions = int(red);
        int expectedDepth = std::max(depth - reductions, 1);
        int history = (*(stack-1)->contHist)[pc][to] + mainHistory[pos.sideToMove][from][to];
        
        stack->quarterRed = (red - reductions) * 4;

        if (   !capture
            && bestScore > -MAXMATE
            && depth <= 4
            && moveCount > (3 + improving) * (11 * depth - ((stack-1)->quarterRed * 10) / 4) / 4)
            continue;

        if (   !PvNode
            && !capture
            && bestScore > -MAXMATE
            && depth <= 7
            && stack->staticEval + 192 + 212 * expectedDepth  - (198 * stack->quarterRed) / 4 <= alpha)
            continue;

        if (   !PvNode
            && bestScore > -MAXMATE
            && !capture
            && depth <= 6
            && history < -6011 * expectedDepth - (-6305 * stack->quarterRed) / 4)
            continue;

        if (   depth >= 8
            && !ROOT
            && ttHit
            && currentMove == ttMove
            && ttBound != UPPER
            && ttDepth >= depth - 3
            && !excluded) {
            
            int singDepth = depth / 2;
            int singBeta  = ttScore - 12 + std::min(si.rootMoveCount * 2, 12); 

            stack->excluded = ttMove;
            stack->currMove = NO_MOVE;

            score = search<nt>(singBeta - 1, singBeta, pos, singDepth, si, stack);

            stack->excluded = NO_MOVE;

            if (score < singBeta)
                extensions = 1;

            if (   score > singBeta
                && stack->currMove) 
                mp.setPrioMove(stack->currMove);
        }

        prefetchTTEntry(pos, pc, from, to, capture);

        pos.makeMove(currentMove);

        stack->currMove = currentMove;
        stack->contHist = &continuationHistory[pc][to];

        si.nodeCount++;
        moveCount++;

        if constexpr (ROOT)
            si.rootMoveCount = moveCount;

        history += (*(stack-2)->contHist)[pc][to];

        reductions -= PvNode;
        reductions -= history > 0 ? history / 4085 : history / 25329;
        reductions = std::max(reductions, 0);

        if (depth > 1 && moveCount > 2) {
            score = -search<NonPvNode>(-alpha - 1, -alpha, pos, depth - 1 - reductions + extensions, si, stack+1);

            if (score > alpha && reductions > 0) {
                bool nightmare = bestScore < alpha - 100 && moveCount > 3;
                score = -search<NonPvNode>(-alpha - 1, -alpha, pos, depth - 1 + extensions + nightmare, si, stack+1);
            }

            if (PvNode && score > alpha && score < beta)
                score = -search<PVNode>(-beta, -alpha, pos, depth - 1 + extensions, si, stack+1);
        } else {
            if (!PvNode || moveCount > 1)
                score = -search<NonPvNode>(-alpha - 1, -alpha, pos, depth - 1 + extensions, si, stack+1);

            if (PvNode && ((score > alpha && score < beta) || moveCount == 1))
                score = -search<PVNode>(-beta, -alpha, pos, depth - 1 + extensions, si, stack+1);
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

                if (ROOT || (score >= beta && !pos.isCapture(bestMove))) {
                    if (bestMove != killers[stack->plysInSearch][0]) {
                        killers[stack->plysInSearch][1] = killers[stack->plysInSearch][0];
                        killers[stack->plysInSearch][0] = bestMove;
                    }
                }

                if (score >= beta) {
                    if (!pos.isCapture(bestMove))
                        updateHistory(mainHistory[pos.sideToMove], *(stack-1)->contHist, *(stack-2)->contHist, bestMove, historyUpdates, depth, pos, (stack-1)->currMove && (stack-1)->currMove != NULL_MOVE, (stack-2)->currMove && (stack-2)->currMove != NULL_MOVE);

                    TT.save(tte, key, bestScore, LOWER, bestMove, depth, stack->plysInSearch);
                    return bestScore;
                }

                pvMoves[stack->plysInSearch][stack->plysInSearch] = bestMove;

                for (int nextPly = stack->plysInSearch + 1; nextPly < pvLength[stack->plysInSearch + 1]; nextPly++)
                    pvMoves[stack->plysInSearch][nextPly] = pvMoves[stack->plysInSearch + 1][nextPly];

                pvLength[stack->plysInSearch] = pvLength[stack->plysInSearch + 1];
            }
        }

        if (!pos.isCapture(currentMove))
            historyUpdates.push(currentMove);
    }

    if (ROOT && exact)
        si.bestRootMove = bestMove;

    if (bestScore == -INFINITE)
        return excluded ? alpha : checkers ? (-MATE + stack->plysInSearch) : DRAW;

    if (   !PvNode
        && ttDepth > depth
        && ttBound == (ttScore >= bestScore ? LOWER : UPPER))
        bestScore = ttScore;

    if (!excluded)
        TT.save(tte, key, bestScore, exact ? EXACT : UPPER, bestMove, depth, stack->plysInSearch);

    return bestScore;
}

int qsearch(int alpha, int beta, Position &pos, SearchInfo &si, SearchStack *stack) {

#ifdef TUNE
    std::array<int, 13> PieceValuesSEE = {tune.SEEPawn, tune.SEEKnight, tune.SEEBishop, tune.SEERook, tune.SEEQueen, 0, tune.SEEPawn, tune.SEEKnight, tune.SEEBishop, tune.SEERook, tune.SEEQueen, 0, 0};
#endif

    Move currentMove;
    int staticEval;

    stack->plysInSearch = (stack-1)->plysInSearch + 1;

    if (stack->plysInSearch >= MAXDEPTH)
        return evaluate(pos);

    int ttScore = 0, ttBound = UPPER;
    bool ttHit = false;
    Move ttMove = NO_MOVE;
    u64 key = pos.key();
    TTEntry* tte = TT.probe(key);

    if (tte->key == key) {
        ttBound = tte->bound;
        ttScore = tte->score;
        ttHit   = true;

        if (tte->move && pos.pieceOn(extract<TO>(tte->move)) != NO_PIECE)
            ttMove = tte->move;

        if (ttScore > MAXMATE)
            ttScore -= stack->plysInSearch;
        else if (ttScore < -MAXMATE)
            ttScore += stack->plysInSearch;
    }

    if (   ttHit
        && (    ttBound == EXACT
            || (ttBound == LOWER && ttScore >= beta)
            || (ttBound == UPPER && ttScore <= alpha)))
            return ttScore;

    int bestScore = staticEval = evaluate(pos);    

    if (bestScore >= beta)
        return bestScore;

    Movepicker mp = Movepicker<true>(&pos, ttMove);

    while ((currentMove = mp.pickMove())) {
        int from     = extract<FROM>(currentMove);
        int to       = extract<TO  >(currentMove);
        int pc       = pos.pieceOn(from);
        int captured = pos.pieceOn(to);

        if (   captured != NO_PIECE
            && staticEval + PieceValuesSEE[captured] + 147 <= alpha)
            continue;

        if (   captured != NO_PIECE
            && !see(pos, -108, currentMove))
            continue;

        prefetchTTEntry(pos, pc, from, to, captured != NO_PIECE);

        pos.makeMove(currentMove);
        si.nodeCount++;

        int score = -qsearch(-beta, -alpha, pos, si, stack+1);

        pos.unmakeMove(currentMove);

        if (score > bestScore) {
            bestScore = score;

            if (score > alpha) {
                alpha = score;

                if (score >= beta)
                    return score;
            }
        }
    }

    return bestScore;
}
