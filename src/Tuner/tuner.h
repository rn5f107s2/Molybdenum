#ifndef TUNER_H_INCLUDED
#define TUNER_H_INCLUDED

#include <array>
#include <string>
#include <cstdint>
#include <fstream>
#include <sstream>
#include "../Position.h"

static const int QUANT = 1024;

void initScaleValue(const std::string &filename, Position &pos) {
    std::ifstream in;
    std::ofstream out;
    std::string entry;
    std::array<std::array<int64_t, 486>, 486> occurences    = {};
    std::array<std::array<int64_t, 486>, 486> totalExpected = {};

    in.open(filename);

    std::cout << filename << std::endl;

    while (std::getline(in, entry)) {
        int index = 0;
        std::stringstream stream;
        std::array<std::string, 3> split;
        stream.str(entry);

        while (std::getline(stream, split[index++], '|')) {}

        pos.setBoard(split[0]);
        
        int stmID  = pos.matKey.id( pos.sideToMove);
        int nstmID = pos.matKey.id(!pos.sideToMove);

        occurences[stmID][nstmID]++;
        totalExpected[stmID][nstmID] += std::stof(split[2]) == 0.5f;

        entry = "";
    }

    in.close();
    out.open("moly_scale.bin", std::ios::binary);
    
    for (int i = 0; i < 486; i++) {
        for (int j = 0; j < 486; j++) {
            int16_t val = int16_t((QUANT / 4) * (double(totalExpected[i][j]) / double(std::max(occurences[i][j], int64_t(1)))));

            if (occurences[i][j] < 1000)
                val = 0;

            out.write(reinterpret_cast<char*>(&val), sizeof(val));
        }
    }

    out.close();
}


#endif