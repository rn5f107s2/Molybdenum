#ifndef MOLYBDENUM_NNUE_H
#define MOLYBDENUM_NNUE_H

#include "Constants.h"
#include <array>
#include <string>
#include <algorithm>
#include "BitStuff.h"

enum Toggle{
    Off, On
};

static const int INPUT_SIZE = 12 * 64;
static const int L1_SIZE = 32;
static const int OUTPUT_SIZE = 1;
static const int NET_SIZE = 3;

static const std::array<int, NET_SIZE> LAYER_SIZE = {INPUT_SIZE, L1_SIZE, OUTPUT_SIZE};
static std::array<int16_t , L1_SIZE * INPUT_SIZE> weights0;
static std::array<int16_t, L1_SIZE * OUTPUT_SIZE * 2> weights1;
static std::array<int16_t, L1_SIZE> bias0;
static std::array<int16_t, OUTPUT_SIZE> bias1;
static std::array<std::array<int16_t, L1_SIZE>, 2> accumulator;

void readNetwork(const std::string &filename);
void initAccumulator(std::array<u64, 13> &bitboards);
int calculate(Color c);

inline int16_t relu(int16_t input) {
    return std::clamp(input, int16_t(0), int16_t(255));
}

template<Color C> inline
int index(int pc, int sq) {
    int square = C ? sq ^ 7 : sq ^ 63;
    int piece  = C ? pc : makePiece(typeOf(pc), !colorOf(pc));

    return piece * 64 + square;
}

template<Toggle STATE> inline
void toggleFeature(int piece, int square) {
    int indexWhite = index<WHITE>(piece, square);
    int indexBlack = index<BLACK>(piece, square);

    for (int l = 0; l != L1_SIZE; l++) {
        accumulator[WHITE][l] += weights0[indexWhite * L1_SIZE + l] * (!STATE ? -1 : 1);
        accumulator[BLACK][l] += weights0[indexBlack * L1_SIZE + l] * (!STATE ? -1 : 1);
    }
}


#endif //MOLYBDENUM_NNUE_H
