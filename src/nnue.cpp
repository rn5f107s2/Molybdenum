#include "nnue.h"
#include "Constants.h"
#include "BitStuff.h"
#include "Utility.h"

#include <cmath> 
#include <cstring>
#include <fstream>
#include <iomanip>

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

#define defaultNetPath "src/Nets/params.bin"

INCBIN(network, defaultNetPath);
const Weights defaultWeights = *reinterpret_cast<const Weights*>(gnetworkData);

Net net;

void loadDefaultNet() {
    //for (int i = 0; i < INPUT_SIZE * L1_SIZE; i++)
    //    net.weights0[i] = int16_t(double(defaultWeights.weights0[i]) * double(255));

    //for (int i = 0; i < L1_SIZE; i++)
    //    net.bias0[i] = int16_t(double(defaultWeights.bias0[i]) * double(255));

    //for (int i = 0; i < L1_SIZE * L2_SIZE * 2; i++)
        //net.weights1[i] = int16_t(double(defaultWeights.weights1[i]) * double(64));

    net.weights0 = defaultWeights.weights0;
    net.bias0    = defaultWeights.bias0;
    net.weights1 = defaultWeights.weights1;
    net.bias1    = defaultWeights.bias1;
    net.weights2 = defaultWeights.weights2;
    net.bias2    = defaultWeights.bias2;

    //for (int i = 0; i < L2_SIZE; i++)
        //net.bias1[i] = int16_t(double(defaultWeights.bias1[i]) * double(255 * 64));


    //for (int i = 0; i < OUTPUT_SIZE * L2_SIZE; i++)
        //net.weights2[i] = int16_t(double(defaultWeights.weights2[i]) * double(64));

    //for (int i = 0; i < OUTPUT_SIZE; i++)
        //net.bias2[i] = int16_t(double(defaultWeights.bias2[i]) * double(64));
}

void readNetwork(const std::string &filename) {
    std::ifstream stream{filename, std::ios::binary};

    stream.read(reinterpret_cast<char *>(&net.weights0), sizeof(net.weights0[0]) * net.weights0.size());
    stream.read(reinterpret_cast<char *>(&net.bias0), sizeof(net.bias0[0]) * net.bias0.size());
    stream.read(reinterpret_cast<char *>(&net.weights1), sizeof(net.weights1[0]) * net.weights1.size());
    stream.read(reinterpret_cast<char *>(&net.bias1), sizeof(net.bias1[0]) * net.bias1.size());
}

void initAccumulator(std::array<u64, 13> &bitboards) {
    net.accumulatorStack.clear();

    memcpy(&net.accumulator[WHITE], &net.bias0[0], sizeof(float) * L1_SIZE);
    memcpy(&net.accumulator[BLACK], &net.bias0[0], sizeof(float) * L1_SIZE);

    for (int pc = WHITE_PAWN; pc != NO_PIECE; pc++) {
        u64 pieceBB = bitboards[pc];

        while (pieceBB) {
            int square = popLSB(pieceBB);

            toggleFeature<On>(pc, square);
        }
    }

    pushAccToStack();
}

int calculate(Color c) {
    std::array<float, L2_SIZE> l1Out = net.bias1;

    for (int n = 0; n != L1_SIZE; n++) {
        for (int n2 = 0; n2 != L2_SIZE; n2++) {
            l1Out[n2] += relu(net.accumulator[ c][n]) * net.weights1[n * L2_SIZE + n2                    ];
            l1Out[n2] += relu(net.accumulator[!c][n]) * net.weights1[n * L2_SIZE + n2 + L1_SIZE * L2_SIZE];
        }
    }

    float output = 0;

    for (int n2 = 0; n2 != L2_SIZE; n2++) {
        output += relu(l1Out[n2]) * net.weights2[n2];
    }

    return (output + net.bias2[0]) * 400;
}
