#pragma once

#include <array>
#include <algorithm>

#include "Constants.h"

const int LAYER1_SIZE = 256;
const int LAYER2_SIZE = 32;

struct ValueWeights {
    std::array<float, 768 * LAYER1_SIZE> l0Weights;
    std::array<float, LAYER1_SIZE> l0Biases;
    std::array<float, LAYER1_SIZE * LAYER2_SIZE> l1Weights;
    std::array<float, LAYER2_SIZE> l1Biases;
    std::array<float, LAYER2_SIZE> l2Weights;
    std::array<float, 1> l2Biases;
};

inline float SCReLU(float val) {
    float clamped = std::clamp(val, 0.0f, 1.0f);
    return clamped * clamped;
}

class ValueNet {
    ValueWeights weights;

public:
    void  loadDefault();
    float forward(std::array<u64, 13> &bitboards, Color stm);
};
