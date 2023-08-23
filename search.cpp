#include "Position.h"
#include "Movegen.h"
#include "search.h"
#include "eval.h"
#include "Constants.h"

template<bool ROOT>
int search(int alpha, int beta, Position &pos, int depth);

int nodeCount = 0;

int startSearch(Position &pos, int depth) {
    nodeCount = 0;
    return search<true>(-INFINITE, INFINITE, pos, depth);
}

template<bool ROOT>
int search(int alpha, int beta, Position &pos, int depth) {
    if (depth <= 0)
        return evaluate(pos);

    MoveList ml;
    Move bestMove;
    int bestScore = -INFINITE;
    bool check = generateMoves(pos, ml);

    while (ml.currentIdx > 0) {
        Move currentMove = ml.moves[--ml.currentIdx].move;
        pos.makeMove(currentMove);
        nodeCount++;

        int score = -search<false>(-beta, -alpha, pos, depth - 1);

        pos.unmakeMove(currentMove);

        if (score > bestScore) {
            bestScore = score;
            bestMove = currentMove;

            if(score > alpha) {
                alpha = score;

                if (score > beta)
                    return beta;
            }
        }
    }

    if (bestScore == -INFINITE) {
        return check ? -MATE : DRAW;
    }

    if constexpr (ROOT)
        std::cout << "bestmove " << moveToString(bestMove) << "\n";
    return bestScore;
}
