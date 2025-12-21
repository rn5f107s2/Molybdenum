#ifndef MOLYBDENUM_NNUE_H
#define MOLYBDENUM_NNUE_H

#include "Constants.h"
#include <array>
#include <string>
#include <tuple>
#include <algorithm>
#include <immintrin.h>
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
    void toggleFeature(Position& pos, uint64_t cleanBitboard, int piece, int square);
    template<Color ON_COLOR, Color OFF_COLOR>
    void addSub(Position& pos, uint64_t cleanBitboard, uint64_t white, int onPiece, int onSquare, int offPiece, int offSquare, int refreshPc, int refreshSq);
    template<Color ON_COLOR, Color OFF_COLOR, Color CAP_COLOR>
    inline void addSubSub(Position& pos, uint64_t cleanBitboard, int onPiece, int onSquare, int offPiece, int offSquare, int capPc, int capSq);
    template<Color C>
    inline void addaddSubSub(Position& pos, uint64_t cleanBitboard, int from, int to, int rFrom, int tRo, int rook, int king);
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
void Net::toggleFeature(Position& pos, uint64_t cleanBitboard, int piece, int square) {
    if (colorOf(piece)) {
        while (cleanBitboard) {
            int sq   = popLSB(cleanBitboard);
            Piece pc = pos.pieceOn(sq);

            int wOffset = index_new<WHITE>(pc, sq, piece, square);

            int offset = 4;

            int idx = sq * 8;

            // Load 8 weights
            __m128i w = _mm_loadu_si128((const __m128i *)(&weights0[0] + wOffset));

            // Load 8 accumulator values
            __m128i acc = _mm_loadu_si128((__m128i *)(&accumulator[0] + idx));

            // Accumulate
            if constexpr (STATE == On)
                acc = _mm_add_epi16(acc, w);
            else
                acc = _mm_sub_epi16(acc, w);

            // Store result
            _mm_storeu_si128((__m128i *)(&accumulator[0] + idx), acc);
        }
    } else {
        while (cleanBitboard) {
            int sq   = popLSB(cleanBitboard);
            Piece pc = pos.pieceOn(sq);

            int wOffset = index_new<WHITE>(pc, sq, piece, square);
            
            int offset = -4;

            int idx = sq * 8;

            // Load first 4 weights: wOffset + 0..3
            __m128i w_lo = _mm_loadl_epi64(
                (const __m128i *)(&weights0[0] + wOffset)
            );

            // Load second 4 weights: wOffset - 4..-1
            __m128i w_hi = _mm_loadl_epi64(
                (const __m128i *)(&weights0[0] + wOffset - 4)
            );

            // Combine into one vector: [lo | hi]
            __m128i w = _mm_unpacklo_epi64(w_lo, w_hi);

            // Load 8 accumulator values
            __m128i acc = _mm_loadu_si128((__m128i *)(&accumulator[0] + idx));

            // Accumulate
            if constexpr (STATE == On)
                acc = _mm_add_epi16(acc, w);
            else
                acc = _mm_sub_epi16(acc, w);

            // Store result
            _mm_storeu_si128((__m128i *)(&accumulator[0] + idx), acc);
        }
    }
}

template<Color C>
inline void refreshSingle(Net* net, int offset, int bpc, int bsq, int fpc, int fsq) {
    int idx = bsq * 8;

    // Load 8 weights
    __m128i w;
    if constexpr (C) {
        w = _mm_loadu_si128((const __m128i *)(&net->weights0[0] + offset));
    } else {
        // Load first 4 weights: wOffset + 0..3
        __m128i w_lo = _mm_loadl_epi64(
            (const __m128i *)(&net->weights0[0] + offset)
        );

        // Load second 4 weights: wOffset - 4..-1
        __m128i w_hi = _mm_loadl_epi64(
            (const __m128i *)(&net->weights0[0] + offset - 4)
        );

        // Combine into one vector: [lo | hi]
        w = _mm_unpacklo_epi64(w_lo, w_hi);
    }

    // Load 8 accumulator values
    __m128i acc = _mm_loadu_si128((__m128i *)(&net->accumulator[0] + idx));

    // Accumulate
    acc = _mm_add_epi16(acc, w);

    // Store result
    _mm_storeu_si128((__m128i *)(&net->accumulator[0] + idx), acc);   
}

template<Color ON_COLOR, Color OFF_COLOR>
inline void Net::addSub(Position& pos, uint64_t cleanBitboard, uint64_t white, int onPiece, int onSquare, int offPiece, int offSquare, int refreshPc, int refreshSq) {
    uint64_t w = cleanBitboard & white;
    uint64_t b = cleanBitboard & ~white;

    int bSquare = refreshSq ^ 56;
    Piece bPiece = makePiece(typeOf(refreshPc), !colorOf(refreshPc));

    memcpy(&accumulator[refreshSq * 4 * 2    ], &bias0[refreshSq  * 4 + L1_SIZE * refreshPc], 4 * sizeof(int16_t));
    memcpy(&accumulator[refreshSq * 4 * 2 + 4], &bias0[bSquare    * 4 + L1_SIZE * bPiece   ], 4 * sizeof(int16_t));

    if (colorOf(refreshPc))
        refreshSingle<WHITE>(this, index_new<WHITE>(refreshPc, refreshSq, refreshPc, refreshSq), refreshPc, refreshSq, refreshPc, refreshSq);
    else
        refreshSingle<BLACK>(this, index_new<WHITE>(refreshPc, refreshSq, refreshPc, refreshSq), refreshPc, refreshSq, refreshPc, refreshSq);

    while (w) {
        int sq   = popLSB(w);
        Piece pc = pos.pieceOn(sq);

        int  onOffset     = index_new<WHITE>(pc, sq,  onPiece, onSquare);
        int offOffset     = index_new<WHITE>(pc, sq, offPiece, offSquare);
        int refreshOffset = index_new<WHITE>(refreshPc, refreshSq, pc, sq);

        refreshSingle<WHITE>(this, refreshOffset, refreshPc, refreshSq, pc, sq);

        int idx = sq * 8;

        __m128i wA, wS;

        // Load 8 weights
        if constexpr (ON_COLOR) {
            wA = _mm_loadu_si128((const __m128i *)(&weights0[0] +  onOffset));
        } else {
            // Load first 4 weights: wOffset + 0..3
            __m128i wA_lo = _mm_loadl_epi64(
                (const __m128i *)(&weights0[0] + onOffset)
            );

            // Load second 4 weights: wOffset - 4..-1
            __m128i wA_hi = _mm_loadl_epi64(
                (const __m128i *)(&weights0[0] + onOffset - 4)
            );

            wA = _mm_unpacklo_epi64(wA_lo, wA_hi);
        }

        if constexpr (OFF_COLOR) {
            wS = _mm_loadu_si128((const __m128i *)(&weights0[0] + offOffset));
        } else {
            // Load first 4 weights: wOffset + 0..3
            __m128i wS_lo = _mm_loadl_epi64(
                (const __m128i *)(&weights0[0] + offOffset)
            );

            // Load second 4 weights: wOffset - 4..-1
            __m128i wS_hi = _mm_loadl_epi64(
                (const __m128i *)(&weights0[0] + offOffset - 4)
            );

            wS = _mm_unpacklo_epi64(wS_lo, wS_hi);
        }

        // Load 8 accumulator values
        __m128i acc = _mm_loadu_si128((__m128i *)(&accumulator[0] + idx));

        // Accumulate
        acc = _mm_add_epi16(acc, wA);
        acc = _mm_sub_epi16(acc, wS);

        // Store result
        _mm_storeu_si128((__m128i *)(&accumulator[0] + idx), acc);
    }

    while (b) {
        int sq   = popLSB(b);
        Piece pc = pos.pieceOn(sq);

        int  onOffset     = index_new<WHITE>(pc, sq,  onPiece, onSquare);
        int offOffset     = index_new<WHITE>(pc, sq, offPiece, offSquare);
        int refreshOffset = index_new<WHITE>(refreshPc, refreshSq, pc, sq);

        refreshSingle<BLACK>(this, refreshOffset, refreshPc, refreshSq, pc, sq);

        int idx = sq * 8;

        __m128i wA, wS;

        // Load 8 weights
        if constexpr (ON_COLOR) {
            wA = _mm_loadu_si128((const __m128i *)(&weights0[0] +  onOffset));
        } else {
            // Load first 4 weights: wOffset + 0..3
            __m128i wA_lo = _mm_loadl_epi64(
                (const __m128i *)(&weights0[0] + onOffset)
            );

            // Load second 4 weights: wOffset - 4..-1
            __m128i wA_hi = _mm_loadl_epi64(
                (const __m128i *)(&weights0[0] + onOffset - 4)
            );

            wA = _mm_unpacklo_epi64(wA_lo, wA_hi);
        }

        if constexpr (OFF_COLOR) {
            wS = _mm_loadu_si128((const __m128i *)(&weights0[0] + offOffset));
        } else {
            // Load first 4 weights: wOffset + 0..3
            __m128i wS_lo = _mm_loadl_epi64(
                (const __m128i *)(&weights0[0] + offOffset)
            );

            // Load second 4 weights: wOffset - 4..-1
            __m128i wS_hi = _mm_loadl_epi64(
                (const __m128i *)(&weights0[0] + offOffset - 4)
            );

            wS = _mm_unpacklo_epi64(wS_lo, wS_hi);
        }

        // Load 8 accumulator values
        __m128i acc = _mm_loadu_si128((__m128i *)(&accumulator[0] + idx));

        // Accumulate
        acc = _mm_add_epi16(acc, wA);
        acc = _mm_sub_epi16(acc, wS);

        // Store result
        _mm_storeu_si128((__m128i *)(&accumulator[0] + idx), acc);
    }
}

template<Color ON_COLOR, Color OFF_COLOR, Color CAP_COLOR>
inline void Net::addSubSub(Position& pos, uint64_t cleanBitboard, int onPiece, int onSquare, int offPiece, int offSquare, int capPiece, int capSq) {
    while (cleanBitboard) {
        int sq   = popLSB(cleanBitboard);
        Piece pc = pos.pieceOn(sq);

        int  onOffset = index_new<WHITE>(pc, sq,  onPiece,  onSquare);
        int offOffset = index_new<WHITE>(pc, sq, offPiece, offSquare);
        int capOffset = index_new<WHITE>(pc, sq, capPiece,     capSq);

        int idx = sq * 8;

        __m128i wA, wS1, wS2;

        // Load 8 weights
        if constexpr (ON_COLOR) {
            wA = _mm_loadu_si128((const __m128i *)(&weights0[0] +  onOffset));
        } else {
            // Load first 4 weights: wOffset + 0..3
            __m128i wA_lo = _mm_loadl_epi64(
                (const __m128i *)(&weights0[0] + onOffset)
            );

            // Load second 4 weights: wOffset - 4..-1
            __m128i wA_hi = _mm_loadl_epi64(
                (const __m128i *)(&weights0[0] + onOffset - 4)
            );

            wA = _mm_unpacklo_epi64(wA_lo, wA_hi);
        }

        if constexpr (OFF_COLOR) {
            wS1 = _mm_loadu_si128((const __m128i *)(&weights0[0] + offOffset));
        } else {
            // Load first 4 weights: wOffset + 0..3
            __m128i wS_lo = _mm_loadl_epi64(
                (const __m128i *)(&weights0[0] + offOffset)
            );

            // Load second 4 weights: wOffset - 4..-1
            __m128i wS_hi = _mm_loadl_epi64(
                (const __m128i *)(&weights0[0] + offOffset - 4)
            );

            wS1 = _mm_unpacklo_epi64(wS_lo, wS_hi);
        }

        if constexpr (CAP_COLOR) {
            wS2 = _mm_loadu_si128((const __m128i *)(&weights0[0] + capOffset));
        } else {
            // Load first 4 weights: wOffset + 0..3
            __m128i wS_lo = _mm_loadl_epi64(
                (const __m128i *)(&weights0[0] + capOffset)
            );

            // Load second 4 weights: wOffset - 4..-1
            __m128i wS_hi = _mm_loadl_epi64(
                (const __m128i *)(&weights0[0] + capOffset - 4)
            );

            wS2 = _mm_unpacklo_epi64(wS_lo, wS_hi);
        }

        // Load 8 accumulator values
        __m128i acc = _mm_loadu_si128((__m128i *)(&accumulator[0] + idx));

        // Accumulate
        acc = _mm_add_epi16(acc, wA );
        acc = _mm_sub_epi16(acc, wS1);
        acc = _mm_sub_epi16(acc, wS2);

        // Store result
        _mm_storeu_si128((__m128i *)(&accumulator[0] + idx), acc);
    }
}

template<Color C>
inline void Net::addaddSubSub(Position& pos, uint64_t cleanBitboard, int from, int to, int rFrom, int rTo, int rook, int king) {
    while (cleanBitboard) {
        int sq   = popLSB(cleanBitboard);
        Piece pc = pos.pieceOn(sq);

        int add1Offset = index_new<WHITE>(pc, sq, king, to);
        int add2Offset = index_new<WHITE>(pc, sq, rook, rTo);
        int sub1Offset = index_new<WHITE>(pc, sq, king, from);
        int sub2Offset = index_new<WHITE>(pc, sq, rook, rFrom);

        int idx = sq * 8;

        __m128i wA1, wA2, wS1, wS2;

        // Load 8 accumulator values
        __m128i acc = _mm_loadu_si128((__m128i *)(&accumulator[0] + idx));

        // Load 8 weights
        if constexpr (C) {
            wA1 = _mm_loadu_si128((const __m128i *)(&weights0[0] +  add1Offset));
        } else {
            // Load first 4 weights: wOffset + 0..3
            __m128i wA_lo = _mm_loadl_epi64(
                (const __m128i *)(&weights0[0] + add1Offset)
            );

            // Load second 4 weights: wOffset - 4..-1
            __m128i wA_hi = _mm_loadl_epi64(
                (const __m128i *)(&weights0[0] + add1Offset - 4)
            );

            wA1 = _mm_unpacklo_epi64(wA_lo, wA_hi);
        }

        if constexpr (C) {
            wA2 = _mm_loadu_si128((const __m128i *)(&weights0[0] +  add2Offset));
        } else {
            // Load first 4 weights: wOffset + 0..3
            __m128i wA_lo = _mm_loadl_epi64(
                (const __m128i *)(&weights0[0] + add2Offset)
            );

            // Load second 4 weights: wOffset - 4..-1
            __m128i wA_hi = _mm_loadl_epi64(
                (const __m128i *)(&weights0[0] + add2Offset - 4)
            );

            wA2 = _mm_unpacklo_epi64(wA_lo, wA_hi);
        }

        acc = _mm_add_epi16(acc, wA1);
        acc = _mm_add_epi16(acc, wA2);

        if constexpr (C) {
            wS1 = _mm_loadu_si128((const __m128i *)(&weights0[0] + sub1Offset));
        } else {
            // Load first 4 weights: wOffset + 0..3
            __m128i wS_lo = _mm_loadl_epi64(
                (const __m128i *)(&weights0[0] + sub1Offset)
            );

            // Load second 4 weights: wOffset - 4..-1
            __m128i wS_hi = _mm_loadl_epi64(
                (const __m128i *)(&weights0[0] + sub1Offset - 4)
            );

            wS1 = _mm_unpacklo_epi64(wS_lo, wS_hi);
        }

        if constexpr (C) {
            wS2 = _mm_loadu_si128((const __m128i *)(&weights0[0] + sub2Offset));
        } else {
            // Load first 4 weights: wOffset + 0..3
            __m128i wS_lo = _mm_loadl_epi64(
                (const __m128i *)(&weights0[0] + sub2Offset)
            );

            // Load second 4 weights: wOffset - 4..-1
            __m128i wS_hi = _mm_loadl_epi64(
                (const __m128i *)(&weights0[0] + sub2Offset - 4)
            );

            wS2 = _mm_unpacklo_epi64(wS_lo, wS_hi);
        }

        // Accumulate
        acc = _mm_sub_epi16(acc, wS1);
        acc = _mm_sub_epi16(acc, wS2);

        // Store result
        _mm_storeu_si128((__m128i *)(&accumulator[0] + idx), acc);
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
    uint64_t white = pos.getOccupied<WHITE>();
    uint64_t black = pos.getOccupied<BLACK>();

    int bSquare = square ^ 56;
    Piece bPiece = makePiece(typeOf(piece), !colorOf(piece));

    memcpy(&accumulator[square * 4 * 2    ], &bias0[square  * 4 + L1_SIZE * piece ], 4 * sizeof(int16_t));
    memcpy(&accumulator[square * 4 * 2 + 4], &bias0[bSquare * 4 + L1_SIZE * bPiece], 4 * sizeof(int16_t));

    while (white) {
        int sq = popLSB(white);
        Piece pc = pos.pieceOn(sq);

        int wOffset = index_new<WHITE>(piece, square, pc, sq);

        refreshSingle<WHITE>(this, wOffset, piece, square, pc, sq);
    }

    while (black) {
        int sq = popLSB(black);
        Piece pc = pos.pieceOn(sq);

        int wOffset = index_new<WHITE>(piece, square, pc, sq);

        int idx = square * 8;

        refreshSingle<BLACK>(this, wOffset, piece, square, pc, sq);
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
