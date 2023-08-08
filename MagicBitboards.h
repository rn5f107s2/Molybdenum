#ifndef MOLYBDENUM_MAGICBITBOARDS_H
#define MOLYBDENUM_MAGICBITBOARDS_H

#include "BitStuff.h"
#include <cstring>

constexpr std::array<int, 64> bishopShifts =
        {58, 59, 59, 59, 59, 59, 59, 58,
         59, 59, 59, 59, 59, 59, 59, 59,
         59, 59, 57, 57, 57, 57, 59, 59,
         59, 59, 57, 55, 55, 57, 59, 59,
         59, 59, 57, 55, 55, 57, 59, 59,
         59, 59, 57, 57, 57, 57, 59, 59,
         59, 59, 59, 59, 59, 59, 59, 59,
         58, 59, 59, 59, 59, 59, 59, 58};

constexpr std::array<int, 64> rookShifts =
        {52, 53, 53, 53, 53, 53, 53, 52,
         53, 54, 54, 54, 54, 54, 54, 53,
         53, 54, 54, 54, 54, 54, 54, 53,
         53, 54, 54, 54, 54, 54, 54, 53,
         53, 54, 54, 54, 54, 54, 54, 53,
         53, 54, 54, 54, 54, 54, 54, 53,
         53, 54, 54, 54, 54, 54, 54, 53,
         52, 53, 53, 53, 53, 53, 53, 52};

constexpr int initialSeed = 0x5F10752;

template<Slider Type, bool MASK>
u64 getSliderAttacks(int square, u64 blocker) {
    constexpr bool ISROOK = Type == ROOK_S;
    u64 squareL = 1ULL << square;
    u64 stop = (((~lFIleOf(square) & edgeFiles) | (~lRankOf(square) & promotionRanks)) | blocker) & ~squareL;
    u64 attacks = 0;
    int shift1 = 7;
    int shift2 = 9;

    if (ISROOK) {
        shift1 = 1;
        shift2 = 8;
    }

    u64 bit = squareL;
    while ((bit & ~stop) && ((ISROOK && !(squareL & FILEA)) || (!ISROOK && !(squareL & FILEH)))) {
        bit <<= shift1;
        attacks |= bit;
    }

    bit = squareL;
    while (bit & ~stop && (ISROOK || !(squareL & FILEA))) {
        bit <<= shift2;
        attacks |= bit;
    }

    bit = squareL;
    while (bit & ~stop && (ISROOK || !(squareL & FILEH))) {
        bit >>= shift2;
        attacks |= bit;
    }

    bit = squareL;
    while (bit & ~stop && ((!ISROOK && !(squareL & FILEA)) || (ISROOK && !(squareL & FILEH)))) {
        bit >>= shift1;
        attacks |= bit;
    }

    if (MASK) {
        u64 edgeToRemove = 0ULL;
        u64 temp;
        u64 fileOfRook = (FILEA >> fileOf(square));
        u64 rankOfRook = (RANK1 << rankOf(square) * 8);

        temp = fileOfRook & edgeFiles;
        if (temp)
            edgeToRemove |= edgeFiles & ~temp;
        else
            edgeToRemove |= edgeFiles;

        temp = rankOfRook & promotionRanks;
        if (temp)
            edgeToRemove |= promotionRanks & ~temp;
        else
            edgeToRemove |= promotionRanks;

        attacks &= ~edgeToRemove;
    }

    if (Type == QUEEN_S)
        attacks |= getSliderAttacks<ROOK_S, MASK>(square, blocker);

    return attacks;
}

constexpr u64 randomULL(u64 seed){
    seed ^= (seed << 21);
    seed ^= (seed >> 35);
    seed ^= (seed << 4);
    return seed;
}

template <Slider TYPE>
void initMagics(std::array<u64, 64> &magics, std::array<std::array<u64, 4096>, 64> &table) {
    if (TYPE == QUEEN_S)
        return;

    constexpr bool ISROOK = TYPE == ROOK_S;

    std::array<int, 64> shifts = ISROOK ? rookShifts : bishopShifts;


    for (int square = 0; square != 64; square++) {
        bool done = false;
        u64 possibleMagic = randomULL(initialSeed);

        while (!done) {
            int loopCount = 0;
            possibleMagic = randomULL(possibleMagic);
            u64 blockers = 0;
            u64 mask = getSliderAttacks<TYPE, true>(square ,0ULL);
            memset(&table, 0ULL, table.size() * sizeof(typeof(table[0])));
            magics[square] = possibleMagic;

            do {
                u64 idx = ((mask & blockers) * possibleMagic) >> shifts[square];
                u64 attacks = getSliderAttacks<TYPE, false>(square, blockers);
                loopCount++;

                if (table[square][idx] == attacks || table[square][idx] == 0ULL) {
                    table[square][idx] = attacks;
                    blockers = (blockers - mask) & mask;
                } else
                    break;
            } while (blockers);

            if (loopCount >= int(1ULL << ((64 - shifts[square]) - 1)))
                done = true;
        }
        std::cout << "square " << square << ":\n";
        std::cout << magics[square] << "\n";
    }
}


#endif //MOLYBDENUM_MAGICBITBOARDS_H
