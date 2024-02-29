#ifndef MOLYBDENUM_SEARCHUTIL_H
#define MOLYBDENUM_SEARCHUTIL_H

#include <array>
#include "Move.h"
#include "Movegen.h"

#include "tune.h"

#ifdef TUNE
extern Tune tune;
#endif

using FromToHist = std::array<std::array<int, 64>, 64>;
using PieceToHist = std::array<std::array<int, 64>, 13>;
using SideFromToHist = std::array<FromToHist, 2>;
using ContHist = std::array<std::array<PieceToHist, 64>, 13>;

//const int histLimits = 2 << tune.HistLimit;

inline void prefetchTTEntry(Position &pos, int pc, int from, int to, bool capture) {
    u64 prefetchKey = pos.key();
    updateKey(pc, from, prefetchKey);
    updateKey(pc, to, prefetchKey);
    updateKey(prefetchKey);

    if (capture)
        updateKey(pos.pieceOn(to), to, prefetchKey);

    __builtin_prefetch(TT.probe(prefetchKey));
}

inline void updateHistory(FromToHist &history, PieceToHist &contHist, PieceToHist  &contHist2, Move bestMove, Stack<Move> &movesToUpdate, int depth, Position &pos, const bool updateCont, const bool updateCont2) {
    int from = extract<FROM>(bestMove);
    int to   = extract<TO  >(bestMove);
    int pc   = pos.pieceOn(from);
    int bonus = std::min(depth * depth * tune.HistDepthMult, tune.HistMax);
    int malus = -bonus;

    int histLimits = 2 << tune.HistLimit;

    history[from][to] += bonus - history [from][to] * abs(bonus) / histLimits;
    if (updateCont)
        contHist[pc][to]  += bonus - contHist[pc][to] * abs(bonus) / histLimits;
    if (updateCont2)
        contHist2[pc][to]  += bonus - contHist2[pc][to] * abs(bonus) / histLimits;

    while (movesToUpdate.getSize()) {
        Move move = movesToUpdate.pop();
        from = extract<FROM>(move);
        to   = extract<TO  >(move);
        pc   = pos.pieceOn(from);

        history[from][to] += malus - history [from][to] * abs(malus) / histLimits;
        if (updateCont)
            contHist[pc][to] += malus - contHist[pc][to] * abs(malus) / histLimits;
        if (updateCont2)
            contHist2[pc][to] += malus - contHist2[pc][to] * abs(malus) / histLimits;
    }
}

#ifndef TUNE
    static const std::array<int, 13> PieceValuesSEE = {81, 257, 325, 491, 972, 0, 81, 257, 325, 491, 972, 0, 0};
#endif

inline bool see(Position &pos, int threshold, Move move) {

#ifdef TUNE
    std::array<int, 13> PieceValuesSEE = {tune.SEEPawn, tune.SEEKnight, tune.SEEBishop, tune.SEERook, tune.SEEQueen, 0, tune.SEEPawn, tune.SEEKnight, tune.SEEBishop, tune.SEERook, tune.SEEQueen, 0, 0};
#endif

    int flag = extract<FLAG>(move);
    int to   = extract<TO  >(move);
    int from = extract<FROM>(move);

    if (flag == ENPASSANT || flag == CASTLING)
        return threshold >= 0;

    int trade;
    Color us = pos.sideToMove;
    Color stm = us;
    PieceType stage = PAWN;
    std::array<int, 2> stages = {PAWN - 1, PAWN - 1};
    std::array<u64, 2> attackers = {0, 0};
    std::array<u64, 2> prioAttackers = {0, 0};
    std::array<int, 2> prioStage = {0, 0};
    u64 blockers = (pos.getOccupied()) ^ (1ULL << from);

    trade = PieceValuesSEE[pos.pieceOn(to)] - threshold;
    if (trade < 0)
        return false;

    trade -= PieceValuesSEE[pos.pieceOn(from)];
    if (trade >= 0)
        return true;

    while (true) {
        stm = !stm;

        if (prioAttackers[stm]) {
            popLSB(prioAttackers[stm]);
            trade = -trade - 1 - PieceValuesSEE[stages[stm]];

            if (trade >= 0)
                break;

            continue;
        }

        while (!attackers[stm] && stages[stm] < KING + 1) {
            stage = PieceType(++stages[stm]);
            attackers[stm] |= getAttacks(stages[stm], to, blockers, !stm) & pos.getPieces(stm, stage) & blockers;
        }

        if (!attackers[stm])
            break;

        blockers ^= 1ULL << popLSB(attackers[stm]);
        trade = -trade - 1 - PieceValuesSEE[stages[stm]];

        if (trade >= 0) {
            if (stages[stm] == KING && attackersTo<true, false>(to, blockers, stm ? BLACK_PAWN : WHITE_PAWN, pos) & blockers)
                break;

            return stm == us;
        }

        if (stages[stm] == QUEEN) {
            prioAttackers[stm] |= getAttacks<ROOK  >(to, blockers) & blockers & pos.getPieces<ROOK>(stm);
            if (prioAttackers[stm])
                prioStage[stm] = ROOK;

            prioAttackers[stm] |= getAttacks<BISHOP>(to, blockers) & blockers & pos.getPieces<BISHOP>(stm);
            if (prioAttackers[stm])
                prioStage[stm] = BISHOP;
        }

        if (stages[stm] == ROOK || stages[stm] == BISHOP || stages[stm] == QUEEN)
            attackers[stm] |= getAttacks(stages[stm], to, blockers) & blockers & pos.getPieces(stm, stage);
    }

    return stm != us;
}

#endif //MOLYBDENUM_SEARCHUTIL_H
