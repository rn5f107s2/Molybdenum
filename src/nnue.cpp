#include "nnue.h"
#include "Constants.h"
#include "BitStuff.h"
#include "Utility.h"
#include <cstring>
#include <fstream>
#include <tuple>
#include <cmath>

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

#ifndef WDLFILE
#define WDLFILE // silence syntax highlighting
#endif

INCBIN(network, EVALFILE);
INCBIN(wdlHead, WDLFILE);

const Weights defaultWeights = *reinterpret_cast<const Weights*>(gnetworkData);
const WDLHead defaultWdl     = *reinterpret_cast<const WDLHead*>(gwdlHeadData);

void Net::loadDefaultNet() {
    weights0 = defaultWeights.weights0;
    weights1 = defaultWeights.weights1;
    bias0 = defaultWeights.bias0;
    bias1 = defaultWeights.bias1;

    wdlWeights = defaultWdl.weights1;
    wdlBias    = defaultWdl.bias1;
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

int Net::calculate(Color c, uint64_t occupied) {
    int output = 0;

    while (occupied) {
        int sq = popLSB(occupied);
        int nextSq = lsb(occupied);

        __builtin_prefetch(&accumulator[ c][nextSq * 4]);
        __builtin_prefetch(&accumulator[!c][(nextSq ^ 56) * 4]);
        __builtin_prefetch(&weights1[sq * 4]);
        __builtin_prefetch(&weights1[(sq ^ 56) * 4]);

        for (int i = 0; i < 4; i++) {
            int nUs   = sq * 4 + i;
            int nThem = (sq ^ 56) * 4 + i; 

            output += screlu(accumulator[ c][nUs  ]) * weights1[nUs            ];
            output += screlu(accumulator[!c][nThem]) * weights1[nThem + L1_SIZE];
        }
    }

    return ((output / 255) + bias1[0]) * 133 / (64 * 255);
}

std::tuple<float, float, float> Net::getWDL(Color c) {
    int output[3] = {0, 0, 0};
    std::tuple<float, float, float> tpl;

    for (int n = 0; n < L1_SIZE; n++) {
        for (int n2 = 0; n2 < 3; n2++) {
            output[n2] += screlu(accumulator[ c][n]) * wdlWeights[n * 3 + n2              ];
            output[n2] += screlu(accumulator[!c][n]) * wdlWeights[n * 3 + n2 + L1_SIZE * 3];
        }
    }

    float sum = 0.0f;
    float scores[3];

    for (int i = 0; i < 3; i++)
        sum += (scores[i] = std::exp(double((output[i] / 255) + wdlBias[i]) / double(64 * 255)));

    std::get<0>(tpl) = scores[0] / sum;
    std::get<1>(tpl) = scores[1] / sum;
    std::get<2>(tpl) = scores[2] / sum;

    return tpl;
}
