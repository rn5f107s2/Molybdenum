#include <iostream>
#include "UCI.h"
#include "Transpositiontable.h"
#include "BitStuff.h"
#include "MagicBitboards.h"
#include "Movegen.h"

int main(int argc, char** argv) {
    TT.setSize(16);
    if (argc > 1)
        uciCommunication(argc, argv[1]);
    else
        uciCommunication(argc);
    return 0;
}
