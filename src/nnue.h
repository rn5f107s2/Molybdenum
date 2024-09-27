#ifndef MOLYBDENUM_NNUE_H
#define MOLYBDENUM_NNUE_H

#include "Constants.h"
#include <array>
#include <string>
#include <tuple>
#include <algorithm>
#include "BitStuff.h"
#include "Utility.h"

enum Toggle {
    Off, On
};

static const int INPUT_SIZE = 12 * 64;
static const int L1_SIZE = 256;
static const int L2_SIZE = 4;
static const int L3_SIZE = 16;
static const int OUT_SIZE = 1;

struct Weights {
    std::array<float, L1_SIZE * INPUT_SIZE> weights0{};
    std::array<float, L1_SIZE> bias0{};
    std::array<float, L1_SIZE * L2_SIZE * 2> weights1{};
    std::array<float, L2_SIZE> bias1{};
    std::array<float, L2_SIZE * L3_SIZE> weights2{};
    std::array<float, L3_SIZE> bias2{};
    std::array<float, L3_SIZE * OUT_SIZE> weights3{};
    std::array<float, OUT_SIZE> bias3{};
};

struct WDLHead {
    std::array<int16_t, L1_SIZE * 3 * 2> weights1{};
    std::array<int16_t, 3> bias1{};
};

class Net {
public:
    std::array<int16_t , L1_SIZE * INPUT_SIZE> weights0{};
    std::array<int16_t, L1_SIZE> bias0{};
    std::array<float, 64 * 4 * 2> weights1{};
    std::array<float, L2_SIZE> bias1{};
    std::array<float, L2_SIZE * L3_SIZE> weights2{};
    std::array<float, L3_SIZE> bias2{};
    std::array<float, L3_SIZE * OUT_SIZE> weights3{};
    std::array<float, OUT_SIZE> bias3{};
    std::array<std::array<int16_t, L1_SIZE>, 2> accumulator{};
    std::array<int16_t, L1_SIZE * 3 * 2> wdlWeights{};
    std::array<int16_t, 3> wdlBias{};
    Stack<std::array<std::array<int16_t, L1_SIZE>, 2>, MAXDEPTH> accumulatorStack;

    void initAccumulator(std::array<u64, 13> &bitboards);
    int calculate(Color c);
    std::tuple<float, float, float> getWDL(Color c);
    void loadDefaultNet();

    template<Toggle STATE> inline
    void toggleFeature(int piece, int square);
    inline void moveFeature(int piece, int from, int to);
    inline void pushAccToStack();
    inline void popAccStack();
};

inline int screlu(int16_t input) {
    int clamped = std::clamp(input, int16_t(0), int16_t(255));
    return clamped * clamped;
}

inline float screlu(float input) {
    float clamped = std::clamp(input, 0.0f, 1.0f);
    return clamped * clamped;
}

inline float leakysrelu(float input) {
    return input > 0 ? input * input : input * 0.1f;
}

inline float relu(float input) {
    return std::max(input, 0.0f);
}

template<Color C> inline
int index(int pc, int sq) {
    int square = C ? sq : sq ^ 56;
    int piece  = C ? pc : makePiece(typeOf(pc), !colorOf(pc));

    return piece * 64 + square;
}

template<Toggle STATE> inline
void Net::toggleFeature(int piece, int square) {
    int indexWhite = index<WHITE>(piece, square);
    int indexBlack = index<BLACK>(piece, square);

    for (int l = 0; l != L1_SIZE; l++) {
        accumulator[WHITE][l] += weights0[indexWhite * L1_SIZE + l] * (!STATE ? -1 : 1);
        accumulator[BLACK][l] += weights0[indexBlack * L1_SIZE + l] * (!STATE ? -1 : 1);
    }
}

inline void Net::moveFeature(int piece, int from, int to) {
    toggleFeature<Off>(piece, from);
    toggleFeature<On >(piece, to);
}

inline void Net::pushAccToStack() {
    accumulatorStack.push(accumulator);
}

inline void Net::popAccStack() {
    accumulatorStack.pop();
    accumulator = accumulatorStack.top();
}


#endif //MOLYBDENUM_NNUE_H
