#ifndef MOLYBDENUM_TIMEMANAGEMENT_H
#define MOLYBDENUM_TIMEMANAGEMENT_H

#include <chrono>
#include <array>

static int moveOverHead = 10;

enum SearchLimit {
    Time, Nodes, Depth
};

enum LimitType {
    Hard, Soft
};

struct searchTime{
    std::array<std::chrono::milliseconds, 2> thinkingTime = {std::chrono::milliseconds::max(), std::chrono::milliseconds::max()};
    std::chrono::time_point<std::chrono::steady_clock> searchStart = std::chrono::steady_clock::now();
    std::array<uint64_t, 2> nodeLimit = {18446744073709551615ull, 18446744073709551615ull};
    SearchLimit limit = Time;
};

searchTime calcThinkingTime(int timeLeft, int increment, int movesToGo = 6000);

inline void setOverhead(int value) {
    moveOverHead = value;
}

#endif //MOLYBDENUM_TIMEMANAGEMENT_H
