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
    // for (int bucketPc = WHITE_PAWN; bucketPc <= BLACK_KING; bucketPc++) {
    //     for (int featurePc = WHITE_PAWN; featurePc <= BLACK_KING; featurePc++) {
    //         for (int featureSq = 0; featureSq < 64; featureSq++) {
    //             for (int bucketSq = 0; bucketSq < 64; bucketSq++) {
    //                 for (int n = 0; n < 4; n++) {
    //                     int featureIndexOld = featurePc * 64 + featureSq;
    //                     // arr[fpc][fsq][bpc][bsq][n] 
                           // target = arr[bpt][bsq][fpt][fsq][ci][n]
                           // where ci = 00 for ww, 01 for bb, 10 for wb, 01 for bw
                           // and fsq = sq if fpcolor == white else sq ^ 56
                           // and bsq = sq if bpcolor == white else sq ^ 56
    //                     int originalIndex = featureIndexOld * L1_SIZE * 12 + L1_SIZE * bucketPc + bucketSq * 4 + n;

    //                     // change feature indexing, such that a feature and the "flipped" feature are consective in memory
    //                     int featureIndexNew = typeOf(Piece(featurePc)) * 64 * 2 + (colorOf(featurePc) == BLACK ? featureSq ^ 56 : featureSq) * 2 + (colorOf(featurePc) == BLACK);
    //                     int targetIndex     = featureIndexNew * L1_SIZE * 12 + L1_SIZE * bucketPc + bucketSq * 4 + n;

    //                     std::cout << featurePc << " " << featureSq << ": " << featureIndexOld << " " << featureIndexNew << std::endl;
    //                 }
    //             }
    //         }
    //     }
    // }

    // [fpt][fsq][bpt][bsq][ci][n]

    bias1 = defaultWeights.bias1;
    weights0 = defaultWeights.weights0;

    for (int fpc = 0; fpc < 12; fpc++)
       for (int fsq = 0; fsq < 64; fsq++)
            for (int bpc = 0; bpc < 12; bpc++)
                for (int bsq = 0; bsq < 64; bsq++)
                    for (int n = 0; n < 4; n++)
                        weights0[index_new<WHITE>(bpc, bsq, fpc, fsq) + n] = defaultWeights.weights0[index_old<WHITE>(bpc, bsq, fpc, fsq) + n];

    for (int sq = 0; sq < 64; sq++) {
        for (int pc = 0; pc < 12; pc++) {
            for (int n = 0; n < 4; n++) {
                weights1[256 * 2 * pc + (sq * 4 * 2) +     n] = defaultWeights.weights1[256 * pc + (sq * 4) + n];
                weights1[256 * 2 * pc + (sq * 4 * 2) + 4 + n] = defaultWeights.weights1[256 * makePiece(typeOf(pc), !colorOf(pc)) + ((sq ^ 56) * 4) + n + L1_SIZE * 12];
            }
        }
    }

    for (int sq = 0; sq < 64; sq++)
        for (int pc = 0; pc < 12; pc++)
            for (int n = 0; n < 4; n++)
                bias0[L1_SIZE * 2 * typeOf(pc) + (colorOf(pc) ? sq : sq ^ 56) * 8 + 4 * (!colorOf(pc)) + n]
                = defaultWeights.bias0[sq * 4 + L1_SIZE * pc + n] + weights0[index_new<WHITE>(pc, sq, pc, sq) + n];

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

std::tuple<float, float, float> Net::getWDL(Color c) {
    int output[3] = {0, 0, 0};
    std::tuple<float, float, float> tpl;

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
