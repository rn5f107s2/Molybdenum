#include <algorithm>
#include "timemanagement.h"

searchTime calcThinkingTime(int timeLeft, int increment, int movesToGo) {
    searchTime st;
    int normalHardTime = std::min((timeLeft / 5) + (increment / 2), timeLeft / 3);
    int normalSoftTime = std::min((timeLeft / 20) + (increment / 2), timeLeft / 3);
    int mtgTime = (timeLeft / movesToGo);
    int hardTime = std::max(normalHardTime, mtgTime);
    st.thinkingTime[Hard] = std::chrono::milliseconds(hardTime - moveOverHead);
    st.thinkingTime[Soft] = std::chrono::milliseconds(normalSoftTime - moveOverHead);
    return st;
}