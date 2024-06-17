#include <iostream>
#include "UCI.h"
#include "Transpositiontable.h"
#include "BitStuff.h"
#include "MagicBitboards.h"
#include "Movegen.h"

int main(int argc, char** argv) {
    TT.setSize(16);
    UCI uci;
    uci.start(argc, argv);
    return 0;
}
