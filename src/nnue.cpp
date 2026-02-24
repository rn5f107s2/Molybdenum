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
    for (size_t i = 0; i < bias1.size(); i++)
        bias1[i] = int(defaultWeights.bias1[i] * 64 * 255 * 255);

    for (int fpc = 0; fpc < 12; fpc++)
       for (int fsq = 0; fsq < 64; fsq++)
            for (int bpc = 0; bpc < 12; bpc++)
                for (int bsq = 0; bsq < 64; bsq++)
                    for (int n = 0; n < MINI_ACC_SIZE; n++)
                        weights0[index_new<WHITE>(bpc, bsq, fpc, fsq) + n] = int16_t(defaultWeights.weights0[index_old<WHITE>(bpc, bsq, fpc, fsq) + n] * 255);

    for (int sq = 0; sq < 64; sq++) {
        for (int pc = 0; pc < 12; pc++) {
            for (int n = 0; n < MINI_ACC_SIZE; n++) {
                for (int m = 0; m < L2_SIZE; m++) {
                    int  stmIdx = 64 * L2_SIZE * 2 * pc + (sq * L2_SIZE * 2) + m;
                    int nstmIdx = stmIdx + L2_SIZE;

                    // original
                    // weights1[side][pc][sq][l1_idx][l2_idx]
                    // target
                    // weights1[pc][sq][side][l2_idx][l1_idx]

                    int  oldStm = L1_SIZE * pc + (sq * MINI_ACC_SIZE) + n;
                    int oldNstm = L1_SIZE * makePiece(typeOf(pc), !colorOf(pc)) + ((sq ^ 56) * MINI_ACC_SIZE) + n;

                    weights1[stmIdx  * MINI_ACC_SIZE + n] = int16_t(defaultWeights.weights1[L2_SIZE * oldStm  + m] * 64);
                    weights1[nstmIdx * MINI_ACC_SIZE + n] = int16_t(defaultWeights.weights1[L2_SIZE * oldNstm + m + L1_SIZE * L2_SIZE * 12] * 64);
                }
            }
        }
    }

    for (int sq = 0; sq < 64; sq++)
        for (int pc = 0; pc < 12; pc++)
            for (int n = 0; n < MINI_ACC_SIZE; n++)
                bias0[L1_SIZE * 2 * typeOf(pc) + (colorOf(pc) ? sq : sq ^ 56) * MINI_ACC_SIZE * 2 + MINI_ACC_SIZE * (!colorOf(pc)) + n]
                = int16_t((defaultWeights.bias0[sq * MINI_ACC_SIZE + L1_SIZE * pc + n] + defaultWeights.weights0[index_old<WHITE>(pc, sq, pc, sq) + n]) * 255);

    weights2 = defaultWeights.weights2;
    bias2    = defaultWeights.bias2;
    weights3 = defaultWeights.weights3;
    bias3    = defaultWeights.bias3;

    wdlWeights = defaultWdl.weights1;
    wdlBias    = defaultWdl.bias1;
}

void Net::initAccumulator(Position &pos) {
    accumulatorStack.clear();

    memcpy(&accumulator[WHITE], &bias0[0], sizeof(int16_t) * L1_SIZE);
    memcpy(&accumulator[BLACK], &bias0[0], sizeof(int16_t) * L1_SIZE);

    uint64_t occupied = pos.getOccupied();

    while (occupied) {
        int sq = popLSB(occupied);

        refreshMiniAcc(pos, pos.pieceOn(sq), sq);
    }

    // occupied = pos.getOccupied();

    // while (occupied) {
    //     int sq = popLSB(occupied);

    //     for (int i = 0; i < MINI_ACC_SIZE * 2; i++)
    //         std::cout << accumulator[sq * MINI_ACC_SIZE * 2 + i] << std::endl;
    // }

    occupied = pos.getOccupied();

    pushAccToStack(occupied);
}

std::tuple<float, float, float> Net::getWDL([[maybe_unused]] Color c) {
    int output[3] = {0, 0, 0};
    std::tuple<float, float, float> tpl;

    // currently not used
    return tpl;

    for (int n = 0; n < L1_SIZE; n++) {
        for (int n2 = 0; n2 < 3; n2++) {
            output[n2] += screlu(accumulator[n]) * wdlWeights[n * 3 + n2              ];
            output[n2] += screlu(accumulator[n]) * wdlWeights[n * 3 + n2 + L1_SIZE * 3];
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
