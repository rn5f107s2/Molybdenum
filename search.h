#ifndef MOLYBDENUM_SEARCH_H
#define MOLYBDENUM_SEARCH_H

#include "Position.h"

int startSearch(Position &pos);
int iterativeDeepening(Position  &pos);

struct SearchInfo {
    int nodeCount = 0;
    Move bestRootMove = 0;
};

#endif //MOLYBDENUM_SEARCH_H
