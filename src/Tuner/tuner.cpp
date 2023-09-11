#include "tuner.h"
#include "../Position.h"
#include "../search.h"

#ifdef tune

int tuneQ(int alpha, int beta, Position &pos) {
    Move currentMove;
    bool check;
    int bestScore = tuneEval(pos);

    if (bestScore >= beta)
        return bestScore;

    Movepicker mp;
    while ((currentMove = pickNextMove<true>(mp, 0, pos, check)) != 0) {
        pos.makeMove(currentMove);

        int score = -tuneQ(-beta, -alpha, pos);

        pos.unmakeMove(currentMove);

        if (score > bestScore) {
            bestScore = score;

            if (score > alpha) {
                alpha = score;

                if (score >= beta)
                    return score;
            }
        }
    }

    return bestScore;
}

int tuneEval(Position &pos) {

}

#endif