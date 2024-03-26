#ifndef MOLYBDENUM_NNUE_H
#define MOLYBDENUM_NNUE_H

#include "Constants.h"
#include <array>
#include <string>
#include <algorithm>
#include "BitStuff.h"
#include "Utility.h"

enum Toggle {
    Off, On
};

static const int INPUT_SIZE = 12 * 64;
static const int L1_SIZE = 128;
static const int OUTPUT_SIZE = 1;
static const int NET_SIZE = 3;
static const std::array<int, NET_SIZE> LAYER_SIZE = {INPUT_SIZE, L1_SIZE, OUTPUT_SIZE};

struct Weights {
    std::array<int16_t , L1_SIZE * INPUT_SIZE> weights0{};
    std::array<int16_t, L1_SIZE> bias0{};
    std::array<int16_t, L1_SIZE * OUTPUT_SIZE * 2> weights1{};
    std::array<int16_t, OUTPUT_SIZE> bias1{};
};

struct Net {
    std::array<int16_t , L1_SIZE * INPUT_SIZE> weights0{};
    std::array<int16_t, L1_SIZE> bias0{};
    std::array<int16_t, L1_SIZE * OUTPUT_SIZE * 2> weights1{};
    std::array<int16_t, OUTPUT_SIZE> bias1{};
    std::array<std::array<int16_t, L1_SIZE>, 2> accumulator{};
    Stack<std::array<std::array<int16_t, L1_SIZE>, 2>> accumulatorStack;
    std::array<std::array<std::array<int16_t, L1_SIZE>, 2>, 6000> accDiffStack;
    int accDiffStackHead = 0;
};

extern Net net;

void readNetwork(const std::string &filename);
void initAccumulator(std::array<u64, 13> &bitboards);
int calculate(Color c);
void loadDefaultNet();

inline int16_t relu(int16_t input) {
    return std::clamp(input, int16_t(0), int16_t(255));
}

template<Color C> inline
int index(int pc, int sq) {
    int square = C ? sq : sq ^ 56;
    int piece  = C ? pc : makePiece(typeOf(pc), !colorOf(pc));

    return piece * 64 + square;
}

template<Toggle STATE, bool FIRST = false> inline
void toggleFeature(int piece, int square) {
    int indexWhite = index<WHITE>(piece, square);
    int16_t indexBlack = index<BLACK>(piece, square);

    for (int n = 0; n != L1_SIZE; n++) {
        int updateW = net.weights0[indexWhite * L1_SIZE + n] * (!STATE ? -1 : 1);
        int updateB = net.weights0[indexBlack * L1_SIZE + n] * (!STATE ? -1 : 1);

        if constexpr (FIRST)
            net.accDiffStack[net.accDiffStackHead][WHITE][n] = 
            net.accDiffStack[net.accDiffStackHead][BLACK][n] = 0;

        net.accDiffStack[net.accDiffStackHead][WHITE][n] += updateW;
        net.accDiffStack[net.accDiffStackHead][BLACK][n] += updateB;

        net.accumulator[WHITE][n] += updateW;
        net.accumulator[BLACK][n] += updateB;
    }
}

inline void moveFeature(int piece, int from, int to) {
    toggleFeature<Off>(piece, from);
    toggleFeature<On >(piece, to);
}

inline void pushAccToStack() {
    net.accDiffStackHead++;
}

inline void popAccStack() {
    net.accDiffStackHead--;
    
    for (int n = 0; n != L1_SIZE; n++) {
        net.accumulator[WHITE][n] -= net.accDiffStack[net.accDiffStackHead][WHITE][n];
        net.accumulator[BLACK][n] -= net.accDiffStack[net.accDiffStackHead][BLACK][n];
    }
}


#endif //MOLYBDENUM_NNUE_H
