#include "Position.h"
#include "Movegen.h"
#include "search.h"
#include "eval.h"
#include "Constants.h"

template<bool ROOT>
int search(int alpha, int beta, Position &pos, int depth, SearchInfo &si);

int startSearch(Position &pos) {
    return iterativeDeepening(pos);
}

int iterativeDeepening(Position  &pos) {
    int score;
    SearchInfo si;

    for (int depth = 1; depth != 6; depth++) {
        score = search<true>(-INFINITE, INFINITE, pos, depth, si);
        std::cout << "info depth " << depth << " currmove " << moveToString(si.bestRootMove) << " score cp " << score << "\n";
    }

    std::cout << "bestmove " << moveToString(si.bestRootMove) << "\n";
    return score;
}

template<bool ROOT>
int search(int alpha, int beta, Position &pos, int depth, SearchInfo &si) {
    if (depth <= 0)
        return evaluate(pos);

    MoveList ml;
    Move bestMove;
    int bestScore = -INFINITE;
    bool check = generateMoves(pos, ml);

    while (ml.currentIdx > 0) {
        Move currentMove = ml.moves[--ml.currentIdx].move;
        pos.makeMove(currentMove);
        si.nodeCount++;

        int score = -search<false>(-beta, -alpha, pos, depth - 1, si);

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
        si.bestRootMove = bestMove;
    return bestScore;
}
