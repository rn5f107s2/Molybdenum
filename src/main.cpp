#include <iostream>
#include <iomanip>
#include <fstream>

#include "UCI.h"
#include "Transpositiontable.h"
#include "BitStuff.h"
#include "MagicBitboards.h"
#include "Movegen.h"
#include "policy.h"

int main(int argc, char** argv) {
    Network*  net = new Network();
    Position* pos = new Position();
    pos->setBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    std::ifstream weights("weights.bin", std::ios::binary);
    pos->policyNet.loadWeights(weights);

    std::array<std::array<std::array<float, 10>, 10>, 12> input;
    bbsToPaddedInput(pos->bitBoards, pos->sideToMove, input);

    pos->policyNet.forward(input);

    //TT.setSize(16);
    //uciCommunication(argc > 1 ? argv[1] : "");
    return 0;
}
