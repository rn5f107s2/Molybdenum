#ifndef MOLYBDENUM_BITSTUFF_H
#define MOLYBDENUM_BITSTUFF_H

#include <cstdint>
#include <iostream>
#include "Constants.h"

using u64 = uint64_t;

inline int lsb(u64 &bitboard) {
    return __builtin_ctzll(bitboard);
}

inline constexpr int fileOf(int square) {
    return 7 - (square & 7);
}

inline constexpr u64 lFIleOf(int square) {
    return FILEA >> fileOf(square);
}

inline constexpr int rankOf(int square) {
    return square >> 3;
}

inline constexpr u64 lRankOf(int square) {
    return RANK1 << (rankOf(square) * 8);
}

inline u64 createSquare(int file, int rank) {
    return (rank * 8) + file;
}

constexpr std::array<u64, 64> knightMask = {};

constexpr std::array<u64, 64> initKnightMasks() {
    std::array<u64, 64> masks = {};

    for (int square = 0; square != 64; square++) {
        int file = fileOf(square);
        u64 squareL = 1ULL << square;
        u64 mask = 0;

        if (file < 6) {
            mask |= squareL >>  6;
            mask |= squareL << 10;
        }

        if (file < 7) {
            mask |= squareL >> 15;
            mask |= squareL << 17;
        }

        if (file > 1) {
            mask |= squareL <<  6;
            mask |= squareL >> 10;
        }

        if (file > 0) {
            mask |= squareL << 15;
            mask |= squareL >> 17;
        }

        masks[square] = mask;
    }

    return masks;
}

inline void printBB(u64 bitboard) {
    std::string bb;
    u64 relevantBit = 1L << 63;

    while (relevantBit) {
        std::cout << ((relevantBit & bitboard) >> (lsb(relevantBit))) << " ";

        if (fileOf(lsb(relevantBit)) == 7)
            std::cout << "\n";

        relevantBit >>= 1;
    }

    std::cout << std::endl;
}

#endif //MOLYBDENUM_BITSTUFF_H
