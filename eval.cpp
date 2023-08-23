#include "Position.h"
#include "eval.h"

const int PawnValue   = 100;
const int KnightValue = 300;
const int BishopValue = 300;
const int RookValue   = 500;
const int QueenValue  = 900;

int evaluate(Position &pos) {
    return evalColor(pos, pos.sideToMove) - evalColor(pos, !pos.sideToMove);
}

int evalColor(Position &pos, bool color) {
    int pawnIdx = color ? WHITE_PAWN : BLACK_PAWN;
    return  __builtin_popcount(pos.bitBoards[pawnIdx + PAWN]) * PawnValue
          + __builtin_popcount(pos.bitBoards[pawnIdx + KNIGHT]) * KnightValue
          + __builtin_popcount(pos.bitBoards[pawnIdx + BISHOP]) * BishopValue
          + __builtin_popcount(pos.bitBoards[pawnIdx + ROOK]) * RookValue
          + __builtin_popcount(pos.bitBoards[pawnIdx + QUEEN]) * QueenValue;
}