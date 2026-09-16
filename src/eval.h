#ifndef MOLYBDENUM_EVAL_H
#define MOLYBDENUM_EVAL_H

#include "Position.h"
#include "nnue.h"

inline int evaluate(Position &pos) {
    int val = pos.sideToMove == WHITE ? pos.net->calculate<WHITE>(pos)
                                      : pos.net->calculate<BLACK>(pos);

    return std::clamp(val, -MAXMATE + 1, MAXMATE - 1);
}           

#endif //MOLYBDENUM_EVAL_H
