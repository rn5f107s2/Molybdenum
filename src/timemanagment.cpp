#include <algorithm>
#include <iostream>
#include "timemanagement.h"

searchTime calcThinkingTime(int timeLeft, int increment, int movesToGo) {
    searchTime st;
    int normalHardTime = std::min((timeLeft / 20) + (increment / 2), timeLeft / 3);
    int normalSoftTime = std::min((timeLeft / 25) + (increment / 3), timeLeft / 3);
    int mtgTime = (timeLeft / movesToGo);
    st.thinkingTime[Hard] = std::chrono::milliseconds(std::max(normalHardTime, mtgTime) - moveOverHead);
    st.thinkingTime[Soft] = std::chrono::milliseconds(std::max(normalSoftTime, mtgTime) - moveOverHead);
    std::cout << std::max(normalHardTime, mtgTime) - moveOverHead << " " << std::max(normalSoftTime, mtgTime) - moveOverHead << "\n";
    return st;
}