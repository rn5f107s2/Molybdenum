#include <iostream>
#include "UCI.h"
#include "Transpositiontable.h"
#include "BitStuff.h"
#include "MagicBitboards.h"
#include "Movegen.h"
#include "mcts.h"

int main(int argc, char** argv) {
    TT.setSize(16);
    UCI *uci = new UCI();
    uci->start(argc, argv);
    delete uci;
    return 0;
}
