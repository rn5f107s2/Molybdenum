#include "Position.h"
#include "eval.h"
#include "PSQT.h"

int evaluate(Position &pos) {
    return evalPSQT(pos) * (pos.sideToMove ? -1 : 1);
}

int evalPSQT(Position &pos) {
    int eval = 0;

    for (int pt = WHITE_PAWN; pt != NO_PIECE; pt++) {
        u64 pieceBB = pos.bitBoards[pt];

        while (pieceBB) {
            int square = popLSB(pieceBB);

            eval += PSQT[pt][square];
        }
    }

    return eval;
}