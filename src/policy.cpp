#include "policy.h"

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

#include <string.h>

#include "Constants.h"
#include "BitStuff.h"

#ifndef POLICYFILE
#define POLICYFILE // silence syntax highlighting
#endif

INCBIN(policy, POLICYFILE);
const PolicyWeights defaultWeights = *reinterpret_cast<const PolicyWeights*>(gpolicyData);

void PolicyNet::loadDefault() {
    weights = defaultWeights;
}

void PolicyNet::initAccumulator(std::array<u64, 13> &bitboards, Color stm) {
    memcpy(&l1Out, &weights.l0Biases[0], sizeof(float) * HIDDEN_SIZE);
    memcpy(&l2Out, &weights.l1Biases[0], sizeof(float) * L2_SIZE);

    for (int pc = WHITE_PAWN; pc != NO_PIECE; pc++) {
        u64 pieceBB = bitboards[pc];

        while (pieceBB) {
            int square = popLSB(pieceBB);
            int piece  = pc;

            if (!stm) {
                square ^= 56;
                piece = makePiece(typeOf(pc), !colorOf(pc));
            }

            int idx = 64 * piece + square;

            for (int i = 0; i < HIDDEN_SIZE; i++)
                l1Out[i] += weights.l0Weights[idx * HIDDEN_SIZE + i];
        }
    }

    for (int i = 0; i < HIDDEN_SIZE; i++)
        for (int j = 0; j < L2_SIZE; j++)
            l2Out[j] += ReLU(l1Out[i]) * weights.l1Weights[i * L2_SIZE + j];
}

float PolicyNet::forward(Move move, bool stm) {
    int from = extract<FROM>(move);
    int to   = extract<TO  >(move);

    if (!stm) {
        from ^= 56;
        to   ^= 56;
    }

    int idx = from * 64 + to;
    float out = weights.l2Biases[idx];

    for (int i = 0; i < L2_SIZE; i++)
        out += ReLU(l2Out[i]) * weights.l2Weights[i * OUT_SIZE + idx];

    return out;
}