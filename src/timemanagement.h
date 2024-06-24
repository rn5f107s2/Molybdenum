#ifndef MOLYBDENUM_TIMEMANAGEMENT_H
#define MOLYBDENUM_TIMEMANAGEMENT_H

#include <chrono>
#include <array>

extern int moveOverHead;

enum SearchLimit {
    Time, Nodes, Depth
};

enum LimitType {
    Hard, Soft
};

struct SearchTime{
    std::array<std::chrono::milliseconds, 2> thinkingTime = {std::chrono::milliseconds::max(), std::chrono::milliseconds::max()};
    std::chrono::time_point<std::chrono::steady_clock> searchStart = std::chrono::steady_clock::now();
    uint64_t nodeLimit = 18446744073709551615ull;
    SearchLimit limit = Time;

    void calcThinkingTime(int timeLeft, int increment, int movesToGo = 6000);
};

void setOverhead(int value);

#endif //MOLYBDENUM_TIMEMANAGEMENT_H
