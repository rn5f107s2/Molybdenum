#include "nnue.h"
#include "Constants.h"
#include "BitStuff.h"
#include "Utility.h"
#include <cstring>
#include <fstream>

#ifdef _MSC_VER
#define PUSHED_MACRO
#pragma push_macro("_MSC_VER")
#undef _MSC_VER
#endif

#include "incbin/incbin.h"

#ifdef PUSHED_MACRO
#pragma pop_macro("_MSC_VER")
#undef PUSHED_MACRO
#endif

#ifndef EVALFILE
#define EVALFILE // silence syntax highlighting
#endif

INCBIN(network, EVALFILE);
const Weights defaultWeights = *reinterpret_cast<const Weights*>(gnetworkData);

void Net::loadDefaultNet() {
    for (size_t i = 0; i < defaultWeights.weights0.size(); i++)
        weights0[i] = int16_t(defaultWeights.weights0[i] * 255);

    for (size_t i = 0; i < defaultWeights.bias0.size(); i++)
        bias0[i] = int16_t(defaultWeights.bias0[i] * 255);

    weights1 = defaultWeights.weights1;
    bias1    = defaultWeights.bias1;
    weights2 = defaultWeights.weights2;
    bias2    = defaultWeights.bias2;
}

void Net::initAccumulator(std::array<u64, 13> &bitboards) {
    accumulatorStack.clear();

    memcpy(&accumulator[WHITE], &bias0[0], sizeof(int16_t) * L1_SIZE);
    memcpy(&accumulator[BLACK], &bias0[0], sizeof(int16_t) * L1_SIZE);

    for (int pc = WHITE_PAWN; pc != NO_PIECE; pc++) {
        u64 pieceBB = bitboards[pc];

        while (pieceBB) {
            int square = popLSB(pieceBB);

            toggleFeature<On>(pc, square);
        }
    }

    pushAccToStack();
}

int Net::calculate(Color c) {
    float out = bias2[0];
    std::array<float, L2_SIZE> output = bias1;

    for (int n = 0; n < L1_SIZE; n++) {
        for (int m = 0; m < L2_SIZE; m++) {
            output[m] += screlu(float(accumulator[ c][n]) / 255.0f) * weights1[n * L2_SIZE + m                    ];
            output[m] += screlu(float(accumulator[!c][n]) / 255.0f) * weights1[n * L2_SIZE + m + L1_SIZE * L2_SIZE];
        }
    }

    for (int n = 0; n < L2_SIZE; n++)
        out += screlu(output[n]) * weights2[n];

    return int(out * 133.0f);
}
