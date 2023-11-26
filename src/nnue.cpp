#include "nnue.h"
#include "Constants.h"
#include "BitStuff.h"
#include "Utility.h"
#include <cstring>
#include <fstream>

const int INPUT_SIZE = 12 * 64;
const int L1_SIZE = 32;
const int OUTPUT_SIZE = 1;
const int NET_SIZE = 3;

const std::array<int, NET_SIZE> LAYER_SIZE = {INPUT_SIZE, L1_SIZE, OUTPUT_SIZE};
std::array<int16_t , L1_SIZE * INPUT_SIZE> weights0;
std::array<int16_t, L1_SIZE * OUTPUT_SIZE * 2> weights1;
std::array<int16_t, L1_SIZE> bias0;
std::array<int16_t, OUTPUT_SIZE> bias1;
std::array<int16_t, L1_SIZE> accumulator;

void readNetwork(const std::string &filename) {
    std::ifstream stream{filename, std::ios::binary};

    stream.read(reinterpret_cast<char *>(&weights0), sizeof(weights0[0]) * weights0.size());
    stream.read(reinterpret_cast<char *>(&bias0), sizeof(bias0[0]) * bias0.size());
    stream.read(reinterpret_cast<char *>(&weights1), sizeof(weights1[0]) * weights1.size());
    stream.read(reinterpret_cast<char *>(&bias1), sizeof(bias1[0]) * bias1.size());
}

void initAccumulator(std::array<u64, 13> &bitboards) {
    std::array<std::array<int16_t, L1_SIZE>, 2> l1{};
    memset(&accumulator, 0, sizeof(accumulator[0]) * accumulator.size());
    memccpy(&l1[WHITE], &bias0[0], sizeof(int16_t), L1_SIZE / 2);
    memccpy(&l1[BLACK], &bias0  [L1_SIZE / 2], sizeof(int16_t), L1_SIZE / 2);

    for (int pc = WHITE_PAWN; pc != NO_PIECE; pc++) {
        u64 pieceBB = bitboards[pc];

        while (pieceBB) {
            int square = popLSB(pieceBB);

            for (int n = 0; n != L1_SIZE; n++)
                l1[pc > WHITE_KING][n] += weights0[pc * square];
        }
    }

    for (int w = 0; w != L1_SIZE / 2; w++) {
        accumulator[WHITE] += relu(l1[WHITE][w]) * weights1[w];
    }

    std::cout << ((accumulator[WHITE] - accumulator[BLACK]) + bias1[0]) * 400 / (255 * 64);
}