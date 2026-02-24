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
static const int MINI_ACC_SIZE = 8;
static const int L1_SIZE = MINI_ACC_SIZE * 64;
static const int L2_SIZE = 8;
static const int L3_SIZE = 16;
static const int OUTPUT_SIZE = 1;

struct Weights {
    std::array<float, L1_SIZE * INPUT_SIZE * 12> weights0{};
    std::array<float, L1_SIZE * 12> bias0{};
    std::array<float, L1_SIZE * L2_SIZE * 2 * 12> weights1{};
    std::array<float, L2_SIZE> bias1{};
    std::array<float, L2_SIZE * L3_SIZE> weights2;
    std::array<float, L3_SIZE> bias2;
    std::array<float, L3_SIZE * OUTPUT_SIZE> weights3;
    std::array<float, OUTPUT_SIZE> bias3;
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
    std::array<int16_t, L1_SIZE * L2_SIZE * 2 * 12> weights1{};
    std::array<int, L2_SIZE> bias1{};
    std::array<float, L2_SIZE * L3_SIZE> weights2;
    std::array<float, L3_SIZE> bias2;
    std::array<float, L3_SIZE * OUTPUT_SIZE> weights3;
    std::array<float, OUTPUT_SIZE> bias3;
    std::array<int16_t, L1_SIZE * 2> accumulator{};
    std::array<int16_t, L1_SIZE * 3 * 2> wdlWeights{};
    std::array<int16_t, 3> wdlBias{};
    Stack<std::array<int16_t, L1_SIZE * 2>, MAXDEPTH> accumulatorStack;

    void initAccumulator(Position &pos);
    template<Color C>
    int calculate(uint64_t occupied, Piece* mailbox);
    std::tuple<float, float, float> getWDL(Color c);
    void loadDefaultNet();

    template<Color C> inline
    void loadWeightsVec(__m256i& w, int offset);

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
    int clamped = std::clamp(input, int16_t(0), int16_t(255));
    return clamped * clamped;
}

inline float screlu(float input) {
    float clamped = std::clamp(input, 0.0f, 1.0f);
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
void Net::loadWeightsVec(__m256i& w, int offset) {
    if constexpr (C) {
        w = _mm256_loadu_si256((const __m256i *)(&weights0[0] + offset));
    } else {
        w = _mm256_loadu2_m128i(
            (const __m128i*)(&weights0[offset - MINI_ACC_SIZE]),
            (const __m128i*)(&weights0[offset                ])
        );
    }
}

template<Color C> 
inline void refreshSingle(Net* net, int offset, int bsq) {
    int idx = bsq * MINI_ACC_SIZE * 2;

    __m256i w;
   
    net->loadWeightsVec<C>(w, offset);

    __m256i acc = _mm256_loadu_si256((__m256i *)(&net->accumulator[0] + idx));

    acc = _mm256_add_epi16(acc, w);

    _mm256_storeu_si256((__m256i *)(&net->accumulator[0] + idx), acc);  
}

template<Color ON_COLOR, Color OFF_COLOR>
inline void Net::addSubSingle(int sq, int onOffset, int offOffset) {
    int idx = sq * MINI_ACC_SIZE * 2;

    __m256i wA, wS;

    loadWeightsVec< ON_COLOR>(wA, onOffset);
    loadWeightsVec<OFF_COLOR>(wS, offOffset);

    __m256i acc = _mm256_loadu_si256((__m256i *)(&accumulator[0] + idx));

    acc = _mm256_add_epi16(acc, wA);
    acc = _mm256_sub_epi16(acc, wS);

    _mm256_storeu_si256((__m256i *)(&accumulator[0] + idx), acc);
}

template<Color ON_COLOR, Color OFF_COLOR, Color CAP_COLOR>
inline void Net::addSubSubSingle(int sq, int onOffset, int offOffset, int capOffset) {
    int idx = sq * MINI_ACC_SIZE * 2;

    __m256i wA, wS1, wS2;

    loadWeightsVec<ON_COLOR >(wA,  onOffset);
    loadWeightsVec<OFF_COLOR>(wS1, offOffset);
    loadWeightsVec<CAP_COLOR>(wS2, capOffset);

    __m256i acc = _mm256_loadu_si256((__m256i *)(&accumulator[0] + idx));

    acc = _mm256_add_epi16(acc, wA );
    acc = _mm256_sub_epi16(acc, wS1);
    acc = _mm256_sub_epi16(acc, wS2);

    _mm256_storeu_si256((__m256i *)(&accumulator[0] + idx), acc);
}

template<Color C>
inline void Net::addaddSubSubSingle(int sq, int add1Offset, int add2Offset, int sub1Offset, int sub2Offset) {
    int idx = sq * MINI_ACC_SIZE * 2;

    __m256i wA1, wA2, wS1, wS2;

    __m256i acc = _mm256_loadu_si256((__m256i *)(&accumulator[0] + idx));

    loadWeightsVec<C>(wA1, add1Offset);
    loadWeightsVec<C>(wA2, add2Offset);

    acc = _mm256_add_epi16(acc, wA1);
    acc = _mm256_add_epi16(acc, wA2);

    loadWeightsVec<C>(wS1, sub1Offset);
    loadWeightsVec<C>(wS2, sub2Offset);

    acc = _mm256_sub_epi16(acc, wS1);
    acc = _mm256_sub_epi16(acc, wS2);

    _mm256_storeu_si256((__m256i *)(&accumulator[0] + idx), acc);
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

static inline int32_t hsum_8_epi32(__m256i v) {
    __m128i low = _mm256_castsi256_si128(v);
    __m128i high = _mm256_extracti128_si256(v, 1);
    __m128i sum = _mm_add_epi32(low, high);
    sum = _mm_add_epi32(sum, _mm_shuffle_epi32(sum, _MM_SHUFFLE(1, 0, 3, 2)));
    sum = _mm_add_epi32(sum, _mm_shuffle_epi32(sum, _MM_SHUFFLE(0, 0, 0, 1)));
    return _mm_cvtsi128_si32(sum);
}

template<Color C> inline
int Net::calculate(uint64_t occupied, Piece* mailbox) {
    std::array<int, L2_SIZE> output1 = bias1;
    std::array<float, L3_SIZE> output2 = bias2;
    float out = bias3[0];

    int temp[MINI_ACC_SIZE * 2];

    while (occupied) {
        int sq = popLSB(occupied);

        int ourPiece   = mailbox[sq ^ (56 * (C == BLACK))];
        int theirPiece = makePiece(typeOf(ourPiece), !colorOf(ourPiece));

        if (C == BLACK) {
            int temp = ourPiece;
            ourPiece = theirPiece;
            theirPiece = temp;
        }

        for (int i = 0; i < MINI_ACC_SIZE; i++) {
            int nUs   = ((sq ^ (56 * (C == BLACK))) * MINI_ACC_SIZE * 2) + (MINI_ACC_SIZE * (C == BLACK)) + i;
            int nThem = ((sq ^ (56 * (C == BLACK))) * MINI_ACC_SIZE * 2) + (MINI_ACC_SIZE * (C == WHITE)) + i;

            temp[i                ] = screlu(accumulator[nUs]);
            temp[i + MINI_ACC_SIZE] = screlu(accumulator[nThem]);
        }

        for (int m = 0; m < L2_SIZE; m++) {
            __m256i sum_v = _mm256_setzero_si256();
            
            const int16_t* w_row_a = &weights1[MINI_ACC_SIZE * (64 * L2_SIZE * 2 * ourPiece + (sq * L2_SIZE * 2) + m)];
            const int16_t* w_row_b = &weights1[MINI_ACC_SIZE * (64 * L2_SIZE * 2 * ourPiece + (sq * L2_SIZE * 2) + m + MINI_ACC_SIZE)];

            for (int n = 0; n < MINI_ACC_SIZE; n += 8) {
                __m256i l0_a = _mm256_load_si256((const __m256i*)&temp[n]);
                __m256i l0_b = _mm256_load_si256((const __m256i*)&temp[n + MINI_ACC_SIZE]);

                __m256i w_a = _mm256_cvtepi16_epi32(_mm_load_si128((const __m128i*)&w_row_a[n]));
                __m256i w_b = _mm256_cvtepi16_epi32(_mm_load_si128((const __m128i*)&w_row_b[n]));

                __m256i prod_a = _mm256_mullo_epi32(l0_a, w_a);
                __m256i prod_b = _mm256_mullo_epi32(l0_b, w_b);

                sum_v = _mm256_add_epi32(sum_v, _mm256_add_epi32(prod_a, prod_b));
            }

            output1[m] += hsum_8_epi32(sum_v);
        }
    }

    for (int n = 0; n < L2_SIZE; n++)
        for (int m = 0; m < L3_SIZE; m++)
            output2[m] += std::max(float(output1[n]) / float(255 * 255 * 64), 0.0f) * weights2[n * L3_SIZE + m];

    for (int i = 0; i < L3_SIZE; i++)
        out += screlu(output2[i]) * weights3[i];

    return int(out * 133);
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
