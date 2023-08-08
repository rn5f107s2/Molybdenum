#include <iostream>
#include "UCI.h"
#include "BitStuff.h"
#include "MagicBitboards.h"

int main() {
    std::array<std::array<u64, 4096>, 64> t = {{0}};
    std::array<u64, 64> m = {0};

    initMagics<BISHOP_S>(m, t);

    for (int i = 0; i != 64; i++)
        printBB(m[i]);
    uciCommunication();
    return 0;
}
