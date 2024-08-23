#ifndef MOLYBDENUM_MOVE_H
#define MOLYBDENUM_MOVE_H

#include <cstdint>
#include <array>
#include "Constants.h"
#include "BitStuff.h"
#include "Utility.h"

using Move = uint16_t;

enum MoveOperation {
    FROM, TO, PROMOTIONTYPE, FLAG
};

enum MoveFlag {
    NORMAL, PROMOTION, CASTLING, ENPASSANT
};

const Move NO_MOVE = 0;
const Move NULL_MOVE = 65;

// first 6 bits = from square, next 6 bits = to square, next 2 bits promotion piece rest is flags for the kind of move

static const int fromMask      =           0b111111;
static const int toMask        =     0b111111000000;
static const int promotionMask =   0b11000000000000;
static const int flagMask      = 0b1100000000000000;
static const std::array<int, 4> maskArray = {fromMask, toMask, promotionMask, flagMask};

static const int fromShift = 0;
static const int toShift = 6;
static const int promotionShift = 12;
static const int flagShift = 14;
static const std::array<int, 4> shiftArray = {fromShift, toShift, promotionShift, flagShift};

template<MoveOperation idx>
inline int extract(Move move) {
    return (move & maskArray[idx]) >> shiftArray[idx];
}

inline Move createMove(int from, int to, int promotionPiece = PROMO_KNIGHT, int flag = NORMAL) {
    Move move;
    move  = flag << flagShift;
    move |= promotionPiece << promotionShift;
    move |= to << toShift;
    move |= from;
    return move;
}

inline std::string moveToString(Move move) {
    int from = extract<FROM>(move);
    int to   = extract<TO  >(move);

    char fromFile = char('a' + (fileOf(from)));
    char toFile   = char('a' + (fileOf(to  )));

    std::string moveS;
    moveS += fromFile;
    moveS += char('1' + (rankOf(from)));
    moveS += toFile;
    moveS += char('1' + (rankOf(to)));

    if (extract<FLAG>(move) == PROMOTION)
        moveS += pieceToChar(extract<PROMOTIONTYPE>(move) + 7);

    return moveS;
}

struct ScoredMove {
    Move move;
    int score;
};

struct MoveList {
    std::array<ScoredMove, 218> moves = {};
    int currentIdx = 0;
    int length = 0;
};

#endif //MOLYBDENUM_MOVE_H
