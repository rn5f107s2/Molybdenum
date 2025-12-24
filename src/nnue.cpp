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

void Net::initAccumulator(Position &pos) {
    accumulatorStack.clear();

    memcpy(&accumulator[WHITE], &bias0[0], sizeof(int16_t) * L1_SIZE);
    memcpy(&accumulator[BLACK], &bias0[0], sizeof(int16_t) * L1_SIZE);

    uint64_t occupied = pos.getOccupied();

    while (occupied) {
        int sq = popLSB(occupied);

        refreshMiniAcc(pos, pos.pieceOn(sq), sq);
    }

    occupied = pos.getOccupied();

    pushAccToStack();
}

int Net::calculate(Color c, uint64_t occupied, Piece* mailbox) {
    int output = 0;

    while (occupied) {
        int sq = popLSB(occupied);
        int nextSq = lsb(occupied);

        int ourPiece   = mailbox[sq ^ (56 * (c == BLACK))];
        int theirPiece = makePiece(typeOf(ourPiece), !colorOf(ourPiece));

        if (c == BLACK) {
            int temp = ourPiece;
            ourPiece = theirPiece;
            theirPiece = temp;
        }

        for (int i = 0; i < 8; i++) {
            int nUs   = (sq * 8) + i;
            int nThem = ((sq ^ 56) * 8) + i;

            output += screlu(accumulator[ c][nUs  ]) * weights1[L1_SIZE * ourPiece   + nUs                 ];
            output += screlu(accumulator[!c][nThem]) * weights1[L1_SIZE * theirPiece + nThem + L1_SIZE * 12];
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
