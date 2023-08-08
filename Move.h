#ifndef MOLYBDENUM_MOVE_H
#define MOLYBDENUM_MOVE_H

#include <cstdint>
#include <array>

using Move = uint16_t;

enum MoveOperation {
    FROM, TO, PROMOTIONTYPE, FLAG
};

enum MoveFlags {
    NORMAL, PROMOTION, CASTLING, ENPASSANT
};

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

inline Move createMove(int from, int to, int promotionPiece, int flag) {
    Move move;
    move  = flag << flagShift;
    move |= promotionPiece << promotionShift;
    move |= to << toShift;
    move |= from;
    return move;
}

#endif //MOLYBDENUM_MOVE_H
