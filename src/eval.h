#ifndef MOLYBDENUM_EVAL_H
#define MOLYBDENUM_EVAL_H

#include "Position.h"

int evaluate(Position &pos);
int evalPSQT(Position &pos, int phase);
int getGamePhase(Position &pos);

#endif //MOLYBDENUM_EVAL_H
