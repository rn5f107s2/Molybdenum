#ifndef MOLYBDENUM_POLICY_H
#define MOLYBDENUM_POLICY_H

#include <array>
#include <cstdint>

#include "Move.h"

const int HIDDEN_SIZE = 256;
const int OUT_SIZE    = 64 * 64;

struct Weights {
    std::array<int16_t, 768 * HIDDEN_SIZE> l0Weights;
    std::array<int16_t, HIDDEN_SIZE> l0Biases;
    std::array<int16_t, HIDDEN_SIZE * OUT_SIZE> l1Weights;
    std::array<int16_t, OUT_SIZE> l1Biases;
};

int16_t ReLU(int16_t val) {
    return std::max(val, int16_t(0));
}

class Net {
    Weights weights;
    std::array<int16_t, HIDDEN_SIZE> accumulator;

    void  loadDefault();
    void  initAccumulator(std::array<u64, 13> &bitboards, Color stm);
    float forward(Move move);
};

#endif