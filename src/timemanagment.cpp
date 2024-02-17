#include <algorithm>
#include "timemanagement.h"

#include "tune.h"

#ifdef TUNETIME
Tune tune;
#endif

searchTime calcThinkingTime(int timeLeft, int increment, int movesToGo) {
    searchTime st;
    int normalHardTime = std::min(int((timeLeft * tune.TimeleftMultHard) + (increment / tune.IncrementMultHard)), timeLeft / 3);
    int normalSoftTime = normalHardTime * tune.SoftMult;
    int mtgTime = (timeLeft / movesToGo);
    int hardTime = std::max(normalHardTime, mtgTime);
    st.thinkingTime[Hard] = std::chrono::milliseconds(hardTime - moveOverHead);
    st.thinkingTime[Soft] = std::chrono::milliseconds(normalSoftTime - moveOverHead);
    return st;
}