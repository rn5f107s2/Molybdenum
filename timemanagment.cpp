#include "timemanagement.h"

searchTime calcThinkingTime(int timeLeft, int increment) {
    searchTime st;
    st.thinkingTime = std::chrono::milliseconds((timeLeft / 20) + (increment / 2) - moveOverHead);
    return st;
}