#ifndef MOLYBDENUM_TUNER_H
#define MOLYBDENUM_TUNER_H

#include "../Position.h"

#define tune
#ifdef tune

int getEval();
int tuneQ(int alpha, int beta, Position &pos);
int tuneEval(Position &pos);

#endif
#endif //MOLYBDENUM_TUNER_H
