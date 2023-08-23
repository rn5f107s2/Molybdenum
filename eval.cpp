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
    int eval = 0;
    eval +=  __builtin_popcountll(pos.bitBoards[pawnIdx + PAWN]) * PawnValue;
    eval += __builtin_popcountll(pos.bitBoards[pawnIdx + KNIGHT]) * KnightValue;
    eval += __builtin_popcountll(pos.bitBoards[pawnIdx + BISHOP]) * BishopValue;
    eval += __builtin_popcountll(pos.bitBoards[pawnIdx + ROOK]) * RookValue;
    eval += __builtin_popcountll(pos.bitBoards[pawnIdx + QUEEN]) * QueenValue;
    return eval;
}