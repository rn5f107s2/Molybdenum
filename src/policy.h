#ifndef MOLYBDENUM_POLICY_H
#define MOLYBDENUM_POLICY_H

#include <array>
#include <cstdint>

#include "Move.h"

const int HIDDEN_SIZE = 256;
const int L2_SIZE     = 256;
const int OUT_SIZE    = 64 * 64;

struct PolicyWeights {
    std::array<float, 768 * HIDDEN_SIZE> l0Weights;
    std::array<float, HIDDEN_SIZE> l0Biases;
    std::array<float, HIDDEN_SIZE * L2_SIZE> l1Weights;
    std::array<float, L2_SIZE> l1Biases;
    std::array<float, OUT_SIZE * L2_SIZE> l2Weights;
    std::array<float, OUT_SIZE> l2Biases;
};

inline float ReLU(float val) {
    return std::max(val, 0.0f);
}

class PolicyNet {
    PolicyWeights weights;
    std::array<float, HIDDEN_SIZE> l1Out;
    std::array<float, L2_SIZE> l2Out;

public:
    void  loadDefault();
    void  initAccumulator(std::array<u64, 13> &bitboards, Color stm);
    float forward(Move move, bool stm);
};

#endif