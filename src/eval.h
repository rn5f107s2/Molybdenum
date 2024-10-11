#ifndef MOLYBDENUM_EVAL_H
#define MOLYBDENUM_EVAL_H

#include "Position.h"
#include "nnue.h"

inline int evaluate(Position &pos) {
    pos.net.initAccumulator(pos.bitBoards, pos.sideToMove);
    return pos.net.calculate(pos.sideToMove);
}

#endif //MOLYBDENUM_EVAL_H
