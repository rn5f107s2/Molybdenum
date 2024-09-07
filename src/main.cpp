#include <iostream>
#include <iomanip>

#include "UCI.h"
#include "Transpositiontable.h"
#include "BitStuff.h"
#include "MagicBitboards.h"
#include "Movegen.h"
#include "convolution.h"

int main(int argc, char** argv) {
    ConvolutionalLayer<1, 2> cl;
    std::array<std::array<std::array<float, 10>, 10>, 1> paddedInput = 
    {{{{
        { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, 0.0f },
        { 0.0f,  1.0f,  2.0f,  3.0f,  4.0f,  5.0f,  6.0f,  7.0f,  8.0f, 0.0f },
        { 0.0f,  9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f, 0.0f },
        { 0.0f, 17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f, 0.0f },
        { 0.0f, 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f, 0.0f },
        { 0.0f, 33.0f, 34.0f, 35.0f, 36.0f, 37.0f, 38.0f, 39.0f, 40.0f, 0.0f },
        { 0.0f, 41.0f, 42.0f, 43.0f, 44.0f, 45.0f, 46.0f, 47.0f, 48.0f, 0.0f },
        { 0.0f, 49.0f, 50.0f, 51.0f, 52.0f, 53.0f, 54.0f, 55.0f, 56.0f, 0.0f },
        { 0.0f, 57.0f, 58.0f, 59.0f, 60.0f, 61.0f, 62.0f, 63.0f, 64.0f, 0.0f },
        { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, 0.0f }
    }}}};

    auto& output = cl.forward(paddedInput);

    for (int a = 0; a < 2; a++) {
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                std::cout << std::setprecision(4) << output[a][i][j] << ", ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    std::cout << "Im done! Why would I ever segfault now?" << std::endl;  

    //TT.setSize(16);
    //uciCommunication(argc > 1 ? argv[1] : "");
    return 0;
}
