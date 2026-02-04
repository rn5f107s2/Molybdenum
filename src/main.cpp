#include <iostream>
#include "UCI.h"
#include "Transpositiontable.h"
#include "BitStuff.h"
#include "MagicBitboards.h"
#include "Movegen.h"

#include <unistd.h>
#include <chrono> 

int main(int argc, char** argv) {
    srand(getpid());
    srand(std::chrono::system_clock::now().time_since_epoch().count() ^ rand());

    TT.setSize(16);
    UCI uci;
    uci.start(argc, argv);
    return 0;
}
