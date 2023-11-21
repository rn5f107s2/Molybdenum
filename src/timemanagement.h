#ifndef MOLYBDENUM_TIMEMANAGEMENT_H
#define MOLYBDENUM_TIMEMANAGEMENT_H

#include <chrono>

const int moveOverHead = 10;

enum SearchLimit {
    Time, Nodes, Depth
};

struct searchTime{
    std::chrono::milliseconds thinkingTime = std::chrono::milliseconds::max();
    std::chrono::time_point<std::chrono::steady_clock> searchStart = std::chrono::steady_clock::now();
    uint64_t nodeLimit = 18446744073709551615ull;
    SearchLimit limit = Time;
};

searchTime calcThinkingTime(int timeLeft, int increment, int movesToGo = 6000);

#endif //MOLYBDENUM_TIMEMANAGEMENT_H
