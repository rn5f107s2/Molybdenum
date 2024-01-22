#include "searchUtil.h"
#include "Movegen.h"
#include "Movepicker.h"

template<bool qsearch>
class Movepickernew {
    private:
        std::array<Move, 2> *killers{};
        FromToHist *mainHist{};
        PieceToHist *contHist1{};
        PieceToHist *contHist2{};
        Move ttMove = NO_MOVE;
        MoveList ml{};
        Position *pos;
        ScoredMove *currentMove = nullptr;
        ScoredMove *endMoveList = nullptr;
        bool scored = false;

    public:
        Movepickernew(Position *p, Move ttm, 
                      std::array<Move, 2> *k, 
                      FromToHist *main, 
                      PieceToHist *cont1, 
                      PieceToHist *cont2, 
                      u64 checkers = 0ULL) 
        {
            ttMove = ttm; 
            killers = k; 
            mainHist = main; 
            contHist1 = cont1; 
            contHist2 = cont2;
            pos = p;

            generateMoves<qsearch>(*pos, ml, checkers);

            currentMove = &ml.moves[0];
            endMoveList = &ml.moves[ml.length];
        }

        inline Move scoreMoves() {
            int idx = 0;

            int bestScore = std::numeric_limits<int>::min(); 
            ScoredMove *best = currentMove;

            while (idx < ml.length) {
                ScoredMove *current = &ml.moves[idx++];
                Move move  = current->move;
                int *score = &current->score;

                int from = extract<FROM>(move);
                int to   = extract<TO  >(move);
                int movingPiece   = pos->pieceLocations[from];
                int capturedPiece = pos->pieceLocations[to];
                Piece pc = pos->pieceOn(from);

                if (move == ttMove)
                    *score = 10000000;
                else if (move == killers[0][0])
                    *score = 900000;
                else if (move == killers[0][1])
                    *score = 800000;

                *score += mainHist [0][from][to];
                *score += contHist1[0][pc][to];
                *score += contHist2[0][pc][to];

                *score += MVVLVA[movingPiece][capturedPiece];

                if (*score > bestScore) {
                    bestScore = *score;
                    best = current;
                }
            }

            ScoredMove temp = *currentMove;
            *currentMove = *best;
            *best = temp;
 
            return currentMove++->move;
        }

        inline Move sortNext() {
            ScoredMove *best = currentMove;
            int bestScore = std::numeric_limits<int>::min();


            for (ScoredMove* i = currentMove; i != endMoveList; i++) {
                if (i->score > bestScore) {
                    bestScore = i->score;
                    best = i;
                }
            }

            ScoredMove temp = *currentMove;
            *currentMove = *best;
            *best = temp;

            return currentMove++->move;
        }

        inline Move pickMove() {
            if (!scored && (scored = true))
                return scoreMoves();

            if (currentMove == endMoveList)
                return NO_MOVE;

            return sortNext();
        }
        
};
