#pragma once

#include <array>
#include <cmath>
#include <fstream>

template<int CHANNEL>
class Batchnorm {
    static constexpr int PADDING     = 1;
    static constexpr int DIM1        = 8;
    static constexpr int DIM2        = 8;
    static constexpr int DIM1_PADDED = DIM1 + 2 * PADDING;
    static constexpr int DIM2_PADDED = DIM2 + 2 * PADDING;

    const float epsilon = 1e-5;

    using Input   = std::array<std::array<std::array<float, DIM2_PADDED>, DIM1_PADDED>, CHANNEL>;
    using Output  = Input;
    using Params  = std::array<float, CHANNEL>;

    Output output;
    Params weights;
    Params biases;
    Params variance;
    Params mean;

public:
    Output& forward(Input &input, Input* skip = nullptr) {
        for (int i = 0; i < CHANNEL; i++) {
            float div = std::sqrt(variance[i] + epsilon);

            for (int j = PADDING; j < DIM1 + PADDING; j++) {
                for (int k = PADDING; k < DIM2 + PADDING; k++) {
                    output[i][j][k] = ((input[i][j][k] - mean[i]) / div) * weights[i] + biases[i];

                    if (skip)
                        output[i][j][k] += (*skip)[i][j][k];
                }
            }
        }

        return output;
    }

    Output& ReLUInplace() {
        for (int i = 0; i < CHANNEL; i++)
            for (int j = PADDING; j < DIM1 + PADDING; j++)
                for (int k = PADDING; k < DIM2 + PADDING; k++)
                    output[i][j][k] = std::max(output[i][j][k], 0.0f);

        return output;
    }

    void loadWeights(std::ifstream &in) {
        in.read(reinterpret_cast<char*>(&weights),  sizeof(Params));
        in.read(reinterpret_cast<char*>(&biases),   sizeof(Params));
        in.read(reinterpret_cast<char*>(&variance), sizeof(Params));
        in.read(reinterpret_cast<char*>(&mean),     sizeof(Params));
    }
};