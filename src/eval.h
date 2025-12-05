#ifndef MOLYBDENUM_EVAL_H
#define MOLYBDENUM_EVAL_H

#include "Position.h"
#include "nnue.h"

inline int evaluate(Position &pos) {
    return pos.sideToMove == WHITE ? pos.net->calculate<WHITE>(pos.getOccupied(), &pos.pieceLocations[0])
                                   : pos.net->calculate<BLACK>(__builtin_bswap64(pos.getOccupied()), &pos.pieceLocations[0]);
}           

#endif //MOLYBDENUM_EVAL_H
