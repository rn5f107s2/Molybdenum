#include "nnue.h"
#include "Constants.h"
#include "BitStuff.h"
#include "Utility.h"
#include <cstring>
#include <fstream>
#include <tuple>
#include <cmath>
#include <iomanip>

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
    for (size_t i = 0; i < defaultWeights.weights0.size(); i++)
        weights0[i] = int16_t(defaultWeights.weights0[i] * 255);

    for (size_t i = 0; i < defaultWeights.bias0.size(); i++)
        bias0[i] = int16_t(defaultWeights.bias0[i] * 255);

    int idx = 0;
    const int bc = 4;
    const int bs = 64;

    for (size_t i = 0; i < defaultWeights.weights1.size(); i++)
        if (((((i / bc) / bs) % bc) == i % bc))
            weights1[idx++] = defaultWeights.weights1[i];
    
    bias1    = defaultWeights.bias1;
    weights2 = defaultWeights.weights2;
    bias2    = defaultWeights.bias2;
    weights3 = defaultWeights.weights3;
    bias3    = defaultWeights.bias3;

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

int Net::calculate(Color c) {
    float out = bias3[0];
    std::array<float, L2_SIZE> l1Out = bias1;
    std::array<float, L3_SIZE> l2Out = bias2;

    // I have absoluetly no clue why I have to manually unroll this, but without the speed is absolute shit
    for (int n = 0; n < 64; n++) {
        l1Out[0] += screlu(float(accumulator[ c][n]) / 255.0f) * weights1[n          ];
        l1Out[0] += screlu(float(accumulator[!c][n]) / 255.0f) * weights1[n + L1_SIZE];
    }

    for (int n = 64; n < 128; n++) {
        l1Out[1] += screlu(float(accumulator[ c][n]) / 255.0f) * weights1[n          ];
        l1Out[1] += screlu(float(accumulator[!c][n]) / 255.0f) * weights1[n + L1_SIZE];
    }

    for (int n = 128; n < 192; n++) {
        l1Out[2] += screlu(float(accumulator[ c][n]) / 255.0f) * weights1[n          ];
        l1Out[2] += screlu(float(accumulator[!c][n]) / 255.0f) * weights1[n + L1_SIZE];
    }

    for (int n = 192; n < 256; n++) {
        l1Out[3] += screlu(float(accumulator[ c][n]) / 255.0f) * weights1[n          ];
        l1Out[3] += screlu(float(accumulator[!c][n]) / 255.0f) * weights1[n + L1_SIZE];
    }

    for (int n = 0; n < L2_SIZE; n++)
        for (int m = 0; m < L3_SIZE; m++)
            l2Out[m] += mscrelu(l1Out[n]) * weights2[n * L3_SIZE + m];

    for (int n = 0; n < L3_SIZE; n++)
        out += screlu(l2Out[n]) * weights3[n];

    return int(out * 133.0f);
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
