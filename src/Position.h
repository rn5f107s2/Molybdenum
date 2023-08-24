#ifndef MOLYBDENUM_POSITION_H
#define MOLYBDENUM_POSITION_H

#include <string>
#include <array>
#include "Constants.h"
#include "BitStuff.h"
#include "Move.h"
#include "Utility.h"
#include "Transpositiontable.h"

class Position {
    public:
        void setBoard(std::string fen);
        void clearBoard();
        void printBoard();
        void makeMove(Move move);
        void unmakeMove(Move move);
        Move fromToToMove(int from, int to, int promotionPiece, int flag = NORMAL);
        std::array<u64, 13> bitBoards;
        std::array<int, 64> pieceLocations;
        int castlingRights;
        u64 enPassantSquare;
        int plys50moveRule;
        bool sideToMove;
        u64 key();
        bool hasRepeated(int plysInSearch);
    private:
        Stack<int>  capturedHistory;
        Stack<int>  plys50mrHistory;
        Stack<int>  castlingHistory;
        Stack<u64> enPassantHistory;
        Stack<u64> keyHistory;
};


#endif //MOLYBDENUM_POSITION_H
