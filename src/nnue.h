#ifndef MOLYBDENUM_NNUE_H
#define MOLYBDENUM_NNUE_H

#include "Constants.h"
#include <array>
#include <string>
#include <algorithm>

void readNetwork(const std::string &filename);
void initAccumulator(std::array<u64, 13> &bitboards);

inline int16_t relu(int16_t input) {
    return std::clamp(input, int16_t(0), int16_t(255));
}

inline int featureIndex(int pc, int sq) {
    return pc * 64 + sq;
}


#endif //MOLYBDENUM_NNUE_H
