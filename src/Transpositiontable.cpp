#include "Transpositiontable.h"

u64 positionToKey(std::array<u64, 13> &bbs, int castlingrights, u64 epSquare, bool sideToMove) {
    u64 key = 0ULL;

    for (int i = 0; i != 12; i++) {
        u64 pieceBB = bbs[i];

        while (pieceBB) {
            int square = popLSB(pieceBB);

            updateKey(i, square, key);
        }
    }

    updateKey(castlingrights, key, true);

    if (epSquare)
        updateKey(fileOf(lsb(epSquare)), key);

    if (!sideToMove)
        updateKey(key);

    return key;
}