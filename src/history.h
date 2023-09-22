#ifndef MOLYBDENUM_HISTORY_H
#define MOLYBDENUM_HISTORY_H

#include <array>
#include "Move.h"

void updateHistory(std::array<std::array<int, 64>, 64> &history, Move bestMove, Stack<Move> movesToUpdate, int depth) {
    int from = extract<FROM>(bestMove);
    int to   = extract<TO  >(bestMove);

    history[from][to] += depth * depth * 2;

    while (movesToUpdate.getSize()) {
        Move move = movesToUpdate.pop();
        from = extract<FROM>(move);
        to   = extract<TO  >(move);

        history[from][to] -= depth * depth;
    }
}

#endif //MOLYBDENUM_HISTORY_H
