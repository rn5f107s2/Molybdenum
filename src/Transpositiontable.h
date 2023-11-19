#ifndef MOLYBDENUM_TRANSPOSITIONTABLE_H
#define MOLYBDENUM_TRANSPOSITIONTABLE_H

#include "Constants.h"
#include "BitStuff.h"
#include "MagicBitboards.h"
#include "Move.h"

enum TTBound {
    LOWER, UPPER, EXACT
};

struct TTEntry {
    int score;
    int depth;
    TTBound bound;
    Move move;
    u64 key;
};

class TranspositionTable{
public:
    TTEntry *probe(u64 key);
    static void save(TTEntry *tte, u64 key, int score, TTBound flag, Move move, int depth, int plys);
    void setSize(int sizeInMb);
    void resize(int sizeInMb);
    void clear();
private:
    TTEntry *tt;
    u64 amountOfEntries;
};

extern TranspositionTable TT;

void resizeTT(int sizeInMb);

constexpr std::array<u64, 781> genPRNs() {
    u64 initialSeed = 0x5F10752;
    u64 random = randomULL(initialSeed);
    std::array<u64, 781> pRNs{};

    for (int i = 0; i != 781; i++) {
        pRNs[i] = random;
        random = randomULL(random);
    }

    return pRNs;
}

constexpr std::array<u64, 781> pseudoRandomNumbers = genPRNs();
const int enPassantFileBaseIdx = 768;
const int castlingRightsBaseIdx = 776;

inline void updateKey(int piece, int square, u64 &key) {
    key ^= pseudoRandomNumbers[piece * 64 + square];
}

inline void updateKey(int enPassantFile, u64 &key) {
    key ^= pseudoRandomNumbers[enPassantFileBaseIdx + enPassantFile];
}

inline void updateKeyCastling(int castlingRights, u64 &key) {
    while (castlingRights)
        key ^= pseudoRandomNumbers[castlingRightsBaseIdx + popLSB(castlingRights)];
}

inline void updateKey(u64 &key) {
    key ^= pseudoRandomNumbers[780];
}

u64 positionToKey(std::array<u64, 13> &bbs, int castlingrights, u64 epSquare, bool sideToMove);


#endif //MOLYBDENUM_TRANSPOSITIONTABLE_H
