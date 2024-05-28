#ifndef MOLYBDENUM_EVAL_H
#define MOLYBDENUM_EVAL_H

#include "Position.h"
#include "nnue.h"

inline int evaluate(Position &pos) {
    int nnue = calculate(pos.sideToMove) / 3;

    int adjustment = (defaultScale.weights[pos.matKey.id(pos.sideToMove) * 486 + pos.matKey.id(!pos.sideToMove)] * nnue) / 1024;

    return nnue - adjustment;
}

#endif //MOLYBDENUM_EVAL_H
