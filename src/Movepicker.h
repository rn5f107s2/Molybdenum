#include "searchUtil.h"
#include "Movegen.h"

std::array<Move, 2> emptyKillers = {NO_MOVE, NO_MOVE};
FromToHist emptyMain  = {{{0}}};
PieceToHist emptyCont = {{{0}}};

const std::array<std::array<int, 13>, 13> MVVLVA =
         {{
                 {1050000, 2050000, 3050000, 4050000, 5050000, 6050000, 1050000, 2050000, 3050000, 4050000, 5050000, 6050000, 0},
                 {1040000, 2040000, 3040000, 4040000, 5040000, 6040000, 1040000, 2040000, 3040000, 4040000, 5040000, 6040000, 0},
                 {1030000, 2030000, 3030000, 4030000, 5030000, 6030000, 1030000, 2030000, 3030000, 4030000, 5030000, 6030000, 0},
                 {1020000, 2020000, 3020000, 4020000, 5020000, 6020000, 1020000, 2020000, 3020000, 4020000, 5020000, 6020000, 0},
                 {1010000, 2010000, 3010000, 4010000, 5010000, 6010000, 1010000, 2010000, 3010000, 4010000, 5010000, 6010000, 0},
                 {1000000, 2000000, 3000000, 4000000, 5000000, 6000000, 1000000, 2000000, 3000000, 4000000, 5000000, 6000000, 0},
                 {1050000, 2050000, 3050000, 4050000, 5050000, 6050000, 1050000, 2050000, 3050000, 4050000, 5050000, 6050000, 0},
                 {1040000, 2040000, 3040000, 4040000, 5040000, 6040000, 1040000, 2040000, 3040000, 4040000, 5040000, 6040000, 0},
                 {1030000, 2030000, 3030000, 4030000, 5030000, 6030000, 1030000, 2030000, 3030000, 4030000, 5030000, 6030000, 0},
                 {1020000, 2020000, 3020000, 4020000, 5020000, 6020000, 1020000, 2020000, 3020000, 4020000, 5020000, 6020000, 0},
                 {1010000, 2010000, 3010000, 4010000, 5010000, 6010000, 1010000, 2010000, 3010000, 4010000, 5010000, 6010000, 0},
                 {1000000, 2000000, 3000000, 4000000, 5000000, 6000000, 1000000, 2000000, 3000000, 4000000, 5000000, 6000000, 0},
                 {     0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0, 0}
         }};

template<bool qsearch>
class Movepicker {
    private:
        std::array<Move, 2> *killers{};
        FromToHist *mainHist{};
        PieceToHist *contHist1{};
        PieceToHist *contHist2{};
        Move ttMove = NO_MOVE;
        Move prioMove = NO_MOVE;
        MoveList ml{};
        Position *pos;
        ScoredMove *currentMove = nullptr;
        ScoredMove *endMoveList = nullptr;
        bool scored = false;
        bool searchedTT = false;
        bool searchedPrio = true;
        u64 checkers;

    public:
        Movepicker(Position *p, Move ttm, 
                      std::array<Move, 2> *k = &emptyKillers, 
                      FromToHist  *main  = &emptyMain, 
                      PieceToHist *cont1 = &emptyCont, 
                      PieceToHist *cont2 = &emptyCont, 
                      u64 check = 0ULL) 
        {
            ttMove = ttm; 
            killers = k; 
            mainHist = main; 
            contHist1 = cont1; 
            contHist2 = cont2;
            pos = p;
            checkers = check;
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


            for (ScoredMove* i = currentMove; i < endMoveList; i++) {
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

            if (!searchedPrio) {
                searchedPrio = true;
                return prioMove;
            }

            if (   ttMove 
                && !searchedTT
                && (searchedTT = true)
                && pos->isLegal(ttMove)) {

                return ttMove;
            }

            if (!scored && (scored = true)) {

                generateMoves<qsearch>(*pos, ml, checkers);

                currentMove = &ml.moves[0];
                endMoveList = &ml.moves[ml.length];

                Move next = scoreMoves();
                
                if (next != ttMove)
                    return next;
            }

            if (currentMove == endMoveList)
                return NO_MOVE;

            Move next = sortNext();
            return (next == prioMove) ? sortNext() : next; 
        }

        inline void setPrioMove(Move pm) {
            prioMove = pm;
            searchedPrio = false;
        }
        
};
