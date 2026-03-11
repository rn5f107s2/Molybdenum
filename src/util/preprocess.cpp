#include "../nnue.h"

#include <fstream>

#ifdef _MSC_VER
#define PUSHED_MACRO
#pragma push_macro("_MSC_VER")
#undef _MSC_VER
#endif

#include "../incbin/incbin.h"

#ifdef PUSHED_MACRO
#pragma pop_macro("_MSC_VER")
#undef PUSHED_MACRO
#endif

#ifndef EVALFILE
#define EVALFILE // silence syntax highlighting
#endif

INCBIN(network, EVALFILE);

const Weights weights = *reinterpret_cast<const Weights*>(gnetworkData);

void preprocess(std::ofstream& outfile) {
    Weights* pp = new Weights();
    Weights& preprocessed = *pp;

    preprocessed.bias1    = weights.bias1;
    preprocessed.weights0 = weights.weights0;

    for (int fpc = 0; fpc < 12; fpc++)
       for (int fsq = 0; fsq < 64; fsq++)
            for (int bpc = 0; bpc < 12; bpc++)
                for (int bsq = 0; bsq < 64; bsq++)
                    for (int n = 0; n < MINI_ACC_SIZE; n++)
                        preprocessed.weights0[index_new<WHITE>(bpc, bsq, fpc, fsq) + n] = weights.weights0[index_old<WHITE>(bpc, bsq, fpc, fsq) + n];

    for (int sq = 0; sq < 64; sq++) {
        for (int pc = 0; pc < 12; pc++) {
            for (int n = 0; n < MINI_ACC_SIZE; n++) {
                preprocessed.weights1[L1_SIZE * 2 * pc + (sq * MINI_ACC_SIZE * 2) +                 n] = weights.weights1[L1_SIZE * pc + (sq * MINI_ACC_SIZE) + n];
                preprocessed.weights1[L1_SIZE * 2 * pc + (sq * MINI_ACC_SIZE * 2) + MINI_ACC_SIZE + n] = weights.weights1[L1_SIZE * makePiece(typeOf(pc), !colorOf(pc)) + ((sq ^ 56) * MINI_ACC_SIZE) + n + L1_SIZE * 12];
            }
        }
    }

    for (int sq = 0; sq < 64; sq++)
        for (int pc = 0; pc < 12; pc++)
            for (int n = 0; n < MINI_ACC_SIZE; n++)
                preprocessed.bias0[L1_SIZE * 2 * typeOf(pc) + (colorOf(pc) ? sq : sq ^ 56) * MINI_ACC_SIZE * 2 + MINI_ACC_SIZE * (!colorOf(pc)) + n]
                = weights.bias0[sq * MINI_ACC_SIZE + L1_SIZE * pc + n] + preprocessed.weights0[index_new<WHITE>(pc, sq, pc, sq) + n];

    std::cout << "alive" << std::endl; 

    outfile.write(reinterpret_cast<char*>(preprocessed.weights0.data()), preprocessed.weights0.size() * sizeof(int16_t));
    outfile.write(reinterpret_cast<char*>(preprocessed.bias0.data()), preprocessed.bias0.size() * sizeof(int16_t));
    outfile.write(reinterpret_cast<char*>(preprocessed.weights1.data()), preprocessed.weights1.size() * sizeof(int16_t));
    outfile.write(reinterpret_cast<char*>(preprocessed.bias1.data()), preprocessed.bias1.size() * sizeof(int16_t));
}

int main(int argc, char** argv) {
    std::ofstream out(argv[1], std::ios::binary);

    preprocess(out);
}
