#ifndef MOLYBDENUM_SEARCH_H
#define MOLYBDENUM_SEARCH_H

#include "Position.h"
#include "timemanagement.h"
#include <chrono>

int startSearch(Position &pos, searchTime &st);
int iterativeDeepening(Position  &pos, searchTime &st);

struct SearchInfo {
    int nodeCount = 0;
    Move bestRootMove = 0;
    bool stop = false;
    searchTime st;
};

#endif //MOLYBDENUM_SEARCH_H
