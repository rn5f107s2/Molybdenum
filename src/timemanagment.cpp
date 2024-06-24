#include <algorithm>

#include "timemanagement.h"

int moveOverHead = 10;

void setOverhead(int val) {
    moveOverHead = val;
}

void SearchTime::calcThinkingTime(int timeLeft, int increment, int movesToGo) {
    int normalHardTime = std::min(int((timeLeft * 0.18) + (increment * 0.48)), timeLeft / 3);
    int normalSoftTime = normalHardTime * 0.17;

    int mtgTime = (timeLeft / movesToGo);
    int hardTime = std::max(normalHardTime, mtgTime);

    thinkingTime[Hard] = std::chrono::milliseconds(hardTime       - moveOverHead);
    thinkingTime[Soft] = std::chrono::milliseconds(normalSoftTime - moveOverHead);
}
