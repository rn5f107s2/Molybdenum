#include "Transpositiontable.h"
#include "Move.h"

TranspositionTable TT;

u64 positionToKey(std::array<u64, 13> &bbs, int castlingrights, u64 epSquare, bool sideToMove) {
    u64 key = 0ULL;

    for (int i = 0; i != 12; i++) {
        u64 pieceBB = bbs[i];

        while (pieceBB) {
            int square = popLSB(pieceBB);

            updateKey(i, square, key);
        }
    }

    updateKeyCastling(castlingrights, key);

    if (epSquare)
        updateKey(fileOf(lsb(epSquare)), key);

    if (!sideToMove)
        updateKey(key);

    return key;
}

TTEntry *TranspositionTable::probe(u64 key) {
    u64 idx = key % amountOfEntries;
    return &tt[idx];
}

void TranspositionTable::save(TTEntry *tte, u64 key, int score, TTBound bound, Move move, int depth, int plysInSearch) {
    if (   tte->key == key
        && depth + bound < tte->depth + tte->bound - 3)
        return;

    tte->key = key;
    tte->move = move;
    tte->bound = bound;
    tte->depth = depth;

    if (score > MAXMATE)
        score += plysInSearch;
    else if (score < -MAXMATE) {
        score -= plysInSearch;
    }

    tte->score = score;
}

void TranspositionTable::setSize(int sizeInMb) {
    size_t sizeInByte = sizeInMb * 1048576;
    tt = (TTEntry*) malloc(sizeInByte);
    u64 numberOfEntries = sizeInByte / sizeof(TTEntry);
    amountOfEntries = u64(numberOfEntries);
    clear();
    //std::cout << "TranspositionTable initialized with " << amountOfEntries << " entries\n";
}

void TranspositionTable::resize(int sizeInMb) {
    free(tt);
    setSize(sizeInMb);
}

void TranspositionTable::clear() {
    memset(tt, 0, amountOfEntries * sizeof(TTEntry));
}

void resizeTT(int sizeInMb) {
    TT.resize(sizeInMb);
}
