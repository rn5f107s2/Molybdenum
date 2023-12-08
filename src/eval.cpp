#include "Position.h"
#include "eval.h"
#include "PSQT.h"
#include "nnue.h"

int evaluate(Position &pos) {
    return calculate(pos.sideToMove) / 3;
}