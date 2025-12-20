#ifndef MOLYBDENUM_NNUE_H
#define MOLYBDENUM_NNUE_H

#include "Constants.h"
#include <array>
#include <string>
#include <tuple>
#include <algorithm>
#include "BitStuff.h"
#include "Utility.h"
#include "Position.h"

enum Toggle {
    Off, On
};

static const int INPUT_SIZE = 12 * 64;
static const int L1_SIZE = 256;
static const int OUTPUT_SIZE = 1;
static const int NET_SIZE = 3;
static const std::array<int, NET_SIZE> LAYER_SIZE = {INPUT_SIZE, L1_SIZE, OUTPUT_SIZE};

struct Weights {
    std::array<int16_t , L1_SIZE * INPUT_SIZE * 12> weights0{};
    std::array<int16_t, L1_SIZE * 12> bias0{};
    std::array<int16_t, L1_SIZE * OUTPUT_SIZE * 2 * 12> weights1{};
    std::array<int16_t, OUTPUT_SIZE> bias1{};
};

struct WDLHead {
    std::array<int16_t, L1_SIZE * 3 * 2> weights1{};
    std::array<int16_t, 3> bias1{};
};

const extern Weights defaultWeights;

class Net {
public:
    std::array<int16_t , L1_SIZE * INPUT_SIZE * 12> weights0{};
    std::array<int16_t, L1_SIZE * 12> bias0{};
    std::array<int16_t, L1_SIZE * OUTPUT_SIZE * 2 * 12> weights1{};
    std::array<int16_t, OUTPUT_SIZE> bias1{};
    std::array<int16_t, L1_SIZE * 2> accumulator{};
    std::array<int16_t, L1_SIZE * 3 * 2> wdlWeights{};
    std::array<int16_t, 3> wdlBias{};
    Stack<std::array<int16_t, L1_SIZE * 2>, MAXDEPTH> accumulatorStack;

    void initAccumulator(Position &pos);
    template<Color C>
    int calculate(uint64_t occupied, Piece* mailbox);
    std::tuple<float, float, float> getWDL(Color c);
    void loadDefaultNet();

    template<Toggle STATE> inline
    void toggleFeature(int piece, int square);
    template<Toggle STATE> inline
    void toggleFeature(Position& pos, uint64_t cleanBitboard, uint64_t whiteBitboard, int piece, int square);
    void refreshMiniAcc(Position& pos, Piece piece, int square);
    inline void moveFeature(int piece, int from, int to);
    inline void pushAccToStack();
    inline void popAccStack();
};

inline int screlu(int16_t input) {
    int clamped = std::clamp(input, int16_t(0), int16_t(255));
    return clamped * clamped;
}

template<Color C> inline
int index(int pc, int sq) {
    int square = C ? sq : sq ^ 56;
    int piece  = C ? pc : makePiece(typeOf(pc), !colorOf(pc));

    return piece * 64 + square;
}

template<Color C> inline
int index_old(int bucketPc, int bucketSq, int featurePc, int featureSq) {
    int idx = index<C>(featurePc, featureSq);

    // indexWhite * L1_SIZE * 12 + wSq * 4 + L1_SIZE * wPc
    return idx * L1_SIZE * 12 + bucketSq * 4 + L1_SIZE * bucketPc;
}

template<Color C> inline
int index_new(int bucketPc, int bucketSq, int featurePc, int featureSq) {
    int bpt = typeOf(bucketPc);
    int fpt = typeOf(featurePc);
    int bpc = colorOf(bucketPc) == C;
    int fpc = colorOf(featurePc) == C;
    int bsq = colorOf(bucketPc)  ? bucketSq  : bucketSq  ^ 56;
    int fsq = colorOf(featurePc) ? featureSq : featureSq ^ 56;
    int ci  = ((bpc ^ fpc) << 1) | !fpc;

    return fpt * 64 * 64 * 4 * 4 * 6 + fsq * 64 * 4 * 4 * 6 + bpt * 64 * 4 * 4 + bsq * 4 * 4 + ci * 4;
}

template<Toggle STATE> inline
void Net::toggleFeature(int piece, int square) {
    int indexWhite = index<WHITE>(piece, square);
    int indexBlack = index<BLACK>(piece, square);

    std::cout << "trololol" << std::endl;

    for (int l = 0; l != L1_SIZE; l++) {
        accumulator[l] += weights0[indexWhite * L1_SIZE + l] * (!STATE ? -1 : 1);
        accumulator[l] += weights0[indexBlack * L1_SIZE + l] * (!STATE ? -1 : 1);
    }
}

template<Toggle STATE> inline
void Net::toggleFeature(Position& pos, uint64_t cleanBitboard, uint64_t whiteBitboard, int piece, int square) {
    while (cleanBitboard & whiteBitboard) {
        int sq   = popLSB(cleanBitboard);
        Piece pc = pos.pieceOn(sq);

        int wOffset = index_new<WHITE>(pc, sq, piece, square);

        for (int i = 0; i < 4; i++) {
            accumulator[(sq * 4 * 2) + i    ] += weights0[wOffset +     i] * (!STATE ? -1 : 1);
            accumulator[(sq * 4 * 2) + 4 + i] += weights0[wOffset + 4 + i] * (!STATE ? -1 : 1);
        }
    }

    while (cleanBitboard & ~whiteBitboard) {
        int sq   = popLSB(cleanBitboard);
        Piece pc = pos.pieceOn(sq);

        int wOffset = index_new<WHITE>(pc, sq, piece, square);

        for (int i = 0; i < 4; i++) {
            accumulator[(sq * 4 * 2) + i    ] += weights0[wOffset + 4 + i] * (!STATE ? -1 : 1);
            accumulator[(sq * 4 * 2) + 4 + i] += weights0[wOffset     + i] * (!STATE ? -1 : 1);
        }
    }
}


template<Color C> inline
int Net::calculate(uint64_t occupied, Piece* mailbox) {
    int output = 0;

    while (occupied) {
        int sq = popLSB(occupied);
        int nextSq = lsb(occupied);

        int ourPiece   = mailbox[sq ^ (56 * (C == BLACK))];
        int theirPiece = makePiece(typeOf(ourPiece), !colorOf(ourPiece));

        if (C == BLACK) {
            int temp = ourPiece;
            ourPiece = theirPiece;
            theirPiece = temp;
        }

        for (int i = 0; i < 4; i++) {
            int nUs   = ((sq ^ (56 * (C == BLACK))) * 4 * 2) + (4 * (C == BLACK)) + i;
            int nThem = ((sq ^ (56 * (C == BLACK))) * 4 * 2) + (4 * (C == WHITE)) + i;

            output += screlu(accumulator[nUs  ]) * weights1[256 * 2 * ourPiece + (sq * 4 * 2) + i];
            output += screlu(accumulator[nThem]) * weights1[256 * 2 * ourPiece + (sq * 4 * 2) + i + 4];
        }
    }

    return ((output / 255) + bias1[0]) * 133 / (64 * 255);
}

inline void Net::refreshMiniAcc(Position& pos, Piece piece, int square) {
    uint64_t occupied = pos.getOccupied();

    int bSquare = square ^ 56;
    Piece bPiece = makePiece(typeOf(piece), !colorOf(piece));

    memcpy(&accumulator[square * 4 * 2    ], &bias0[square  * 4 + L1_SIZE * piece ], 4 * sizeof(int16_t));
    memcpy(&accumulator[square * 4 * 2 + 4], &bias0[bSquare * 4 + L1_SIZE * bPiece], 4 * sizeof(int16_t));

    while (occupied) {
        int sq = popLSB(occupied);
        Piece pc = pos.pieceOn(sq);

        int wOffset = index_new<WHITE>(piece, square, pc, sq);
        int bOffset = index_new<BLACK>(piece, square, pc, sq);

        for (int i = 0; i < 4; i++) {
            accumulator[square * 4 * 2     + i] += weights0[wOffset + i];
            accumulator[square * 4 * 2 + 4 + i] += weights0[bOffset + i];
        }
    }
}

inline void Net::moveFeature(int piece, int from, int to) {
    toggleFeature<Off>(piece, from);
    toggleFeature<On >(piece, to);
}

inline void Net::pushAccToStack() {
    accumulatorStack.push(accumulator);
}

inline void Net::popAccStack() {
    accumulatorStack.pop();
    accumulator = accumulatorStack.top();
}


#endif //MOLYBDENUM_NNUE_H
