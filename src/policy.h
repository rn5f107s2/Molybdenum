#ifndef MOLYBDENUM_POLICY_H
#define MOLYBDENUM_POLICY_H

#include <array>
#include <cstdint>

#include "Move.h"

const int IN_SIZE     = 832;
const int HIDDEN_SIZE = 1024;
const int OUT_SIZE    = 64 * 64;

struct PolicyWeights {
    std::array<int16_t, IN_SIZE * HIDDEN_SIZE> l0Weights;
    std::array<int16_t, HIDDEN_SIZE> l0Biases;
    std::array<int16_t, HIDDEN_SIZE * OUT_SIZE> l1Weights;
    std::array<int16_t, OUT_SIZE> l1Biases;
};

inline int16_t ReLU(int16_t val) {
    return std::max(val, int16_t(0));
}

class PolicyNet {
    PolicyWeights weights;
    std::array<int16_t, HIDDEN_SIZE> accumulator;

public:
    void  loadDefault();
    void  initAccumulator(std::array<u64, 13> &bitboards, Color stm, u64 threats);
    void  scoreMovesList(MoveList &ml, bool stm, std::array<u64, 13> &bitboards, u64 threats, int temperature = 1);
    float forward(Move move, bool stm);
};

#endif