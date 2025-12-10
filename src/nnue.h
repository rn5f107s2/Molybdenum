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
    std::array<int16_t , L1_SIZE * INPUT_SIZE> weights0{};
    std::array<int16_t, L1_SIZE> bias0{};
    std::array<int16_t, L1_SIZE * OUTPUT_SIZE * 2> weights1{};
    std::array<int16_t, OUTPUT_SIZE> bias1{};
};

struct WDLHead {
    std::array<int16_t, L1_SIZE * 3 * 2> weights1{};
    std::array<int16_t, 3> bias1{};
};

class Net {
public:
    std::array<int16_t , L1_SIZE * INPUT_SIZE> weights0{};
    std::array<int16_t, L1_SIZE> bias0{};
    std::array<int16_t, L1_SIZE * OUTPUT_SIZE * 2> weights1{};
    std::array<int16_t, OUTPUT_SIZE> bias1{};
    std::array<std::array<int16_t, L1_SIZE>, 2> accumulator{};
    std::array<int16_t, L1_SIZE * 3 * 2> wdlWeights{};
    std::array<int16_t, 3> wdlBias{};
    Stack<std::array<std::array<int16_t, L1_SIZE>, 2>, MAXDEPTH> accumulatorStack;

    void initAccumulator(Position &pos);
    int calculate(Color c, uint64_t occupied, Piece* mailbox);
    std::tuple<float, float, float> getWDL(Color c);
    void loadDefaultNet();

    template<Toggle STATE> inline
    void toggleFeature(int piece, int square);
    template<Toggle STATE> inline
    void toggleFeature(Position& pos, uint64_t cleanBitboard, int piece, int square);
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

template<Toggle STATE> inline
void Net::toggleFeature(int piece, int square) {
    int indexWhite = index<WHITE>(piece, square);
    int indexBlack = index<BLACK>(piece, square);

    for (int l = 0; l != L1_SIZE; l++) {
        accumulator[WHITE][l] += weights0[indexWhite * L1_SIZE + l] * (!STATE ? -1 : 1);
        accumulator[BLACK][l] += weights0[indexBlack * L1_SIZE + l] * (!STATE ? -1 : 1);
    }
}

template<Toggle STATE> inline
void Net::toggleFeature(Position& pos, uint64_t cleanBitboard, int piece, int square) {
    int indexWhite = index<WHITE>(piece, square);
    int indexBlack = index<BLACK>(piece, square);

    while (cleanBitboard) {
        int wSq   = popLSB(cleanBitboard);
        int bSq   = wSq ^ 56;
        Piece wPc = pos.pieceOn(wSq);
        Piece bPc = makePiece(typeOf(wPc), !colorOf(wPc));

        int wOffset = indexWhite * L1_SIZE * 12 + wSq * 4 + L1_SIZE * wPc;
        int bOffset = indexBlack * L1_SIZE * 12 + bSq * 4 + L1_SIZE * bPc;

        for (int i = 0; i < 4; i++) {
            accumulator[WHITE][wSq * 4 + i] += weights0[wOffset + i] * (!STATE ? -1 : 1);
            accumulator[BLACK][bSq * 4 + i] += weights0[bOffset + i] * (!STATE ? -1 : 1);
        }
    }
}

inline void Net::refreshMiniAcc(Position& pos, Piece piece, int square) {
    uint64_t occupied = pos.getOccupied();

    int bSquare = square ^ 56;
    Piece bPiece = makePiece(typeOf(piece), !colorOf(piece));

    memcpy(&accumulator[WHITE][square * 4], &bias0[square * 4 + L1_SIZE * piece], 4 * sizeof(int16_t));
    memcpy(&accumulator[BLACK][square * 4], &bias0[square * 4 + L1_SIZE * piece], 4 * sizeof(int16_t));

    while (occupied) {
        int sq = popLSB(occupied);
        Piece pc = pos.pieceOn(sq);
        
        int indexWhite = index<WHITE>(pc, sq);
        int indexBlack = index<BLACK>(pc, sq);

        int wOffset = indexWhite * L1_SIZE * 12 +  square * 4 + L1_SIZE *  piece;
        int bOffset = indexBlack * L1_SIZE * 12 + bSquare * 4 + L1_SIZE * bPiece;

        for (int i = 0; i < 4; i++) {
            accumulator[WHITE][ square * 4 + i] += weights0[wOffset + i];
            accumulator[BLACK][bSquare * 4 + i] += weights0[bOffset + i];
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
