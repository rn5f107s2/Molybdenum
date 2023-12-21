#include "nnue.h"
#include "Constants.h"
#include "BitStuff.h"
#include "Utility.h"
#include <cstring>
#include <fstream>

Net net;

void readNetwork(const std::string &filename) {
    std::ifstream stream{filename, std::ios::binary};

    stream.read(reinterpret_cast<char *>(&net.weights0), sizeof(net.weights0[0]) * net.weights0.size());
    stream.read(reinterpret_cast<char *>(&net.bias0), sizeof(net.bias0[0]) * net.bias0.size());
    stream.read(reinterpret_cast<char *>(&net.weights1), sizeof(net.weights1[0]) * net.weights1.size());
    stream.read(reinterpret_cast<char *>(&net.bias1), sizeof(net.bias1[0]) * net.bias1.size());
}

void initAccumulator(std::array<u64, 13> &bitboards) {
    net.accumulatorStack.clear();

    memcpy(&net.accumulator[WHITE], &net.bias0[0], sizeof(int16_t) * L1_SIZE);
    memcpy(&net.accumulator[BLACK], &net.bias0[0], sizeof(int16_t) * L1_SIZE);

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
    int output = 0;

    for (int n = 0; n != L1_SIZE; n++) {
         output += relu(net.accumulator[ c][n]) * net.weights1[n          ];
         output += relu(net.accumulator[!c][n]) * net.weights1[n + L1_SIZE];
    }

    return (output + net.bias1[0]) * 400 / 16320;
}