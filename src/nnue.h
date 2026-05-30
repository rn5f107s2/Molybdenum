#ifndef MOLYBDENUM_NNUE_H
#define MOLYBDENUM_NNUE_H

#include "Constants.h"
#include <array>
#include <string>
#include <tuple>
#include <algorithm>
#include <immintrin.h>
#include <type_traits>
#include "BitStuff.h"
#include "Utility.h"
#include "Position.h"
#include "simd.h"

enum Toggle {
    Off, On
};

static constexpr int INPUT_SIZE = 12 * 64;
static constexpr int MINI_ACC_SIZE = 32;
static constexpr int L1_SIZE = MINI_ACC_SIZE * 64;
static constexpr int L2_SIZE = 8;
static constexpr int L3_SIZE = 32;
static constexpr int OUTPUT_SIZE = 1;

static constexpr int I16_PER_REG = (sizeof(vec_t) / sizeof(int16_t));

static constexpr int NUM_REGS    = (MINI_ACC_SIZE * 2) / I16_PER_REG;

static constexpr int NUM_REGS_PERS = MINI_ACC_SIZE / I16_PER_REG;
static constexpr int NUM_REGS_DUAL = 2 * NUM_REGS_PERS == NUM_REGS? 0 : 1;

static_assert(NUM_REGS == NUM_REGS_PERS * 2 + NUM_REGS_DUAL);
static_assert((MINI_ACC_SIZE * 2) % I16_PER_REG == 0);

template<bool PREPROCESSED>
struct NetWeights {
    using FT_W = std::conditional_t<PREPROCESSED, int8_t, int16_t>;
    using LW_T = std::conditional_t<PREPROCESSED, float , int16_t>;
    using B_T  = std::conditional_t<PREPROCESSED, int   , int16_t>;

    constexpr static int N_WEIGHTS0 = PREPROCESSED ? L1_SIZE / 2 * INPUT_SIZE * 12
                                                   : L1_SIZE * INPUT_SIZE * 12;

    std::array<int16_t, N_WEIGHTS0> weights0{};
    std::array<int16_t, L1_SIZE * 12> bias0{};
    std::array<FT_W, L1_SIZE * L2_SIZE * 2 * 12> weights1{};
    std::array<B_T , L2_SIZE * 2> bias1{};
    std::array<LW_T, L2_SIZE * 2 * L3_SIZE> weights2{};
    std::array<LW_T, L3_SIZE> bias2{};
    std::array<LW_T, L3_SIZE * OUTPUT_SIZE> weights3{};
    std::array<LW_T, OUTPUT_SIZE> bias3{};
};

using Weights = NetWeights<true>;
using RawWeights = NetWeights<false>;

struct WDLHead {
    std::array<int16_t, L1_SIZE * 3 * 2> weights1{};
    std::array<int16_t, 3> bias1{};
};

const extern Weights defaultWeights;

class Net {
public:
    const std::array<int16_t, L1_SIZE / 2 * INPUT_SIZE * 12>& weights0;
    const std::array<int16_t, L1_SIZE * 12>& bias0;
    const std::array<int8_t, L1_SIZE * L2_SIZE * 2 * 12>& weights1;
    const std::array<int, L2_SIZE * 2>& bias1;
    const std::array<float, L2_SIZE * 2 * L3_SIZE>& weights2;
    const std::array<float, L3_SIZE>& bias2;
    const std::array<float, L3_SIZE * OUTPUT_SIZE>& weights3;
    const std::array<float, OUTPUT_SIZE>& bias3;
    std::array<int16_t, L1_SIZE * 3 * 2> wdlWeights{};
    std::array<int16_t, 3> wdlBias{};

    int16_t* accumulator;
    int16_t  accumulatorStack[MAXDEPTH + 7][L1_SIZE * 2];
    int accumulatorHead = 0;

    Net() : weights0(defaultWeights.weights0), bias0(defaultWeights.bias0), weights1(defaultWeights.weights1), bias1(defaultWeights.bias1),
    weights2(defaultWeights.weights2), bias2(defaultWeights.bias2), weights3(defaultWeights.weights3), bias3(defaultWeights.bias3) {}

    void initAccumulator(Position &pos);
    template<Color C>
    int calculate(uint64_t occupied, Piece* mailbox);
    std::tuple<float, float, float> getWDL(Color c);
    void loadDefaultNet();

    int laterLayers(float* l1Out);

    template<Color C> inline
    void loadWeightsVecDual(vec_t& w, int offset, int i);

    template<Color C> inline
    void loadWeightsVec(vec_t& w, int offset, int i);

    template<Color C> inline
    void loadWeightsVecs(vec_t* w, int offset);

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

    int8_t activated[MINI_ACC_SIZE * 2];
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
    int bsq = (bpc ? bucketSq  : bucketSq  ^ 56) | ((bpc ^ fpc) << 2);
    int fsq = colorOf(featurePc) ? featureSq : featureSq ^ 56;
    int ci  = !fpc;

    return   fpt * 64 * 64 * MINI_ACC_SIZE * 2 * 6 
           + fsq * 64 * MINI_ACC_SIZE * 2 * 6 
           + bpt * 64 * MINI_ACC_SIZE * 2 
           + bsq * MINI_ACC_SIZE * 2 
           + ci * MINI_ACC_SIZE;
}

template<Color C, Color FPC> inline
int index_new(int bucketPc, int bucketSq, int featurePc, int featureSq) {
    int bpt = typeOf(bucketPc);
    int fpt = typeOf(featurePc);
    int bpc = colorOf(bucketPc);
    int bsq = (bpc ? bucketSq  : bucketSq  ^ 56) | ((bpc ^ FPC) << 2);
    int fsq = FPC ? featureSq : featureSq ^ 56;
    int ci  = !FPC;

    return     fpt * 64 * 64 * MINI_ACC_SIZE * 2 * 6
             + fsq * 64 * MINI_ACC_SIZE * 2 * 6 
             + bpt * 64 * MINI_ACC_SIZE * 2 
             + bsq * MINI_ACC_SIZE * 2
             + ci * MINI_ACC_SIZE;
}

template<Color C, Color BPC, Color FPC> inline
int index_new(int bucketPc, int bucketSq, int featurePc, int featureSq) {
    int bpt = typeOf(bucketPc);
    int fpt = typeOf(featurePc);
    int bsq = BPC ? bucketSq  : bucketSq  ^ 56;
    int fsq = FPC ? featureSq : featureSq ^ 56;
    int ci  = !FPC;
    int ci2 = ((BPC ^ FPC) << 2);


    // Color indexing maps the following way:
    // 1 1 -> 0 
    // 0 0 -> 1
    // 0 1 -> 2
    // 1 0 -> 3 
    // since we hm on bsq, the 3rd bit can never be set. Normally mapping
    // the bsq into 0...31 is slow, so instead as we only really need 1 1, 0 0 and 0 1, 1 0
    // to be next to each other, we map !FPC to the color index, but use BPC ^ FPC as the 3rd bit 
    // in the bsq since that is almost free. Since the bit is never set, setting it is equivalant to adding 4,
    // since FPC and BPC are constexpr, we can rewrite that into the index calculation without ever actually needint to set the bit.

    return    fpt * 64 * 64 * MINI_ACC_SIZE * 2 * 6 
            + fsq * 64 * MINI_ACC_SIZE * 2 * 6 
            + bpt * 64 * MINI_ACC_SIZE * 2
            + ci2 * MINI_ACC_SIZE * 2 
            + bsq * MINI_ACC_SIZE * 2
            + ci * MINI_ACC_SIZE;
}

/**
 * This doesnt work currently for networks with (atleast) one reg per pers + one dual (for black pers updates)
 * The following idea should work:
 *     - Read white pers
 *     - Read mixed
 *     - Read black pers
 * (what is currently done (for black pers updates) is 
 *  - Read mixed
 *  - Read white
 *  - Read black
 * which doesnt align with how the accumulators are loaded or the loadWeightsVec func thinks weights are loaded)
 * although it feels a little ugly
 */

template<Color C> inline
void Net::loadWeightsVecDual(vec_t& w, int offset, int i) {
    if constexpr (C) {
        w = vec_loadu((const vec_t *)(&weights0[0] + offset + I16_PER_REG * i));
    } else {
        constexpr int HALF = I16_PER_REG / 2;

        w = vec_loadu2(
            (const halfvec_t*)(&weights0[offset - I16_PER_REG * i - HALF]),
            (const halfvec_t*)(&weights0[offset - I16_PER_REG * i       ])
        );
    }
}

template<Color C> inline
void Net::loadWeightsVecs(vec_t* w, int offset) {
    int i = 0;

    for (; i < NUM_REGS_PERS; i++)
        loadWeightsVec<1>(w[i], offset, i);

    // untested
    for (; i < NUM_REGS_PERS + NUM_REGS_DUAL; i++) 
        loadWeightsVecDual<C>(w[i], offset, i);

    for (; i < NUM_REGS; i++)
        loadWeightsVec<C>(w[i], offset, i);
}

template<Color C> inline
void Net::loadWeightsVec(vec_t& w, int offset, int i) {
    if constexpr (C) {
        w = vec_loadu((const vec_t*)(&weights0[0] + offset + I16_PER_REG * i));
    } else {
        w = vec_loadu((const vec_t*)(&weights0[0] + offset + I16_PER_REG * i - I16_PER_REG * NUM_REGS_PERS - MINI_ACC_SIZE));
    }
}

template<Color C> 
inline void refreshSingle(Net* net, int offset, int bsq, int16_t* accum) {
    int idx = bsq * MINI_ACC_SIZE * 2;

    vec_t w  [NUM_REGS];
    vec_t acc[NUM_REGS];

    net->loadWeightsVecs<C>(w, offset);

    for (int i = 0; i < NUM_REGS; i++)
        acc[i] = vec_loadu((vec_t *)(accum + idx + I16_PER_REG * i));

    for (int i = 0; i < NUM_REGS; i++)
        acc[i] = vec_add_epi16(acc[i], w[i]);

    for (int i = 0; i < NUM_REGS; i++)
        vec_storeu((vec_t *)(accum + idx + I16_PER_REG * i), acc[i]);  
}

template<Color C> 
inline void refreshSingle(Net* net, int offset, vec_t* acc) {
    vec_t w  [NUM_REGS];

    net->loadWeightsVecs<C>(w, offset);

    for (int i = 0; i < NUM_REGS; i++)
        acc[i] = vec_add_epi16(acc[i], w[i]);
}

template<Color ON_COLOR, Color OFF_COLOR>
inline void Net::addSubSingle(int sq, int onOffset, int offOffset) {
    int idx = sq * MINI_ACC_SIZE * 2;

    vec_t wA[NUM_REGS];
    vec_t wS[NUM_REGS];
    vec_t acc[NUM_REGS];

    loadWeightsVecs< ON_COLOR>(wA,  onOffset);
    loadWeightsVecs<OFF_COLOR>(wS, offOffset);

    for (int i = 0; i < NUM_REGS; i++)
        acc[i] = vec_loadu((vec_t *)(accumulator + idx + I16_PER_REG * i));

    for (int i = 0; i < NUM_REGS; i++) {
        acc[i] = vec_add_epi16(acc[i], wA[i]);
        acc[i] = vec_sub_epi16(acc[i], wS[i]);
    }

    for (int i = 0; i < NUM_REGS; i++)
        vec_storeu((vec_t *)(accumulatorStack[accumulatorHead + 1] + idx + I16_PER_REG * i), acc[i]);
}

template<Color ON_COLOR, Color OFF_COLOR, Color CAP_COLOR>
inline void Net::addSubSubSingle(int sq, int onOffset, int offOffset, int capOffset) {
    int idx = sq * MINI_ACC_SIZE * 2;

    vec_t wA [NUM_REGS]; 
    vec_t wS1[NUM_REGS];
    vec_t wS2[NUM_REGS];
    vec_t acc[NUM_REGS];

    loadWeightsVecs<ON_COLOR >(wA , onOffset );
    loadWeightsVecs<OFF_COLOR>(wS1, offOffset);
    loadWeightsVecs<CAP_COLOR>(wS2, capOffset);

    for (int i = 0; i < NUM_REGS; i++)
        acc[i] = vec_loadu((vec_t *)(accumulator + idx + I16_PER_REG * i));

    for (int i = 0; i < NUM_REGS; i++) {
        acc[i] = vec_add_epi16(acc[i], wA [i]);
        acc[i] = vec_sub_epi16(acc[i], wS1[i]);
        acc[i] = vec_sub_epi16(acc[i], wS2[i]);
    }

    for (int i = 0; i < NUM_REGS; i++)
        vec_storeu((vec_t *)(accumulatorStack[accumulatorHead + 1] + idx + I16_PER_REG * i), acc[i]);
}

template<Color C>
inline void Net::addaddSubSubSingle(int sq, int add1Offset, int add2Offset, int sub1Offset, int sub2Offset) {
    int idx = sq * MINI_ACC_SIZE * 2;

    vec_t wA1[NUM_REGS];
    vec_t wA2[NUM_REGS];
    vec_t wS1[NUM_REGS];
    vec_t wS2[NUM_REGS];
    vec_t acc[NUM_REGS];

    for (int i = 0; i < NUM_REGS; i++)
        acc[i] = vec_loadu((vec_t *)(accumulator + idx + I16_PER_REG * i));

    loadWeightsVecs<C>(wA1, add1Offset);
    loadWeightsVecs<C>(wA2, add2Offset);

    for (int i = 0; i < NUM_REGS; i++) {
        acc[i] = vec_add_epi16(acc[i], wA1[i]);
        acc[i] = vec_add_epi16(acc[i], wA2[i]);
    }

    loadWeightsVecs<C>(wS1, sub1Offset);
    loadWeightsVecs<C>(wS2, sub2Offset);

    for (int i = 0; i < NUM_REGS; i++) {
        acc[i] = vec_sub_epi16(acc[i], wS1[i]);
        acc[i] = vec_sub_epi16(acc[i], wS2[i]);
    }

    for (int i = 0; i < NUM_REGS; i++)
        vec_storeu((vec_t *)(accumulatorStack[accumulatorHead + 1] + idx + I16_PER_REG * i), acc[i]);
}

template<Color ON_COLOR, Color OFF_COLOR>
inline void Net::addSub(Position& pos, uint64_t cleanBitboard, uint64_t white, int onPiece, int onSquare, int offPiece, int offSquare, int refreshPc, int refreshSq) {
    uint64_t wK = cleanBitboard & white & KINGSIDE;
    uint64_t wQ = cleanBitboard & white & QUEENSIDE;
    uint64_t bK = cleanBitboard & ~white & KINGSIDE;
    uint64_t bQ = cleanBitboard & ~white & QUEENSIDE;

    constexpr bool RPC = ON_COLOR; // colorOf(refreshPc) == colorOf(movedPiece) == ON_COLOR

    int rFlip = (refreshSq & 4) ? 7 : 0; 

    int biasIndex = L1_SIZE * 2 * typeOf(refreshPc) + (RPC ? refreshSq ^ rFlip : refreshSq ^ 56 ^ rFlip) * MINI_ACC_SIZE * 2;

    int16_t* t = accumulatorStack[accumulatorHead + 1];

    memcpy(&t[refreshSq * MINI_ACC_SIZE * 2                ], &bias0[biasIndex + MINI_ACC_SIZE * !RPC], MINI_ACC_SIZE * sizeof(int16_t));
    memcpy(&t[refreshSq * MINI_ACC_SIZE * 2 + MINI_ACC_SIZE], &bias0[biasIndex + MINI_ACC_SIZE *  RPC], MINI_ACC_SIZE * sizeof(int16_t));

    int idx = refreshSq * MINI_ACC_SIZE * 2;

    vec_t refreshAccs[NUM_REGS];

    for (int i = 0; i < NUM_REGS; i++)
        refreshAccs[i] = vec_loadu((vec_t *)(t + idx + I16_PER_REG * i));

    while (wK) {
        int sq   = popLSB(wK);
        Piece pc = pos.pieceOn(sq);

        int flip = 0;

        int  onOffset     = index_new<WHITE, WHITE, ON_COLOR >(pc, sq ^ flip,  onPiece, onSquare  ^ flip);
        int offOffset     = index_new<WHITE, WHITE, OFF_COLOR>(pc, sq ^ flip, offPiece, offSquare ^ flip);
        int refreshOffset = index_new<WHITE, RPC, WHITE>(refreshPc, refreshSq ^ rFlip, pc, sq ^ rFlip);

        refreshSingle<WHITE>(this, refreshOffset, refreshAccs);

        addSubSingle<ON_COLOR, OFF_COLOR>(sq, onOffset, offOffset);
    }

    while (wQ) {
        int sq   = popLSB(wQ);
        Piece pc = pos.pieceOn(sq);

        int flip = 7;

        int  onOffset     = index_new<WHITE, WHITE, ON_COLOR >(pc, sq ^ flip,  onPiece, onSquare  ^ flip);
        int offOffset     = index_new<WHITE, WHITE, OFF_COLOR>(pc, sq ^ flip, offPiece, offSquare ^ flip);
        int refreshOffset = index_new<WHITE, RPC, WHITE>(refreshPc, refreshSq ^ rFlip, pc, sq ^ rFlip);

        refreshSingle<WHITE>(this, refreshOffset, refreshAccs);

        addSubSingle<ON_COLOR, OFF_COLOR>(sq, onOffset, offOffset);
    }

    while (bK) {
        int sq   = popLSB(bK);
        Piece pc = pos.pieceOn(sq);

        int flip = 0;

        int  onOffset     = index_new<WHITE, BLACK, ON_COLOR >(pc, sq ^ flip,  onPiece, onSquare ^ flip);
        int offOffset     = index_new<WHITE, BLACK, OFF_COLOR>(pc, sq ^ flip, offPiece, offSquare ^ flip);
        int refreshOffset = index_new<WHITE, RPC, BLACK>(refreshPc, refreshSq ^ rFlip, pc, sq ^ rFlip);

        refreshSingle<BLACK>(this, refreshOffset, refreshAccs);

        addSubSingle<ON_COLOR, OFF_COLOR>(sq, onOffset, offOffset);
    }

    while (bQ) {
        int sq   = popLSB(bQ);
        Piece pc = pos.pieceOn(sq);

        int flip = 7;

        int  onOffset     = index_new<WHITE, BLACK, ON_COLOR >(pc, sq ^ flip,  onPiece, onSquare ^ flip);
        int offOffset     = index_new<WHITE, BLACK, OFF_COLOR>(pc, sq ^ flip, offPiece, offSquare ^ flip);
        int refreshOffset = index_new<WHITE, RPC, BLACK>(refreshPc, refreshSq ^ rFlip, pc, sq ^ rFlip);

        refreshSingle<BLACK>(this, refreshOffset, refreshAccs);

        addSubSingle<ON_COLOR, OFF_COLOR>(sq, onOffset, offOffset);
    }

    for (int i = 0; i < NUM_REGS; i++)
        vec_storeu((vec_t *)(t + idx + I16_PER_REG * i), refreshAccs[i]);  

    accumulator = accumulatorStack[++accumulatorHead];
}

template<Color ON_COLOR, Color OFF_COLOR, Color CAP_COLOR>
inline void Net::addSubSub(Position& pos, uint64_t cleanBitboard, uint64_t white, int onPiece, int onSquare, int offPiece, int offSquare, int capPiece, int capSq, int refreshPc, int refreshSq) {
    uint64_t wK = cleanBitboard & white & KINGSIDE;
    uint64_t wQ = cleanBitboard & white & QUEENSIDE;
    uint64_t bK = cleanBitboard & ~white & KINGSIDE;
    uint64_t bQ = cleanBitboard & ~white & QUEENSIDE;

    constexpr bool RPC = ON_COLOR; // colorOf(refreshPc) == colorOf(movedPiece) == ON_COLOR

    int rFlip = (refreshSq & 4) ? 7 : 0; 

    int biasIndex = L1_SIZE * 2 * typeOf(refreshPc) + (RPC ? refreshSq ^ rFlip : refreshSq ^ 56 ^ rFlip) * MINI_ACC_SIZE * 2;

    int16_t* t = accumulatorStack[accumulatorHead + 1];

    memcpy(&t[refreshSq * MINI_ACC_SIZE * 2                ], &bias0[biasIndex + MINI_ACC_SIZE * !RPC], MINI_ACC_SIZE * sizeof(int16_t));
    memcpy(&t[refreshSq * MINI_ACC_SIZE * 2 + MINI_ACC_SIZE], &bias0[biasIndex + MINI_ACC_SIZE *  RPC], MINI_ACC_SIZE * sizeof(int16_t));

    int idx = refreshSq * MINI_ACC_SIZE * 2;


     //  Last time checking (at num regs == 4) this function used 12 regs in total, 
     //  this may be a slowdown if num regs grows beyond that.

    vec_t refreshAccs[NUM_REGS];

    for (int i = 0; i < NUM_REGS; i++)
        refreshAccs[i] = vec_loadu((vec_t *)(t + idx + I16_PER_REG * i));

    while (wK) {
        int sq   = popLSB(wK);
        Piece pc = pos.pieceOn(sq);

        int flip = 0;

        int  onOffset     = index_new<WHITE, WHITE, ON_COLOR >(pc, sq ^ flip,  onPiece,  onSquare ^ flip);
        int offOffset     = index_new<WHITE, WHITE, OFF_COLOR>(pc, sq ^ flip, offPiece, offSquare ^ flip);
        int capOffset     = index_new<WHITE, WHITE, CAP_COLOR>(pc, sq ^ flip, capPiece,     capSq ^ flip);
        int refreshOffset = index_new<WHITE, RPC, WHITE>(refreshPc, refreshSq ^ rFlip, pc, sq ^ rFlip);

        refreshSingle<WHITE>(this, refreshOffset, refreshAccs);
    
        addSubSubSingle<ON_COLOR, OFF_COLOR, CAP_COLOR>(sq, onOffset, offOffset, capOffset);
    }

    while (wQ) {
        int sq   = popLSB(wQ);
        Piece pc = pos.pieceOn(sq);

        int flip = 7;

        int  onOffset     = index_new<WHITE, WHITE, ON_COLOR >(pc, sq ^ flip,  onPiece,  onSquare ^ flip);
        int offOffset     = index_new<WHITE, WHITE, OFF_COLOR>(pc, sq ^ flip, offPiece, offSquare ^ flip);
        int capOffset     = index_new<WHITE, WHITE, CAP_COLOR>(pc, sq ^ flip, capPiece,     capSq ^ flip);
        int refreshOffset = index_new<WHITE, RPC, WHITE>(refreshPc, refreshSq ^ rFlip, pc, sq ^ rFlip);

        refreshSingle<WHITE>(this, refreshOffset, refreshAccs);
    
        addSubSubSingle<ON_COLOR, OFF_COLOR, CAP_COLOR>(sq, onOffset, offOffset, capOffset);
    }

    while (bK) {
        int sq   = popLSB(bK);
        Piece pc = pos.pieceOn(sq);

        int flip = 0;

        int  onOffset = index_new<WHITE, BLACK, ON_COLOR >(pc, sq ^ flip,  onPiece,  onSquare ^ flip);
        int offOffset = index_new<WHITE, BLACK, OFF_COLOR>(pc, sq ^ flip, offPiece, offSquare ^ flip);
        int capOffset = index_new<WHITE, BLACK, CAP_COLOR>(pc, sq ^ flip, capPiece,     capSq ^ flip);
        int refreshOffset = index_new<WHITE, RPC, BLACK>(refreshPc, refreshSq ^ rFlip, pc, sq ^ rFlip);

        refreshSingle<BLACK>(this, refreshOffset, refreshAccs);

        addSubSubSingle<ON_COLOR, OFF_COLOR, CAP_COLOR>(sq, onOffset, offOffset, capOffset);
    }

    while (bQ) {
        int sq   = popLSB(bQ);
        Piece pc = pos.pieceOn(sq);

        int flip = 7;

        int  onOffset = index_new<WHITE, BLACK, ON_COLOR >(pc, sq ^ flip,  onPiece,  onSquare ^ flip);
        int offOffset = index_new<WHITE, BLACK, OFF_COLOR>(pc, sq ^ flip, offPiece, offSquare ^ flip);
        int capOffset = index_new<WHITE, BLACK, CAP_COLOR>(pc, sq ^ flip, capPiece,     capSq ^ flip);
        int refreshOffset = index_new<WHITE, RPC, BLACK>(refreshPc, refreshSq ^ rFlip, pc, sq ^ rFlip);

        refreshSingle<BLACK>(this, refreshOffset, refreshAccs);

        addSubSubSingle<ON_COLOR, OFF_COLOR, CAP_COLOR>(sq, onOffset, offOffset, capOffset);
    }

    for (int i = 0; i < NUM_REGS; i++)
        vec_storeu((vec_t *)(t + idx + I16_PER_REG * i), refreshAccs[i]);  

    accumulator = accumulatorStack[++accumulatorHead];
}

template<Color C>
inline void Net::addaddSubSub(Position& pos, uint64_t cleanBitboard, int from, int to, int rFrom, int rTo, int rook, int king) {
    while (cleanBitboard) {
        int sq   = popLSB(cleanBitboard);
        Piece pc = pos.pieceOn(sq);

        int flip = (sq & 4) ? 7 : 0;

        int add1Offset = index_new<WHITE>(pc, sq ^ flip, king, to ^ flip);
        int add2Offset = index_new<WHITE>(pc, sq ^ flip, rook, rTo ^ flip);
        int sub1Offset = index_new<WHITE>(pc, sq ^ flip, king, from ^ flip);
        int sub2Offset = index_new<WHITE>(pc, sq ^ flip, rook, rFrom ^ flip);

        addaddSubSubSingle<C>(sq, add1Offset, add2Offset, sub1Offset, sub2Offset);
    }

    accumulator = accumulatorStack[++accumulatorHead];

    refreshMiniAcc(pos, Piece(king), to);
    refreshMiniAcc(pos, Piece(rook), rTo);
}

template<Color C> inline
int Net::calculate(uint64_t occupied, Piece* mailbox) {
    alignas(32) float l1Out[L2_SIZE * 2];

    constexpr int I32_PER_REG = I16_PER_REG / 2;

    vec_t sums[L2_SIZE * 2 / I32_PER_REG];

    for (int i = 0; i < L2_SIZE * 2; i += I32_PER_REG)
        sums[i / I32_PER_REG] = vec_loadu((vec_t*) &bias1[i]);

    while (occupied) {
        int sq = popLSB(occupied);

        int ourPiece   = mailbox[sq ^ (56 * (C == BLACK))];
        int theirPiece = makePiece(typeOf(ourPiece), !colorOf(ourPiece));

        if (C == BLACK) {
            int temp = ourPiece;
            ourPiece = theirPiece;
            theirPiece = temp;
        }

        int nSTM = ((sq ^ (56 * (C == BLACK))) * MINI_ACC_SIZE * 2) + (C == BLACK ? MINI_ACC_SIZE : 0);
        int nNTM = ((sq ^ (56 * (C == BLACK))) * MINI_ACC_SIZE * 2) + (C == WHITE ? MINI_ACC_SIZE : 0);

        vec_t zero = vec_setzero();
        vec_t qa   = vet_set1_epi16(255);

        for (int i = 0; i < MINI_ACC_SIZE; i += I16_PER_REG * 2) {
            vec_t us1   = vec_loadu((vec_t*) &accumulator[nSTM               + i]);
            vec_t us2   = vec_loadu((vec_t*) &accumulator[nSTM + I16_PER_REG + i]);
            vec_t them1 = vec_loadu((vec_t*) &accumulator[nNTM               + i]);
            vec_t them2 = vec_loadu((vec_t*) &accumulator[nNTM + I16_PER_REG + i]);

            vec_t cUs1   = vec_min_epi16(vec_max_epi16(us1  , zero), qa);
            vec_t cUs2   = vec_min_epi16(vec_max_epi16(us2  , zero), qa);
            vec_t cThem1 = vec_min_epi16(vec_max_epi16(them1, zero), qa);
            vec_t cThem2 = vec_min_epi16(vec_max_epi16(them2, zero), qa);

            // (cUs   * (cUs   >> 1)) >> 8;
            vec_t actUs1   = _mm256_mulhi_epi16(cUs1, _mm256_slli_epi16(cUs1, 7));
            vec_t actUs2   = _mm256_mulhi_epi16(cUs2, _mm256_slli_epi16(cUs2, 7));
            vec_t actThem1 = _mm256_mulhi_epi16(cThem1, _mm256_slli_epi16(cThem1, 7));
            vec_t actThem2 = _mm256_mulhi_epi16(cThem2, _mm256_slli_epi16(cThem2, 7));

            vec_t us   = _mm256_packus_epi16(actUs1  , actUs2);
            vec_t them = _mm256_packus_epi16(actThem1, actThem2);

            vec_storeu((vec_t*) &activated[i                ], us);
            vec_storeu((vec_t*) &activated[i + MINI_ACC_SIZE], them);
        }

        int32_t* inputsU = (int32_t*) &activated[0];
        int32_t* inputsT = (int32_t*) &activated[MINI_ACC_SIZE];

        for (int i = 0; i < MINI_ACC_SIZE; i += 4) {
            int32_t wUs = L1_SIZE * 2 * ourPiece + (sq * MINI_ACC_SIZE * 2) + (i / 4) * 4;

            vec_t inUs   = _mm256_set1_epi32(inputsU[i / 4]);
            vec_t inThem = _mm256_set1_epi32(inputsT[i / 4]);

            for (int m = 0; m < L2_SIZE; m += I32_PER_REG) {
                vec_t wU = vec_loadu((vec_t*) &weights1[ wUs                  * L2_SIZE + 4 * m]);
                vec_t wT = vec_loadu((vec_t*) &weights1[(wUs + MINI_ACC_SIZE) * L2_SIZE + 4 * m]);

                dpbusd(sums[m / I32_PER_REG                        ], inUs  , wU);
                dpbusd(sums[m / I32_PER_REG + L2_SIZE / I32_PER_REG], inThem, wT);
            }
        }
    }

    constexpr float DEQUANT_MUL = 256.0f * 2.0f / 255.0f / 255.0f / 193.0f;

    __m256 dequant = _mm256_set1_ps(DEQUANT_MUL);
    __m256 zero    = _mm256_set1_ps(0);

    for (int i = 0; i < L2_SIZE * 2; i += I32_PER_REG) {
        __m256 val = _mm256_mul_ps(_mm256_cvtepi32_ps(sums[i / I32_PER_REG]), dequant);
        __m256 act = _mm256_max_ps(val, zero);

        _mm256_store_ps(&l1Out[i], act);
    }

    return laterLayers(l1Out);
}


inline int Net::laterLayers(float* l1Out) {
    constexpr int F32_PER_REG = sizeof(vec_t) / sizeof(float);

    float l2Out[L3_SIZE];
    float output = bias3[0];

    __m256 zero = _mm256_setzero_ps();

    __m256 damsk[L3_SIZE / F32_PER_REG];

    for (int i = 0; i < L3_SIZE; i += F32_PER_REG)
        damsk[i / F32_PER_REG] = _mm256_loadu_ps(&bias2[i]);

    for (int n = 0; n < L2_SIZE * 2; n++) {
        __m256 in = _mm256_set1_ps(l1Out[n]);

        for (int m = 0; m < L3_SIZE; m += F32_PER_REG) {
            __m256 w = _mm256_loadu_ps(&weights2[n * L3_SIZE + m]);
            damsk[m / F32_PER_REG] = _mm256_fmadd_ps(in, w, damsk[m / F32_PER_REG]);
        }
    }

    __m256 one = _mm256_set1_ps(1.0f);

    for (int i = 0; i < L3_SIZE; i += F32_PER_REG) {
        __m256 c = _mm256_min_ps(_mm256_max_ps(damsk[i / F32_PER_REG], zero), one);

        _mm256_storeu_ps(&l2Out[i], _mm256_mul_ps(c, c));
    }

    __m256 acc = _mm256_setzero_ps();

    for (int i = 0; i < L3_SIZE; i += F32_PER_REG) {
        __m256 l2 = _mm256_loadu_ps(&l2Out[i]);
        __m256 w  = _mm256_loadu_ps(&weights3[i]);
        
        acc = _mm256_fmadd_ps(l2, w, acc);
    }

    __m128 hi = _mm256_extractf128_ps(acc, 1);
    __m128 lo = _mm256_castps256_ps128(acc);

    __m128 sum = _mm_add_ps(hi, lo);

    sum = _mm_hadd_ps(sum, sum);
    sum = _mm_hadd_ps(sum, sum);

    output += _mm_cvtss_f32(sum);

    return output * 133;
}

inline void Net::refreshMiniAcc(Position& pos, Piece piece, int square) {
    uint64_t white = pos.getOccupied<WHITE>() & ~(1ULL << square);
    uint64_t black = pos.getOccupied<BLACK>() & ~(1ULL << square);

    int flip = 0;
    
    if (square & 4)
        flip = 7;

    int biasIndex = L1_SIZE * 2 * typeOf(piece) + (colorOf(piece) ? square ^ flip : square ^ 56 ^ flip) * MINI_ACC_SIZE * 2;

    memcpy(&accumulator[square * MINI_ACC_SIZE * 2                ], &bias0[biasIndex + MINI_ACC_SIZE * !colorOf(piece)], MINI_ACC_SIZE * sizeof(int16_t));
    memcpy(&accumulator[square * MINI_ACC_SIZE * 2 + MINI_ACC_SIZE], &bias0[biasIndex + MINI_ACC_SIZE *  colorOf(piece)], MINI_ACC_SIZE * sizeof(int16_t));

    while (white) {
        int sq = popLSB(white);
        Piece pc = pos.pieceOn(sq);

        int wOffset = index_new<WHITE>(piece, square ^ flip, pc, sq ^ flip);

        refreshSingle<WHITE>(this, wOffset, square, accumulator);
    }

    while (black) {
        int sq = popLSB(black);
        Piece pc = pos.pieceOn(sq);

        int wOffset = index_new<WHITE>(piece, square ^ flip, pc, sq ^ flip);

        refreshSingle<BLACK>(this, wOffset, square, accumulator);
    }
}

inline void Net::pushAccToStack(uint64_t occupied) {
    accumulator = accumulatorStack[++accumulatorHead];
}

inline void Net::popAccStack() {
    accumulator = accumulatorStack[--accumulatorHead];
}


#endif //MOLYBDENUM_NNUE_H
