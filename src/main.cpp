#include <iostream>
#include "UCI.h"
#include "Transpositiontable.h"
#include "BitStuff.h"
#include "MagicBitboards.h"
#include "Movegen.h"

int main(int argc, char** argv) {
    TT.setSize(16);
    uciCommunication(argc > 1 ? argv[1] : "");
    return 0;
}
