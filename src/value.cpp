#include "value.h"

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

#ifndef VALUEFILE
#define VALUEFILE // silence syntax highlighting
#endif


#include <string.h>

#include "BitStuff.h"

INCBIN(value, VALUEFILE);
const ValueWeights defaultWeights = *reinterpret_cast<const ValueWeights*>(gvalueData);

void ValueNet::loadDefault() {
    weights = defaultWeights;
}

float ValueNet::forward(std::array<u64, 13> &bitboards, Color stm) {
    float l1Out[LAYER1_SIZE];
    float l2Out[LAYER2_SIZE];
    float out = weights.l2Biases[0];

    memcpy(l1Out, &weights.l0Biases[0], sizeof(float) * LAYER1_SIZE);
    memcpy(l2Out, &weights.l1Biases[0], sizeof(float) * LAYER2_SIZE);

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

            for (int i = 0; i < LAYER1_SIZE; i++)
                l1Out[i] += weights.l0Weights[idx * LAYER1_SIZE + i];
        }
    }

    for (int i = 0; i < LAYER2_SIZE; i++)
        for (int j = 0; j < LAYER1_SIZE; j++)
            l2Out[i] += SCReLU(l1Out[j]) * weights.l1Weights[j * LAYER2_SIZE + i];

    for (int i = 0; i < LAYER2_SIZE; i++)
        out += SCReLU(l2Out[i]) * weights.l2Weights[i];

    return out * 133;
}