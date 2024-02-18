#include <algorithm>
#include "timemanagement.h"

searchTime calcThinkingTime(int timeLeft, int increment, int movesToGo) {
    searchTime st;
    int normalHardTime = std::min(int((timeLeft * 0.18) + (increment * 0.48)), timeLeft / 3);
    int normalSoftTime = normalHardTime * 0.17;
    int mtgTime = (timeLeft / movesToGo);
    int hardTime = std::max(normalHardTime, mtgTime);
    st.thinkingTime[Hard] = std::chrono::milliseconds(hardTime - moveOverHead);
    st.thinkingTime[Soft] = std::chrono::milliseconds(normalSoftTime - moveOverHead);
    return st;
}
