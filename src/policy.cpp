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
const Weights defaultWeights = *reinterpret_cast<const Weights*>(gpolicyData);

void Net::loadDefault() {
    weights = defaultWeights;
}

void Net::initAccumulator(std::array<u64, 13> &bitboards, Color stm) {
    memcpy(&accumulator, &weights.l0Biases[0], sizeof(int16_t) * HIDDEN_SIZE);

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
                accumulator[i] += weights.l0Weights[i * HIDDEN_SIZE + idx];
        }
    }
}

float Net::forward(Move move) {
    int from = extract<FROM>(move);
    int to   = extract<TO  >(move);

    int idx = from * 64 + to;
    int out = 0;

    for (int i = 0; i < HIDDEN_SIZE; i++)
        out += ReLU(accumulator[i]) * weights.l1Weights[i * OUT_SIZE + idx];

    out += weights.l1Biases[idx];

    return float(out) / float(255 * 64);
}