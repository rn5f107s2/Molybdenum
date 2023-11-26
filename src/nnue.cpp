#include "nnue.h"
#include "Constants.h"
#include "BitStuff.h"
#include "Utility.h"
#include <cstring>
#include <fstream>

void readNetwork(const std::string &filename) {
    std::ifstream stream{filename, std::ios::binary};

    stream.read(reinterpret_cast<char *>(&weights0), sizeof(weights0[0]) * weights0.size());
    stream.read(reinterpret_cast<char *>(&bias0), sizeof(bias0[0]) * bias0.size());
    stream.read(reinterpret_cast<char *>(&weights1), sizeof(weights1[0]) * weights1.size());
    stream.read(reinterpret_cast<char *>(&bias1), sizeof(bias1[0]) * bias1.size());
}

void initAccumulator(std::array<u64, 13> &bitboards) {
    memcpy(&accumulator[WHITE], &bias0[0], sizeof(int16_t) * L1_SIZE);
    memcpy(&accumulator[BLACK], &bias0[0], sizeof(int16_t) * L1_SIZE);

    for (int pc = WHITE_PAWN; pc != NO_PIECE; pc++) {
        u64 pieceBB = bitboards[pc];

        while (pieceBB) {
            int square = popLSB(pieceBB);

            toggleFeature<On>(pc, square);
        }
    }

    std::cout << (calculate(WHITE)) * 400 / 16320 << "\n";
}

int calculate(Color c) {
    int output = 0;

    for (int n = 0; n != L1_SIZE; n++) {
         output += relu(accumulator[ c][n]) * weights1[n          ];
         output += relu(accumulator[!c][n]) * weights1[n + L1_SIZE];
    }

    return output + bias1[0];
}