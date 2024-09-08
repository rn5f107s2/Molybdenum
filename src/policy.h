#ifndef MOLYBDENUM_POLICY_H
#define MOLYBDENUM_POLICY_H

#include <array>
#include <cstdint>
#include <vector>

#include "Move.h"
#include "convolution.h"
#include "batchnorm.h"

const int FILTER = 64;
const int BLOCKS = 4;

const int IN_SIZE  = 832;
const int HIDDEN_SIZE = 256;
const int OUT_SIZE    = 64 * 64;

struct Block {
    using Input  = std::array<std::array<std::array<float, 10>, 10>, FILTER>;
    using Output = Input;

    ConvolutionalLayer<FILTER, FILTER> cl1;
    Batchnorm         <FILTER        > bn1;
    ConvolutionalLayer<FILTER, FILTER> cl2;
    Batchnorm         <FILTER        > bn2;

    Output& forward(Input &input) {
        Input identity; memcpy(&identity, &input, sizeof(Input));

        Output &out = cl1.forward(input);
        bn1.forward(out);
        out = bn1.ReLUInplace();
        out = cl2.forward(out);
        bn2.forward(out, &identity);
        out = bn2.ReLUInplace();

        return out;
    }

    void loadWeights(std::ifstream &in) {
        cl1.loadWeights(in);
        bn1.loadWeights(in);
        cl2.loadWeights(in);
        bn2.loadWeights(in);
    }
};

struct Network {
    using Input  = std::array<std::array<std::array<float, 10>, 10>, 12>;
    using Output = std::array<std::array<std::array<float, 10>, 10>, 64>;

    ConvolutionalLayer<12, FILTER> cl1;
    std::array<Block, BLOCKS> blocks;
    ConvolutionalLayer<FILTER, 64> cl2;

    Output& forward(Input &input) {
        auto &out = cl1.forward(input);
              out = cl1.ReLUInplace();
        
        for (Block &b : blocks)
            out = b.forward(out);

        return cl2.forward(out);
    }

    void loadWeights(std::ifstream &in) {
        cl1.loadWeights(in);

        for (Block &b : blocks)
            b.loadWeights(in);

        cl2.loadWeights(in);
    }
};

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