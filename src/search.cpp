#include "Position.h"
#include "Movegen.h"
#include "search.h"
#include "eval.h"
#include "Constants.h"
#include "Movepicker.h"
#include "searchUtil.h"
#include "thread.h"
#include "Movegen.h"

#include <chrono>
#include <algorithm>

#include "tune.h"

#ifdef TUNE
Tune tune;
#endif

bool prettyprint = false;

std::string SearchState::outputWDL(Position &pos) {
    for (int i = 0; i < pvLength[0]; i++)
        pos.makeMove(pvMoves[0][i]);

    std::tuple<float, float, float> wdl = pos.net.getWDL(pos.sideToMove);

    for (int i = pvLength[0] - 1; i >= 0; i--)
        pos.unmakeMove(pvMoves[0][i]);

    std::string values[3] = {std::to_string(int(std::get<2>(wdl) * 1000)), 
                             std::to_string(int(std::get<1>(wdl) * 1000)),
                             std::to_string(int(std::get<0>(wdl) * 1000))};

    bool flip = pvLength[0] % 2 == 0;

    return  " wdl " + values[flip ? 0 : 2]
              + " " + values[1           ]
              + " " + values[flip ? 2 : 0];
}

int SearchState::startSearch(Position &pos, SearchTime &st, int maxDepth, Move &bestMove) {
    return iterativeDeepening(pos, st, maxDepth, bestMove);
}

void SearchState::clearHistory() {
    TT.clear();
    memset(&mainHistory, 0, sizeof(mainHistory[0]) * mainHistory.size());
    memset(&continuationHistory, 0, sizeof(continuationHistory[0]) * continuationHistory.size());
    memset(&pvMoves, 0, sizeof(pvMoves[0]) * pvMoves.size());
    memset(&pvLength, 0, sizeof(pvLength[0]) * pvLength.size());
    memset(&killers, 0, sizeof(killers[0]) * killers.size());
}

int SearchState::iterativeDeepening(Position  &pos, SearchTime &st, int maxDepth, [[maybe_unused]] Move &bestMove) {
    int score = 0;
    int prevScore = 0;
    si.clear();
    si.st = st;

    //prettyInitial();

    for (int depth = 1; depth != maxDepth; depth++) {
        score = aspirationWindow(score, pos, si, depth);

        if (si.stop.load(std::memory_order_relaxed))
            break;

        if (thread->id())
            continue;

#ifndef DATAGEN
        if (prettyprint)
            prettyPrint(pos, si, score, depth);
        else {
            std::string uciOutput;
            auto searchTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - si.st.searchStart).count();
            uciOutput += "info depth ";
            uciOutput += std::to_string(depth);

            uciOutput += " seldepth ";
            uciOutput += std::to_string(si.selDepth);

            uciOutput += " currmove ";
            uciOutput += moveToString(si.bestRootMove);

            uciOutput += " score ";
            uciOutput += abs(score) > MAXMATE ? "mate " : "cp ";
            uciOutput += std::to_string(abs(score) > MAXMATE ? mateInPlies(score) : score);

            u64 nodeCount = thread->threads->nodes();

            uciOutput += " nodes ";
            uciOutput += std::to_string(nodeCount);

            uciOutput += " time ";
            uciOutput += std::to_string(searchTime);

            uciOutput += " nps ";
            uciOutput += std::to_string((nodeCount / std::max(int(searchTime), 1)) * 1000);

            uciOutput += outputWDL(pos);

            uciOutput += " pv ";
            for (int i = 0; i < pvLength[0]; i++)
                uciOutput += moveToString(pvMoves[0][i]) + " ";

            std::cout << uciOutput << std::endl;
        }

        if (stop<Soft>(st, si)) {
            thread->threads->stop();
            break;
        }
    }

    thread->searching.store(false, std::memory_order_relaxed);

    if (!thread->id()) {
        while (!thread->threads->done()) {}
        std::cout << "bestmove " << moveToString(si.bestRootMove) << std::endl;
    }

#else
    prevScore = score;
    }
    bestMove = si.bestRootMove;
#endif
    return prevScore;
}

int SearchState::aspirationWindow(int prevScore, Position &pos, SearchInfo &si, int depth) {
    int delta = std::clamp(79 - depth * depth, 23, 34);
    int alpha = -INFINITE;
    int beta  =  INFINITE;

    if (depth >= 2) {
        alpha = std::max(-INFINITE, prevScore - delta);
        beta  = std::min( INFINITE, prevScore + delta);
    }

    std::array<SearchStack, STACKSIZE> stack;
    stack[0].contHist = &continuationHistory[NO_PIECE][0];
    stack[1].contHist = &continuationHistory[NO_PIECE][0];

    si.selDepth = 0;

    int score = search<Root>(alpha, beta, pos, depth, si, &stack[2]);

    while ((score >= beta || score <= alpha) && !stop<Hard>(si.st, si)) {
        delta *= 1.23;

        if (score >= beta)
            beta = std::max(score + delta, INFINITE);
        else
            alpha = std::max(score - delta, -INFINITE);

        si.selDepth = 0;

        score = search<Root>(alpha, beta, pos, depth, si, &stack[2]);
    }

    return score;
}

template<NodeType nt>
int SearchState::search(int alpha, int beta, Position &pos, int depth, SearchInfo &si, SearchStack *stack) {
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
        return qsearch<nt>(alpha, beta, pos, si, stack);

    if (   !(si.nodeCount.load(std::memory_order_relaxed) & 1023)
        && stop<Hard>(si.st, si))
        thread->threads->stop();

    if (si.stop.load(std::memory_order_relaxed) && !(ROOT && depth == (1 + check)))
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

    if (   depth >= 8
        && !ROOT
        && ttHit
        && ttBound != UPPER
        && ttDepth >= depth - 3
        && !excluded
        && mp.isLegal(ttMove)) {
            
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
        {
            bool replaceTTM = score >= beta && ttScore <= alpha && ttBound == LOWER;

            Move prioMove  = replaceTTM ? ttMove : stack->currMove;
            Move newTTMove = replaceTTM ? stack->currMove : ttMove;

            mp.setPrioMove(prioMove);
            mp.setTTMove(ttMove = newTTMove);
        }
    }

    while ((currentMove = mp.pickMove())) {
        extensions = !moveCount || (depth + moveCount <= 13) ? extensions : 0;
    
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

        prefetchTTEntry(pos, pc, from, to, capture);

        pos.makeMove(currentMove);

        stack->currMove = currentMove;
        stack->contHist = &continuationHistory[pc][to];

        si.nodeCount.fetch_add(1, std::memory_order_relaxed);
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

        if (si.stop.load(std::memory_order_relaxed) && !(ROOT && depth == (1 + check)))
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

template<NodeType nt>
int SearchState::qsearch(int alpha, int beta, Position &pos, SearchInfo &si, SearchStack *stack) {

#ifdef TUNE
    std::array<int, 13> PieceValuesSEE = {tune.SEEPawn, tune.SEEKnight, tune.SEEBishop, tune.SEERook, tune.SEEQueen, 0, tune.SEEPawn, tune.SEEKnight, tune.SEEBishop, tune.SEERook, tune.SEEQueen, 0, 0};
#endif

    constexpr bool PvNode = nt != NonPvNode;

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
        && !PvNode
        && (    ttBound == EXACT
            || (ttBound == LOWER && ttScore >= beta)
            || (ttBound == UPPER && ttScore <= alpha)))
            return ttScore;

    int bestScore = staticEval = evaluate(pos);    

    pvLength[stack->plysInSearch] = stack->plysInSearch;

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
        si.nodeCount.fetch_add(1, std::memory_order_relaxed);

        int score = -qsearch<nt>(-beta, -alpha, pos, si, stack+1);

        pos.unmakeMove(currentMove);

        if (score > bestScore) {
            bestScore = score;

            if (score > alpha) {
                alpha = score;

                if (score >= beta)
                    return score;

                pvMoves[stack->plysInSearch][stack->plysInSearch] = currentMove;

                for (int nextPly = stack->plysInSearch + 1; nextPly < pvLength[stack->plysInSearch + 1]; nextPly++)
                    pvMoves[stack->plysInSearch][nextPly] = pvMoves[stack->plysInSearch + 1][nextPly];

                pvLength[stack->plysInSearch] = pvLength[stack->plysInSearch + 1];
            }
        }
    }

    return bestScore;
}

void prettyInitial() {
    std::cout << std::endl;
    std::cout << " depth       time       nodes      speed        score" << std::endl;
    std::cout << "_______________________________________________________________________________" << std::endl;
}

// This is pretty print, not pretty code
void SearchState::prettyPrint(Position &pos, SearchInfo &si, int s, int de) {
    auto searchTime             = std::chrono::steady_clock::now() - si.st.searchStart;
    auto searchTimeMinutes      = std::chrono::duration_cast<std::chrono::minutes     >(searchTime).count();
    auto searchTimeSeconds      = std::chrono::duration_cast<std::chrono::seconds     >(searchTime).count();
    auto searchTimeMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(searchTime).count();
    u64 nodes                   = thread->threads->nodes();
    u64 nodesPerMillisecond     = (nodes / (searchTimeMilliseconds + 1));
    std::string temp            = std::to_string(abs(s) % 100);
    std::vector<std::string> sanPV;

    zeroPaddString<Front>(temp, 2);

    for (int i = 0; i < pvLength[0]; i++) {
        sanPV.push_back(pos.moveToSAN(pvMoves[0][i], getAttacks(typeOf(pos.pieceOn(extract<FROM>(pvMoves[0][i]))),
                                                                extract<TO>(pvMoves[0][i]), 
                                                                pos.getOccupied(), 
                                                                pos.sideToMove ^ bool(i + 1))));
        pos.makeMove(pvMoves[0][i]);
    }

    std::tuple<float, float, float> wdl = pos.net.getWDL(pos.sideToMove);

    for (int i = pvLength[0] - 1; i >= 0; i--)
        pos.unmakeMove(pvMoves[0][i]);

    bool flip = pvLength[0] & 1;
    float w   = flip ? std::get<0>(wdl) : std::get<2>(wdl);
    float l   = flip ? std::get<2>(wdl) : std::get<0>(wdl);
    float d   = std::get<1>(wdl);
    
    std::string pretty        = "";
    std::string depth         = std::to_string(de);
    std::string seperator     = "|";
    std::string selDepth      = std::to_string(si.selDepth);
    std::string megaNodes     = std::to_string(nodes / 1000000);
    std::string decimalNodes  = std::to_string((nodes % 100000) / 10000);
    std::string dot           = ".";
    std::string mn            = "Mn";
    std::string minutes       = std::to_string(searchTimeMinutes % 60);
    std::string seconds       = std::to_string(searchTimeSeconds % 60);
    std::string milliseconds  = std::to_string((searchTimeMilliseconds % 10000) / 1000);
    std::string mnps          = std::to_string(nodesPerMillisecond / 1000);
    std::string decimalMnps   = std::to_string(nodesPerMillisecond % 1000 / 100);
    std::string scorePrefix   = s > 0 ? "+" : "-";
    std::string matePrefix    = abs(s) >= MAXMATE ? "M" : "";
    std::string score         = std::to_string(abs(s) >= MAXMATE ? mateInPlies(s) : (abs(s) / 100));
    std::string scoreDecimal  = abs(s) >= MAXMATE ? "" : ("." + temp);
    std::string completeScore = scorePrefix + matePrefix + score + scoreDecimal; 
    std::string wPercentage   = std::to_string(int(std::roundf(w * 100.0f))) + "% ";
    std::string dPercentage   = std::to_string(int(std::roundf(d * 100.0f))) + "% ";
    std::string lPercentage   = std::to_string(int(std::roundf(l * 100.0f))) + "% ";

    paddString<Front>(depth    , 3);
    paddString<Back >(selDepth , 3);
    paddString<Front>(megaNodes, 5);
    paddString<Front>(minutes, 5);
    zeroPaddString<Front>(seconds, 2);
    paddString<Front>(mnps, 6);
    paddString<Front>(completeScore, 11);
    paddString<Back>(completeScore, completeScore.size() + 6);
    paddString<Front>(wPercentage, 5);
    paddString<Front>(dPercentage, 5);
    paddString<Front>(lPercentage, 5);

    wPercentage = "W:" + wPercentage;
    dPercentage = "D:" + dPercentage;
    lPercentage = "L:" + lPercentage;

    paddString<Back>(lPercentage, 13);

    int evalColorTones[11] = {88, 124, 196, 202, 208, 229, 106, 64, 76, 28, 22};
    int evalColorIndex     = std::clamp(5+ (s / 25), 0, 10);

    colorString<Foreground>(wPercentage, 15);
    colorString<Foreground>(dPercentage, 8);
    colorString<Foreground>(lPercentage, 16);
    colorString<Foreground>(completeScore, evalColorTones[evalColorIndex]);

    colorString<Foreground>(sanPV[0], 15);
    sanPV[0] = "\033[1m" + sanPV[0] + "\033[0m";

    for (size_t i = 1; i < sanPV.size(); i++)
        colorString<Foreground>(sanPV[i], 242);

    // Eval bar for wdl? Current issue inability to find a way to display percentages on top of bars in a way that make sense

    pretty += depth + seperator + selDepth;
    pretty += minutes + "m " + seconds + "." + milliseconds + "s ";
    pretty += megaNodes + dot + decimalNodes + mn;
    pretty += mnps + dot + decimalMnps + "Mn/s";

    colorString<Foreground>(pretty, 246);

    pretty += completeScore;
    pretty += wPercentage + dPercentage + lPercentage;

    for (auto &move : sanPV)
        pretty += move;

    std::cout << pretty << std::endl;
}
