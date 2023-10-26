#ifndef MOLYBDENUM_SEARCHUTIL_H
#define MOLYBDENUM_SEARCHUTIL_H

#include <array>
#include "Move.h"

inline void updateHistory(std::array<std::array<int, 64>, 64> &history, Move bestMove, Stack<Move> movesToUpdate, int depth) {
    int from = extract<FROM>(bestMove);
    int to   = extract<TO  >(bestMove);
    int bonus = std::max(depth * depth * 16, 1536);
    int malus = -bonus;

    history[from][to] += bonus - history[from][to] * abs(bonus) / 100000;

    while (movesToUpdate.getSize()) {
        Move move = movesToUpdate.pop();
        from = extract<FROM>(move);
        to   = extract<TO  >(move);

        history[from][to] += malus - history[from][to] * abs(malus) / 100000;
    }
}

static const std::array<int, 13> PieceValuesSEE = {100, 300, 300, 500, 900, 0, 100, 300, 300, 500, 900, 0, 0};

inline bool see(Position &pos, int threshold, Move move) {
    int flag = extract<FLAG>(move);
    int to   = extract<TO  >(move);
    int from = extract<FROM>(move);

    if (flag == ENPASSANT || flag == CASTLING)
        return threshold >= 0;

    int trade;
    bool us = pos.sideToMove;
    bool stm = us;
    std::array<int, 2> stages = {PAWN - 1, PAWN - 1};
    std::array<u64, 2> attackers = {0, 0};
    std::array<u64, 2> prioAttackers = {0, 0};
    std::array<int, 2> prioStage = {0, 0};
    u64 blockers = (pos.getOccupied()) ^ (1ULL << from);

    trade = PieceValuesSEE[pos.pieceLocations[to]] - threshold;
    if (trade < 0)
        return false;

    trade -= PieceValuesSEE[pos.pieceLocations[from]];
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
            stages[stm]++;
            attackers[stm] |= getAttacks(stages[stm], to, blockers, !stm) & pos.bitBoards[stages[stm] + 6 * !stm] & blockers;
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
            prioAttackers[stm] |= getAttacks<ROOK  >(to, blockers) & blockers & pos.bitBoards[ROOK   + 6 * !stm];
            if (prioAttackers[stm])
                prioStage[stm] = ROOK;

            prioAttackers[stm] |= getAttacks<BISHOP>(to, blockers) & blockers & pos.bitBoards[BISHOP + 6 * !stm];
            if (prioAttackers[stm])
                prioStage[stm] = BISHOP;
        }

        if (stages[stm] == ROOK || stages[stm] == BISHOP || stages[stm] == QUEEN)
            attackers[stm] |= getAttacks(stages[stm], to, blockers) & blockers & pos.bitBoards[stages[stm] + 6 * !stm];
    }

    return stm != us;
}

#endif //MOLYBDENUM_SEARCHUTIL_H
