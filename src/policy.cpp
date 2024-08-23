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
#include <cmath>

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

void PolicyNet::initAccumulator(std::array<u64, 13> &bitboards, Color stm, u64 threats) {
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
                accumulator[i] += weights.l0Weights[idx * HIDDEN_SIZE + i];
        }
    }

    if (!stm)
        threats = __builtin_bswap64(threats);

    while (threats) {
        int square = popLSB(threats);
        int idx    = 768 + square;

        for (int i = 0; i < HIDDEN_SIZE; i++)
            accumulator[i] += weights.l0Weights[idx * HIDDEN_SIZE + i];
    }
    
}

float PolicyNet::forward(Move move, bool stm) {
    int from = extract<FROM>(move);
    int to   = extract<TO  >(move);

    if (!stm) {
        from ^= 56;
        to   ^= 56;
    }

    int idx = from * 64 + to;
    int out = 0;

    for (int i = 0; i < HIDDEN_SIZE; i++)
        out += ReLU(accumulator[i]) * weights.l1Weights[i * OUT_SIZE + idx];

    out += weights.l1Biases[idx];

    return float(out) / float(255 * 64);
}

void PolicyNet::scoreMovesList(MoveList &ml, bool stm, std::array<u64, 13> &bitboards, u64 threats, int temperature) {
    initAccumulator(bitboards, stm, threats);

    float scores[218];
    float sum = 0;

    for (int i = 0; i < ml.length; i++)
        sum += (scores[i] = std::exp(forward(ml.moves[i].move, stm) / float(temperature)));

    for (int i = 0; i < ml.length; i++)
        ml.moves[i].score = int((scores[i] / sum) * 16384.0f);
}