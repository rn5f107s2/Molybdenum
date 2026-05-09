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

static constexpr int INPUT_SIZE = 12 * 64;
static constexpr int MINI_ACC_SIZE = 16;
static constexpr int L1_SIZE = MINI_ACC_SIZE * 64;
static constexpr int OUTPUT_SIZE = 1;

static constexpr int NUM_REGS      = (MINI_ACC_SIZE * 2) / (sizeof(__m256i) / sizeof(int16_t));

static constexpr int NUM_REGS_PERS = MINI_ACC_SIZE / (sizeof(__m256i) / sizeof(int16_t));
static constexpr int NUM_REGS_DUAL = 2 * NUM_REGS_PERS == NUM_REGS? 0 : 1;

static_assert(NUM_REGS == NUM_REGS_PERS * 2 + NUM_REGS_DUAL);
static_assert((MINI_ACC_SIZE * 2) % (sizeof(__m256i) / sizeof(int16_t)) == 0);

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
    const std::array<int16_t , L1_SIZE * INPUT_SIZE * 12>& weights0;
    const std::array<int16_t, L1_SIZE * 12>& bias0;
    const std::array<int16_t, L1_SIZE * OUTPUT_SIZE * 2 * 12>& weights1;
    const std::array<int16_t, OUTPUT_SIZE>& bias1;
    std::array<int16_t, L1_SIZE * 2> accumulator{};
    std::array<int16_t, L1_SIZE * 3 * 2> wdlWeights{};
    std::array<int16_t, 3> wdlBias{};
    Stack<std::array<int16_t, L1_SIZE * 2>, MAXDEPTH> accumulatorStack;

    Net() : weights0(defaultWeights.weights0), bias0(defaultWeights.bias0), weights1(defaultWeights.weights1), bias1(defaultWeights.bias1) {}

    void initAccumulator(Position &pos);
    template<Color C>
    int calculate(uint64_t occupied, Piece* mailbox);
    std::tuple<float, float, float> getWDL(Color c);
    void loadDefaultNet();

    template<Color C> inline
    void loadWeightsVecDual(__m256i& w, int offset, int i);

    template<Color C> inline
    void loadWeightsVec(__m256i& w, int offset, int i);

    template<Color C> inline
    void loadWeightsVecs(__m256i* w, int offset);

    template<Color ON_COLOR, Color OFF_COLOR>
    void addSubSingle(int sq, int onOffset, int offOffset);

    template<Color ON_COLOR, Color OFF_COLOR, Color CAP_COLOR>
    void addSubSubSingle(int sq, int onOffset, int offOffset, int capOffset);

    template<Color C>
    void addaddSubSubSingle(int sq, int add1Offset, int add2Offset, int sub1Offset, int sub2Offset);

    template<Color ON_COLOR, Color OFF_COLOR>
    void addSub(Position& pos, uint64_t cleanBitboard, uint64_t white, int onPiece, int onSquare, int offPiece, int offSquare, int refreshPc, int refreshSq);
    template<Color ON_COLOR, Color OFF_COLOR, Color CAP_COLOR>
    inline void addSubSub(Position& pos, uint64_t cleanBitboard, uint64_t white, int onPiece, int onSquare, int offPiece, int offSquare, int capPiece, int capSq, int refreshPc, int refreshSq);
    template<Color C>
    inline void addaddSubSub(Position& pos, uint64_t cleanBitboard, int from, int to, int rFrom, int tRo, int rook, int king);
    void refreshMiniAcc(Position& pos, Piece piece, int square);
    inline void moveFeature(int piece, int from, int to);
    inline void pushAccToStack(uint64_t occupied);
    inline void popAccStack();
};

inline int screlu(int16_t input) {
    int clamped = std::clamp(input, int16_t(0), int16_t(403));
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
    return idx * L1_SIZE * 12 + bucketSq * MINI_ACC_SIZE + L1_SIZE * bucketPc;
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

    return   fpt * 64 * 64 * MINI_ACC_SIZE * 4 * 6 
           + fsq * 64 * MINI_ACC_SIZE * 4 * 6 
           + bpt * 64 * MINI_ACC_SIZE * 4 
           + bsq * MINI_ACC_SIZE * 4 
           + ci * MINI_ACC_SIZE;
}

template<Color C, Color FPC> inline
int index_new(int bucketPc, int bucketSq, int featurePc, int featureSq) {
    int bpt = typeOf(bucketPc);
    int fpt = typeOf(featurePc);
    int bpc = colorOf(bucketPc);
    int bsq = bpc ? bucketSq  : bucketSq  ^ 56;
    int fsq = FPC ? featureSq : featureSq ^ 56;
    int ci  = ((bpc ^ FPC) << 1) | !FPC;

    return     fpt * 64 * 64 * MINI_ACC_SIZE * 4 * 6
             + fsq * 64 * MINI_ACC_SIZE * 4 * 6 
             + bpt * 64 * MINI_ACC_SIZE * 4 
             + bsq * MINI_ACC_SIZE * 4 
             + ci * MINI_ACC_SIZE;
}

template<Color C, Color BPC, Color FPC> inline
int index_new(int bucketPc, int bucketSq, int featurePc, int featureSq) {
    int bpt = typeOf(bucketPc);
    int fpt = typeOf(featurePc);
    int bsq = BPC ? bucketSq  : bucketSq  ^ 56;
    int fsq = FPC ? featureSq : featureSq ^ 56;
    int ci  = ((BPC ^ FPC) << 1) | !FPC;

    return    fpt * 64 * 64 * MINI_ACC_SIZE * 4 * 6 
            + fsq * 64 * MINI_ACC_SIZE * 4 * 6 
            + bpt * 64 * MINI_ACC_SIZE * 4 
            + bsq * MINI_ACC_SIZE * 4 
            + ci * MINI_ACC_SIZE;
}

template<Color C> inline
void Net::loadWeightsVecDual(__m256i& w, int offset, int i) {
    if constexpr (C) {
        w = _mm256_loadu_si256((const __m256i *)(&weights0[0] + offset + MINI_ACC_SIZE * i));
    } else {
        constexpr int INT16_PER_REG = (sizeof(__m256i) / sizeof(int16_t));
        constexpr int HALF = INT16_PER_REG / 2;

        w = _mm256_loadu2_m128i(
            (const __m128i*)(&weights0[offset - MINI_ACC_SIZE * i - HALF]),
            (const __m128i*)(&weights0[offset - MINI_ACC_SIZE * i       ])
        );
    }
}

template<Color C> inline
void Net::loadWeightsVecs(__m256i* w, int offset) {
    int i = 0;

    for (; i < NUM_REGS_PERS; i++)
        loadWeightsVec<C>(w[i], offset, i);

    // untested
    for (; i < NUM_REGS_PERS + NUM_REGS_DUAL; i++) 
        loadWeightsVecDual<C>(w[i], offset, i);

    for (; i < NUM_REGS; i++)
        loadWeightsVec<C>(w[i], offset, i);
}

template<Color C> inline
void Net::loadWeightsVec(__m256i& w, int offset, int i) {
    if constexpr (C) {
        w = _mm256_loadu_si256((const __m256i *)(&weights0[0] + offset + 16 * i));
    } else {
        w = _mm256_loadu_si256((const __m256i *)(&weights0[0] + offset - 16 * i));
    }
}

template<Color C> 
inline void refreshSingle(Net* net, int offset, int bsq) {
    int idx = bsq * MINI_ACC_SIZE * 2;

    __m256i w  [NUM_REGS];
    __m256i acc[NUM_REGS];

    net->loadWeightsVecs<C>(w, offset);

    for (int i = 0; i < NUM_REGS; i++)
        acc[i] = _mm256_loadu_si256((__m256i *)(&net->accumulator[0] + idx + 16 * i));

    for (int i = 0; i < NUM_REGS; i++)
        acc[i] = _mm256_add_epi16(acc[i], w[i]);

    for (int i = 0; i < NUM_REGS; i++)
        _mm256_storeu_si256((__m256i *)(&net->accumulator[0] + idx + 16 * i), acc[i]);  
}

template<Color ON_COLOR, Color OFF_COLOR>
inline void Net::addSubSingle(int sq, int onOffset, int offOffset) {
    int idx = sq * MINI_ACC_SIZE * 2;

    __m256i wA[NUM_REGS];
    __m256i wS[NUM_REGS];
    __m256i acc[NUM_REGS];

    loadWeightsVecs< ON_COLOR>(wA,  onOffset);
    loadWeightsVecs<OFF_COLOR>(wS, offOffset);

    for (int i = 0; i < NUM_REGS; i++)
        acc[i] = _mm256_loadu_si256((__m256i *)(&accumulator[0] + idx + 16 * i));

    for (int i = 0; i < NUM_REGS; i++) {
        acc[i] = _mm256_add_epi16(acc[i], wA[i]);
        acc[i] = _mm256_sub_epi16(acc[i], wS[i]);
    }

    for (int i = 0; i < NUM_REGS; i++)
        _mm256_storeu_si256((__m256i *)(&accumulator[0] + idx + 16 * i), acc[i]);
}

template<Color ON_COLOR, Color OFF_COLOR, Color CAP_COLOR>
inline void Net::addSubSubSingle(int sq, int onOffset, int offOffset, int capOffset) {
    int idx = sq * MINI_ACC_SIZE * 2;

    __m256i wA [NUM_REGS]; 
    __m256i wS1[NUM_REGS];
    __m256i wS2[NUM_REGS];
    __m256i acc[NUM_REGS];

    loadWeightsVecs<ON_COLOR >(wA , onOffset );
    loadWeightsVecs<OFF_COLOR>(wS1, offOffset);
    loadWeightsVecs<CAP_COLOR>(wS2, capOffset);

    for (int i = 0; i < NUM_REGS; i++)
        acc[i] = _mm256_loadu_si256((__m256i *)(&accumulator[0] + idx + 16 * i));

    for (int i = 0; i < NUM_REGS; i++) {
        acc[i] = _mm256_add_epi16(acc[i], wA [i]);
        acc[i] = _mm256_sub_epi16(acc[i], wS1[i]);
        acc[i] = _mm256_sub_epi16(acc[i], wS2[i]);
    }

    for (int i = 0; i < NUM_REGS; i++)
        _mm256_storeu_si256((__m256i *)(&accumulator[0] + idx + 16 * i), acc[i]);
}

template<Color C>
inline void Net::addaddSubSubSingle(int sq, int add1Offset, int add2Offset, int sub1Offset, int sub2Offset) {
    int idx = sq * MINI_ACC_SIZE * 2;

    __m256i wA1[NUM_REGS];
    __m256i wA2[NUM_REGS];
    __m256i wS1[NUM_REGS];
    __m256i wS2[NUM_REGS];
    __m256i acc[NUM_REGS];

    for (int i = 0; i < NUM_REGS; i++)
        acc[i] = _mm256_loadu_si256((__m256i *)(&accumulator[0] + idx + 16 * i));

    loadWeightsVecs<C>(wA1, add1Offset);
    loadWeightsVecs<C>(wA2, add2Offset);

    for (int i = 0; i < NUM_REGS; i++) {
        acc[i] = _mm256_add_epi16(acc[i], wA1[i]);
        acc[i] = _mm256_add_epi16(acc[i], wA2[i]);
    }

    loadWeightsVecs<C>(wS1, sub1Offset);
    loadWeightsVecs<C>(wS2, sub2Offset);

    for (int i = 0; i < NUM_REGS; i++) {
        acc[i] = _mm256_sub_epi16(acc[i], wS1[i]);
        acc[i] = _mm256_sub_epi16(acc[i], wS2[i]);
    }

    for (int i = 0; i < NUM_REGS; i++)
        _mm256_storeu_si256((__m256i *)(&accumulator[0] + idx + 16 * i), acc[i]);
}

template<Color ON_COLOR, Color OFF_COLOR>
inline void Net::addSub(Position& pos, uint64_t cleanBitboard, uint64_t white, int onPiece, int onSquare, int offPiece, int offSquare, int refreshPc, int refreshSq) {
    uint64_t w = cleanBitboard & white;
    uint64_t b = cleanBitboard & ~white;

    constexpr bool RPC = ON_COLOR; // colorOf(refreshPc) == colorOf(movedPiece) == ON_COLOR

    int biasIndex = L1_SIZE * 2 * typeOf(refreshPc) + (RPC ? refreshSq : refreshSq ^ 56) * MINI_ACC_SIZE * 2;

    memcpy(&accumulator[refreshSq * MINI_ACC_SIZE * 2                ], &bias0[biasIndex + MINI_ACC_SIZE * !RPC], MINI_ACC_SIZE * sizeof(int16_t));
    memcpy(&accumulator[refreshSq * MINI_ACC_SIZE * 2 + MINI_ACC_SIZE], &bias0[biasIndex + MINI_ACC_SIZE *  RPC], MINI_ACC_SIZE * sizeof(int16_t));

    auto& t = accumulatorStack.at(accumulatorStack.getSize());

    while (w) {
        int sq   = popLSB(w);
        Piece pc = pos.pieceOn(sq);

        int  onOffset     = index_new<WHITE, WHITE, ON_COLOR >(pc, sq,  onPiece, onSquare);
        int offOffset     = index_new<WHITE, WHITE, OFF_COLOR>(pc, sq, offPiece, offSquare);
        int refreshOffset = index_new<WHITE, WHITE>(refreshPc, refreshSq, pc, sq);

        refreshSingle<WHITE>(this, refreshOffset, refreshSq);

        addSubSingle<ON_COLOR, OFF_COLOR>(sq, onOffset, offOffset);

        memcpy(&t[sq * MINI_ACC_SIZE * 2], &accumulator[sq * MINI_ACC_SIZE * 2], MINI_ACC_SIZE * 2 * sizeof(int16_t));
    }

    while (b) {
        int sq   = popLSB(b);
        Piece pc = pos.pieceOn(sq);

        int  onOffset     = index_new<WHITE, BLACK, ON_COLOR >(pc, sq,  onPiece, onSquare);
        int offOffset     = index_new<WHITE, BLACK, OFF_COLOR>(pc, sq, offPiece, offSquare);
        int refreshOffset = index_new<WHITE, BLACK>(refreshPc, refreshSq, pc, sq);

        refreshSingle<BLACK>(this, refreshOffset, refreshSq);

        addSubSingle<ON_COLOR, OFF_COLOR>(sq, onOffset, offOffset);

        memcpy(&t[sq * MINI_ACC_SIZE * 2], &accumulator[sq * MINI_ACC_SIZE * 2], MINI_ACC_SIZE * 2 * sizeof(int16_t));
    }

    memcpy(&t[refreshSq * MINI_ACC_SIZE * 2], &accumulator[refreshSq * MINI_ACC_SIZE * 2], MINI_ACC_SIZE * 2 * sizeof(int16_t));

    accumulatorStack.incSize();
}

template<Color ON_COLOR, Color OFF_COLOR, Color CAP_COLOR>
inline void Net::addSubSub(Position& pos, uint64_t cleanBitboard, uint64_t white, int onPiece, int onSquare, int offPiece, int offSquare, int capPiece, int capSq, int refreshPc, int refreshSq) {
    uint64_t w = cleanBitboard &  white;
    uint64_t b = cleanBitboard & ~white;

    constexpr bool RPC = ON_COLOR; // colorOf(refreshPc) == colorOf(movedPiece) == ON_COLOR

    int biasIndex = L1_SIZE * 2 * typeOf(refreshPc) + (RPC ? refreshSq : refreshSq ^ 56) * MINI_ACC_SIZE * 2;

    memcpy(&accumulator[refreshSq * MINI_ACC_SIZE * 2                ], &bias0[biasIndex + MINI_ACC_SIZE * !RPC], MINI_ACC_SIZE * sizeof(int16_t));
    memcpy(&accumulator[refreshSq * MINI_ACC_SIZE * 2 + MINI_ACC_SIZE], &bias0[biasIndex + MINI_ACC_SIZE *  RPC], MINI_ACC_SIZE * sizeof(int16_t));

    auto& t = accumulatorStack.at(accumulatorStack.getSize());

    while (w) {
        int sq   = popLSB(w);
        Piece pc = pos.pieceOn(sq);

        int  onOffset     = index_new<WHITE, WHITE, ON_COLOR >(pc, sq,  onPiece,  onSquare);
        int offOffset     = index_new<WHITE, WHITE, OFF_COLOR>(pc, sq, offPiece, offSquare);
        int capOffset     = index_new<WHITE, WHITE, CAP_COLOR>(pc, sq, capPiece,     capSq);
        int refreshOffset = index_new<WHITE, WHITE>(refreshPc, refreshSq, pc, sq);

        refreshSingle<WHITE>(this, refreshOffset, refreshSq);
    
        addSubSubSingle<ON_COLOR, OFF_COLOR, CAP_COLOR>(sq, onOffset, offOffset, capOffset);

        memcpy(&t[sq * MINI_ACC_SIZE * 2], &accumulator[sq * MINI_ACC_SIZE * 2], MINI_ACC_SIZE * 2 * sizeof(int16_t));
    }

    while (b) {
        int sq   = popLSB(b);
        Piece pc = pos.pieceOn(sq);

        int  onOffset = index_new<WHITE, BLACK, ON_COLOR >(pc, sq,  onPiece,  onSquare);
        int offOffset = index_new<WHITE, BLACK, OFF_COLOR>(pc, sq, offPiece, offSquare);
        int capOffset = index_new<WHITE, BLACK, CAP_COLOR>(pc, sq, capPiece,     capSq);
        int refreshOffset = index_new<WHITE, BLACK>(refreshPc, refreshSq, pc, sq);

        refreshSingle<BLACK>(this, refreshOffset, refreshSq);

        addSubSubSingle<ON_COLOR, OFF_COLOR, CAP_COLOR>(sq, onOffset, offOffset, capOffset);

        memcpy(&t[sq * MINI_ACC_SIZE * 2], &accumulator[sq * MINI_ACC_SIZE * 2], MINI_ACC_SIZE * 2 * sizeof(int16_t));
    }

    memcpy(&t[refreshSq * MINI_ACC_SIZE * 2], &accumulator[refreshSq * MINI_ACC_SIZE * 2], MINI_ACC_SIZE * 2 * sizeof(int16_t));

    accumulatorStack.incSize();
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

        addaddSubSubSingle<C>(sq, add1Offset, add2Offset, sub1Offset, sub2Offset);
    }
}

// https://stackoverflow.com/a/35270026
inline int reduce_sum_avx2(__m256i v) {
    __m128i hi128 = _mm256_extracti128_si256(v, 1);
    __m128i lo128 = _mm256_castsi256_si128(v);
    __m128i sum128 = _mm_add_epi32(hi128, lo128);
    __m128i sum64 = _mm_add_epi32(sum128, _mm_unpackhi_epi64(sum128, sum128));
    __m128i sum32 = _mm_add_epi32(sum64, _mm_shufflelo_epi16(sum64, _MM_SHUFFLE(1, 0, 3, 2)));

    return _mm_cvtsi128_si32(sum32);
}

template<Color C> inline
int Net::calculate(uint64_t occupied, Piece* mailbox) {
    int output = 0;

    __m256i sum  = _mm256_setzero_ps();
    __m256i zero = _mm256_setzero_ps();
    __m256i qa   = _mm256_set1_epi16(403);

    while (occupied) {
        int sq = popLSB(occupied);

        int ourPiece   = mailbox[sq ^ (56 * (C == BLACK))];
        int theirPiece = makePiece(typeOf(ourPiece), !colorOf(ourPiece));

        if (C == BLACK) {
            int temp = ourPiece;
            ourPiece = theirPiece;
            theirPiece = temp;
        }

        constexpr int STEP  = sizeof(__m256i) / sizeof(int16_t);
        constexpr int HALF  = STEP / 2;
        constexpr int SPLIT = C == BLACK && NUM_REGS_DUAL != 0 ? NUM_REGS_PERS * STEP : -1; 

        for (int i = 0; i < MINI_ACC_SIZE * 2; i += STEP) {
            int wOffset = C == WHITE ? i : (2 * MINI_ACC_SIZE - STEP) - i;

            int n = ((sq ^ (56 * (C == BLACK))) * MINI_ACC_SIZE * 2) + i;

            __m256i acc = _mm256_load_si256((__m256i*) &accumulator[n]);
            __m256i c   = _mm256_max_epi16(_mm256_min_epi16(acc, qa), zero);

            __m256i w;

            if (i != SPLIT) {
                w = _mm256_load_si256((__m256i*) &weights1[L1_SIZE * 2 * ourPiece + (sq * MINI_ACC_SIZE * 2) + wOffset]);
            } else {
                w = _mm256_loadu2_m128i(
                    (const __m128i*)(&weights1[L1_SIZE * 2 * ourPiece + (sq * MINI_ACC_SIZE * 2) + wOffset       ]),
                    (const __m128i*)(&weights1[L1_SIZE * 2 * ourPiece + (sq * MINI_ACC_SIZE * 2) + wOffset + HALF])
                );
            }

            __m256i prod   =  _mm256_madd_epi16(c, _mm256_mullo_epi16(c, w));

            sum = _mm256_add_epi32(sum, prod);
        }
    }

    output = reduce_sum_avx2(sum);

    return ((output / 403) + bias1[0]) * 133 / (64 * 403);
}

inline void Net::refreshMiniAcc(Position& pos, Piece piece, int square) {
    uint64_t white = pos.getOccupied<WHITE>() & ~(1ULL << square);
    uint64_t black = pos.getOccupied<BLACK>() & ~(1ULL << square);

    int biasIndex = L1_SIZE * 2 * typeOf(piece) + (colorOf(piece) ? square : square ^ 56) * MINI_ACC_SIZE * 2;

    memcpy(&accumulator[square * MINI_ACC_SIZE * 2                ], &bias0[biasIndex + MINI_ACC_SIZE * !colorOf(piece)], MINI_ACC_SIZE * sizeof(int16_t));
    memcpy(&accumulator[square * MINI_ACC_SIZE * 2 + MINI_ACC_SIZE], &bias0[biasIndex + MINI_ACC_SIZE *  colorOf(piece)], MINI_ACC_SIZE * sizeof(int16_t));

    while (white) {
        int sq = popLSB(white);
        Piece pc = pos.pieceOn(sq);

        int wOffset = index_new<WHITE>(piece, square, pc, sq);

        refreshSingle<WHITE>(this, wOffset, square);
    }

    while (black) {
        int sq = popLSB(black);
        Piece pc = pos.pieceOn(sq);

        int wOffset = index_new<WHITE>(piece, square, pc, sq);

        refreshSingle<BLACK>(this, wOffset, square);
    }
}

inline void
 Net::pushAccToStack(uint64_t occupied) {
    auto& t = accumulatorStack.at(accumulatorStack.getSize());

    while (occupied) {
        int sq = popLSB(occupied);

        memcpy(&t[sq * MINI_ACC_SIZE * 2], &accumulator[sq * MINI_ACC_SIZE * 2], MINI_ACC_SIZE * 2 * sizeof(int16_t));
    }
    
    accumulatorStack.incSize();
}

inline void Net::popAccStack() {
    accumulatorStack.pop();
    accumulator = accumulatorStack.top();
}


#endif //MOLYBDENUM_NNUE_H
