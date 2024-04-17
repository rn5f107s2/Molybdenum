#ifndef MOLYBDENUM_EVAL_H
#define MOLYBDENUM_EVAL_H

#include "Position.h"
#include "nnue.h"

inline int evaluate(Position &pos) {
    return calculate(pos.sideToMove, pos.bucketIndex()) / 3;
}

#endif //MOLYBDENUM_EVAL_H
