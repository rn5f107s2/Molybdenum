#ifndef MOLYBDENUM_CONSTANTS_H
#define MOLYBDENUM_CONSTANTS_H

#include <array>

using u64 = uint64_t;

//Position Stuff
constexpr bool WHITE = true;
constexpr bool BLACK = false;

constexpr int WHITE_PAWN   = 0;
constexpr int WHITE_KNIGHT = 1;
constexpr int WHITE_BISHOP = 2;
constexpr int WHITE_ROOK   = 3;
constexpr int WHITE_QUEEN  = 4;
constexpr int WHITE_KING   = 5;
constexpr int BLACK_PAWN   = 6;
constexpr int BLACK_KNIGHT = 7;
constexpr int BLACK_BISHOP = 8;
constexpr int BLACK_ROOK   = 9;
constexpr int BLACK_QUEEN  = 10;
constexpr int BLACK_KING   = 11;
constexpr int NO_PIECE     = 12;

enum PieceType {
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
};

constexpr int PROMO_KNIGHT = 0;
constexpr int PROMO_BISHOP = 1;
constexpr int PROMO_ROOK   = 2;
constexpr int PROMO_QUEEN  = 3;

constexpr u64 RANK1 = 256 - 1;
constexpr u64 RANK2 = RANK1 << 8;
constexpr u64 RANK3 = RANK2 << 8;
constexpr u64 RANK4 = RANK3 << 8;
constexpr u64 RANK5 = RANK4 << 8;
constexpr u64 RANK6 = RANK5 << 8;
constexpr u64 RANK7 = RANK6 << 8;
constexpr u64 RANK8 = RANK7 << 8;

constexpr u64 promotionRanks = RANK1 | RANK8;

constexpr u64 FILEH = 1ULL | (1ULL << 8) | (1ULL << 16) | (1ULL << 24) | (1ULL << 32) | (1ULL << 40) | (1ULL << 48) | (1ULL << 56);
constexpr u64 FILEG = FILEH << 1;
constexpr u64 FILEF = FILEH << 2;
constexpr u64 FILEE = FILEH << 3;
constexpr u64 FILED = FILEH << 4;
constexpr u64 FILEC = FILEH << 5;
constexpr u64 FILEB = FILEH << 6;
constexpr u64 FILEA = FILEH << 7;

constexpr int WHITE_CASTLE_KINGSIDE  = 1;
constexpr int WHITE_CASTLE_QUEENSIDE = 2;
constexpr int BLACK_CASTLE_KINGSIDE  = 4;
constexpr int BLACK_CASTLE_QUEENSIDE = 8;
constexpr u64 WHITE_CASTLING_SQUARES_KINGSIDE  = (FILEF | FILEG) & RANK1;
constexpr u64 WHITE_CASTLING_SQUARES_QUEENSIDE = (FILED | FILEC) & RANK1;
constexpr u64 BLACK_CASTLING_SQUARES_KINGSIDE  = (FILEF | FILEG) & RANK8;
constexpr u64 BLACK_CASTLING_SQUARES_QUEENSIDE = (FILEC | FILED) & RANK8;

constexpr u64 edgeFiles = FILEA | FILEH;

enum Slider {
    BISHOP_S, QUEEN_S, ROOK_S
};

#endif //MOLYBDENUM_CONSTANTS_H
