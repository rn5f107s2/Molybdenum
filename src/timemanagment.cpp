#include <algorithm>
#include "timemanagement.h"

searchTime calcThinkingTime(int timeLeft, int increment, int movesToGo) {
    searchTime st;
    int normalTime = (timeLeft / 20) + (increment / 2);
    int mtgTime = (timeLeft / movesToGo);
    st.thinkingTime = std::chrono::milliseconds(std::max(normalTime, mtgTime) - moveOverHead);
    return st;
}