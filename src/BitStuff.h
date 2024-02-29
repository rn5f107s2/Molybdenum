#ifndef MOLYBDENUM_BITSTUFF_H
#define MOLYBDENUM_BITSTUFF_H

#include <cstdint>
#include <iostream>
#include <bit>
#include "Constants.h"

#ifdef _MSC_VER
#include <intrin.h>
#endif

using u64 = uint64_t;

inline int lsb(const u64& bitboard) {
#ifdef __GNUC__
    return __builtin_ctzll(bitboard);
#elif defined(_MSC_VER)
    return _tzcnt_u64(bitboard);
#endif
}

inline int lsb(const int &bitboard) {
#ifdef __GNUC__
    return __builtin_ctz(bitboard);
#elif defined(_MSC_VER)
    return _tzcnt_u32(bitboard);
#endif
}

inline int popcount(u64 bb) {
    return __builtin_popcountll(bb);
}

inline int popLSB(u64 &bitboard) {
    int bit = lsb(bitboard);
    bitboard &= bitboard - 1;
    return  bit;
}

inline int popLSB(int &bitboard) {
    int bit = lsb(bitboard);
    bitboard &= bitboard - 1;
    return bit;
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

inline constexpr int typeOf(int piece) {
    return piece % 6;
}

inline u64 createSquare(int file, int rank) {
    return 1ULL << ((rank * 8) + file);
}

inline bool multipleBits(u64 bb) {
    return bb & (bb - 1);
}

constexpr std::array<u64, 64> initKnightMasks() {
    std::array<u64, 64> masks = {};

    for (int square = 0; square != 64; square++) {
        int file = fileOf(square);
        u64 squareL = 1ULL << square;
        u64 mask = 0;

        if (file < 6) {
            mask |= squareL <<  6;
            mask |= squareL >> 10;
        }

        if (file < 7) {
            mask |= squareL << 15;
            mask |= squareL >> 17;
        }

        if (file > 1) {
            mask |= squareL >>  6;
            mask |= squareL << 10;
        }

        if (file > 0) {
            mask |= squareL >> 15;
            mask |= squareL << 17;
        }

        masks[square] = mask;
    }

    return masks;
}

constexpr std::array<u64, 64> initKingMasks() {
    std::array<u64, 64> masks = {};

    for (int square = 0; square != 64; square++) {
        u64 squareL = 1ULL << square;
        u64 mask = 0ULL;
        int file = fileOf(square);

        mask |= squareL << 8;
        mask |= squareL >> 8;

        if (file < 7) {
            mask |= squareL >> 1;
            mask |= squareL << 7;
            mask |= squareL >> 9;
        }

        if (file > 0) {
            mask |= squareL << 1;
            mask |= squareL >> 7;
            mask |= squareL << 9;
        }

        masks[square] = mask;
    }

    return masks;
}

constexpr std::array<std::array<u64, 64>, 2> initPawnMasks() {
    std::array<std::array<u64, 64>, 2> masks = {};

    for (int square = 0; square != 64; square++) {
        u64 mask = 0;
        u64 mask2 = 0;
        u64 squareL = 1ULL << square;
        int file = fileOf(square);

        if (file > 0) {
            mask  |= squareL << 9;
            mask2 |= squareL >> 7;
        }

        if (file < 7) {
            mask  |= squareL << 7;
            mask2 |= squareL >> 9;
        }

        masks[0][square] = mask2;
        masks[1][square] = mask;
    }

    return masks;
}

[[maybe_unused]] inline void printBB(u64 bitboard) {
    std::string bb;
    u64 relevantBit = 1ULL << 63;

    while (relevantBit) {
        std::cout << ((relevantBit & bitboard) >> (lsb(relevantBit))) << " ";

        if (fileOf(lsb(relevantBit)) == 7)
            std::cout << "\n";

        relevantBit >>= 1;
    }

    std::cout << std::endl;
}

inline u64 movePawn(u64 squareL, bool white) {
    return white ? squareL << 8 : squareL >> 8;
}

inline Piece makePromoPiece(int pt, Color c) {
    return Piece((pt + 1) + (!c * 6));
}

inline Piece makePiece(int pt, int c) {
    return Piece(pt + 6 * !c);
}

inline Color colorOf(int pc) {
    return pc < BLACK_PAWN;
}

#endif //MOLYBDENUM_BITSTUFF_H
