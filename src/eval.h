#ifndef MOLYBDENUM_EVAL_H
#define MOLYBDENUM_EVAL_H

#include "Position.h"

int evaluate(Position &pos);
int evalPSQT(Position &pos, int phase);
int getGamePhase(Position &pos);

constexpr std::array<int, 6> gamePhaseValues = {0, 2, 3, 5, 10, 0};
constexpr int maxPhase = gamePhaseValues[KNIGHT] * 4
                       + gamePhaseValues[BISHOP] * 4
                       + gamePhaseValues[ROOK  ] * 4
                       + gamePhaseValues[QUEEN ] * 2;
#endif //MOLYBDENUM_EVAL_H
