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

Net net;

void Net::loadDefaultNet() {
    weights0 = defaultWeights.weights0;
    weights1 = defaultWeights.weights1;
    bias0 = defaultWeights.bias0;
    bias1 = defaultWeights.bias1;
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
    int output = 0;

    for (int n = 0; n != L1_SIZE; n++) {
         output += screlu(accumulator[ c][n]) * weights1[n          ];
         output += screlu(accumulator[!c][n]) * weights1[n + L1_SIZE];
    }

    return ((output / 255) + bias1[0]) * 133 / (64 * 255);
}
