#ifndef MOLYBDENUM_TIMEMANAGEMENT_H
#define MOLYBDENUM_TIMEMANAGEMENT_H

#include <chrono>

const int moveOverHead = 10;

struct searchTime{
    std::chrono::milliseconds thinkingTime;
    std::chrono::time_point<std::chrono::steady_clock> searchStart = std::chrono::steady_clock::now();
};

searchTime calcThinkingTime(int timeLeft, int increment);

#endif //MOLYBDENUM_TIMEMANAGEMENT_H
