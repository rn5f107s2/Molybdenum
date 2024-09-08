#pragma once

#include <array>
#include <cstring>

template<int IN_CHANNELS, int OUT_CHANNELS>
class ConvolutionalLayer {
    static constexpr int PADDING     = 1;
    static constexpr int DIM1        = 8;
    static constexpr int DIM2        = 8;
    static constexpr int DIM1_PADDED = DIM1 + 2 * PADDING;
    static constexpr int DIM2_PADDED = DIM2 + 2 * PADDING;
    static constexpr int KERNEL_SIZE = 3;

    using Input   = std::array<std::array<std::array<float, DIM2_PADDED>, DIM1_PADDED>, IN_CHANNELS >;
    using Output  = std::array<std::array<std::array<float, DIM2_PADDED>, DIM1_PADDED>, OUT_CHANNELS>;
    using Weights = std::array<std::array<std::array<std::array<float, KERNEL_SIZE>, KERNEL_SIZE>, IN_CHANNELS>, OUT_CHANNELS>;
    using Biases  = Output;

    Output  output;
    Biases  biases;
    Weights weights;

public:
    ConvolutionalLayer() {
        for (int a = 0; a < 2; a++) {
            for (int i = 0; i < 10; i++) {
                for (int j = 0; j < 10; j++) {
                    biases    [a][i][j] = 0.0f;
                    weights[0][a][i][j] = 1.0f + a;
                }
            }
        }
    }

    Output& forward(Input &input) {
        memcpy(&output, &biases, sizeof(Biases));

        for (int i = 0; i < IN_CHANNELS; i++) {
            for (int j = 0; j < OUT_CHANNELS; j++) {
                for (int k = PADDING; k < DIM1 + PADDING; k++) {
                    for (int l = PADDING; l < DIM2 + PADDING; l++) {
                        for (int m = -PADDING; m < KERNEL_SIZE - PADDING; m++) {
                            for (int n = -PADDING; n < KERNEL_SIZE - PADDING; n++) {
                                output[j][k][l] += input[i][k + m][l + n] * weights[j][i][m + PADDING][n + PADDING];
                            }
                        }
                    }
                }
            }
        }

        return output;
    }

    void loadWeights(std::ifstream &in) {
        float bias;

        in.read(reinterpret_cast<char*>(&weights), sizeof(Weights));
        memset(&biases, 0, sizeof(Biases));

        for (int i = 0; i < OUT_CHANNELS; i++) {
            in.read(reinterpret_cast<char*>(&bias), sizeof(float));

            for (int j = PADDING; j < DIM1 + PADDING; j++)
                for (int k = PADDING; k < DIM2 + PADDING; k++)
                    biases[i][j][k] = bias;
        }
    }
};