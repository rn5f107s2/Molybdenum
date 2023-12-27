#ifndef MOLYBDENUM_CONSTANTS_H
#define MOLYBDENUM_CONSTANTS_H

#include <array>
#include <cstdint>

using u64 = uint64_t;
using Color = bool;

//Position Stuff
constexpr Color WHITE = true;
constexpr Color BLACK = false;

enum PieceType {
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
};

enum Piece {
    WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
    BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING,
    NO_PIECE
};

enum PromotionType {
    PROMO_KNIGHT, PROMO_BISHOP, PROMO_ROOK, PROMO_QUEEN
};

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
constexpr int WHITE_CASTLING = WHITE_CASTLE_KINGSIDE | WHITE_CASTLE_QUEENSIDE;
constexpr int BLACK_CASTLE_KINGSIDE  = 4;
constexpr int BLACK_CASTLE_QUEENSIDE = 8;
constexpr int BLACK_CASTLING = BLACK_CASTLE_KINGSIDE | BLACK_CASTLE_QUEENSIDE;
constexpr u64 WHITE_CASTLING_SQUARES_KINGSIDE  = (FILEF | FILEG) & RANK1;
constexpr u64 WHITE_CASTLING_SQUARES_QUEENSIDE = (FILED | FILEC | FILEB) & RANK1;
constexpr u64 BLACK_CASTLING_SQUARES_KINGSIDE  = (FILEF | FILEG) & RANK8;
constexpr u64 BLACK_CASTLING_SQUARES_QUEENSIDE = (FILEC | FILED | FILEB) & RANK8;
constexpr std::array<int, 64> castlingMask =
        {WHITE_CASTLE_KINGSIDE, 0, 0, WHITE_CASTLING, 0, 0, 0, WHITE_CASTLE_QUEENSIDE,
                             0, 0, 0,              0, 0, 0, 0,                      0,
                             0, 0, 0,              0, 0, 0, 0,                      0,
                             0, 0, 0,              0, 0, 0, 0,                      0,
                             0, 0, 0,              0, 0, 0, 0,                      0,
                             0, 0, 0,              0, 0, 0, 0,                      0,
                             0, 0, 0,              0, 0, 0, 0,                      0,
        BLACK_CASTLE_KINGSIDE , 0, 0, BLACK_CASTLING, 0, 0, 0, BLACK_CASTLE_QUEENSIDE,
        };

constexpr u64 edgeFiles = FILEA | FILEH;

enum Slider {
    BISHOP_S, QUEEN_S, ROOK_S
};

constexpr int MAXDEPTH = 220;
constexpr int INFINITE = 200000;
constexpr int MATE     = 100000;
constexpr int MAXMATE  = MATE - MAXDEPTH;
constexpr int DRAW     = 0;

#endif //MOLYBDENUM_CONSTANTS_H
