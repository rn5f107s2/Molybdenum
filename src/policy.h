#ifndef MOLYBDENUM_POLICY_H
#define MOLYBDENUM_POLICY_H

#include <array>
#include <cstdint>
#include <vector>
#include <sstream>

#include "Move.h"
#include "convolution.h"
#include "batchnorm.h"

const int FILTER = 64;
const int BLOCKS = 4;

const int IN_SIZE  = 832;
const int HIDDEN_SIZE = 256;
const int OUT_SIZE    = 64 * 64;

inline int moveToLayer(int from, int to) {
    const int V_POS_OFFSET   = -1;                //  0  1  2  3  4  5  6
    const int V_NEG_OFFSET   = V_POS_OFFSET  + 7; //  7  8  9 10 11 12 13
    const int H_POS_OFFSET   = V_NEG_OFFSET  + 7; // 14 15 16 17 18 19 20
    const int H_NEG_OFFSET   = H_POS_OFFSET  + 7; // 21 22 23 24 25 26 27
    const int D1_POS_OFFSET  = H_NEG_OFFSET  + 7; // 28 29 30 31 32 33 34 
    const int D1_NEG_OFFSET  = D1_POS_OFFSET + 7; // 35 36 37 38 39 40 41
    const int D2_POS_OFFSET  = D1_NEG_OFFSET + 7; // 42 43 44 45 46 47 48 
    const int D2_NEG_OFFSET  = D2_POS_OFFSET + 7; // 49 50 51 52 53 54 55
    const int KNIGHT_OFFSET1 = 56;
    const int KNIGHT_OFFSET2 = 58;
    const int KNIGHT_OFFSET3 = 60;
    const int KNIGHT_OFFSET4 = 62;

    int fromFile =  from & 0b000111;
    int fromRank = (from & 0b111000) >> 3;
    int toFile   =    to & 0b000111;
    int toRank   = (  to & 0b111000) >> 3;

    if (fromFile == toFile)
        return fromRank > toRank ? (fromRank - toRank + V_POS_OFFSET) : (toRank - fromRank + V_NEG_OFFSET);

    if (fromRank == toRank)
        return fromFile > toFile ? (fromFile - toFile + H_POS_OFFSET) : (toFile - fromFile + H_NEG_OFFSET);

    if (fromRank + fromFile == toRank + toFile)
        return fromFile > toFile ? (fromFile - toFile + D1_POS_OFFSET) : (toFile - fromFile + D1_NEG_OFFSET);

    if (fromRank - fromFile == toRank - toFile)
        return fromFile > toFile ? (fromFile - toFile + D2_POS_OFFSET) : (toFile - fromFile + D2_NEG_OFFSET);

    if (fromRank - toRank   > 1)
        return KNIGHT_OFFSET1 + (fromFile > toFile);

    if (toRank   - fromRank > 1)
        return KNIGHT_OFFSET2 + (fromFile > toFile);

    if (toFile   - fromFile > 1)
        return KNIGHT_OFFSET3 + (fromRank > toRank);

    if (fromFile - toFile   > 1)
        return KNIGHT_OFFSET4 + (fromRank > toRank);

    std::cout << "Hm this shouldnt happen from:" << from << " to:" << to << std::endl;
    return 0;
}

struct Block {
    using Input  = std::array<std::array<std::array<float, 10>, 10>, FILTER>;
    using Output = Input;

    ConvolutionalLayer<FILTER, FILTER> cl1;
    Batchnorm         <FILTER        > bn1;
    ConvolutionalLayer<FILTER, FILTER> cl2;
    Batchnorm         <FILTER        > bn2;

    Output& forward(Input &input) {
        Input identity; memcpy(&identity, &input, sizeof(Input));

        Output &out = cl1.forward(input);
        bn1.forward(out);
        out = bn1.ReLUInplace();
        out = cl2.forward(out);
        bn2.forward(out, &identity);
        out = bn2.ReLUInplace();

        return out;
    }

    void loadWeights(std::istream &in) {
        cl1.loadWeights(in);
        bn1.loadWeights(in);
        cl2.loadWeights(in);
        bn2.loadWeights(in);
    }
};

struct Network {
    using Input  = std::array<std::array<std::array<float, 10>, 10>, 12>;
    using Output = std::array<std::array<std::array<float, 10>, 10>, 64>;

    ConvolutionalLayer<12, FILTER> cl1;
    std::array<Block, BLOCKS> blocks;
    ConvolutionalLayer<FILTER, 64> cl2;

    void scoreMoveList(Input &input, MoveList &ml, float temperature, bool stm) {
        Output &out = forward(input);

        float sum = 0.0f;
        float scores[218];

        for (int i = 0; i < ml.length; i++) {
            int from     = extract<FROM>(ml.moves[i].move);
            int to       = extract<TO  >(ml.moves[i].move);

            if (!stm) {
                from ^= 56;
                to   ^= 56;
            }

            int layer    = moveToLayer(from, to);
            int fromFile = from & 0b111;
            int fromRank = rankOf(from);

            sum += (scores[i] = std::exp(out[layer][fromRank + 1][fromFile + 1] / temperature));
        }

        for (int i = 0; i < ml.length; i++)
            ml.moves[i].score = (scores[i] / sum) * 16384.0f;
    }

    void loadWeights(std::istream &in) {
        cl1.loadWeights(in);

        for (Block &b : blocks)
            b.loadWeights(in);

        cl2.loadWeights(in);
    }

    void loadDefault();

private:
    Output& forward(Input &input) {
        auto &out = cl1.forward(input);
              out = cl1.ReLUInplace();
        
        for (Block &b : blocks)
            out = b.forward(out);

        return cl2.forward(out);
    }
};

inline void bbsToPaddedInput(std::array<u64, 13> &bitboards, Color stm, std::array<std::array<std::array<float, 10>, 10>, 12> &input) {
    memset(&input, 0, sizeof(input));

    for (int pc = WHITE_PAWN; pc != NO_PIECE; pc++) {
        u64 pieceBB = bitboards[pc];

        while (pieceBB) {
            int square = popLSB(pieceBB);
            int piece  = pc;

            if (!stm) {
                square ^= 56;
                piece = makePiece(typeOf(pc), !colorOf(pc));
            }

            int rank = square / 8;
            int file = square % 8;

            input[piece][rank + 1][file + 1] = 1.0f;
        }
    }
}

struct PolicyWeights {
    std::array<int16_t, IN_SIZE * HIDDEN_SIZE> l0Weights;
    std::array<int16_t, HIDDEN_SIZE> l0Biases;
    std::array<int16_t, HIDDEN_SIZE * OUT_SIZE> l1Weights;
    std::array<int16_t, OUT_SIZE> l1Biases;
};

inline int16_t ReLU(int16_t val) {
    return std::max(val, int16_t(0));
}

class PolicyNet {
    PolicyWeights weights;
    std::array<int16_t, HIDDEN_SIZE> accumulator;

public:
    void  loadDefault();
    void  initAccumulator(std::array<u64, 13> &bitboards, Color stm, u64 threats);
    void  scoreMovesList(MoveList &ml, bool stm, std::array<u64, 13> &bitboards, u64 threats, int temperature = 1);
    float forward(Move move, bool stm);
};

#endif