#ifndef MOLYBDENUM_POSITION_H
#define MOLYBDENUM_POSITION_H

#include <string>
#include <array>
#include "Constants.h"
#include "BitStuff.h"
#include "Move.h"

class Position {
    public:
        void setBoard(std::string fen);
        void clearBoard();
        void printBoard();
        void makeMove(Move move);
        Move fromToToMove(int from, int to, int promotionPiece);
    private:
        std::array<u64, 12> bitBoards;
        std::array<int, 64> pieceLocations;
        std::array<bool, 4> castlingRights;
        u64 enPassantSquare;
        int plys50moveRule;
        bool sideToMove;
};

inline int charIntToPiece(int charInt) {
    char piece = char(charInt + '0');
    switch (piece) {
        case 'P':
            return WHITE_PAWN;
        case 'N':
            return WHITE_KNIGHT;
        case 'B':
            return WHITE_BISHOP;
        case 'R':
            return WHITE_ROOK;
        case 'Q':
            return WHITE_QUEEN;
        case 'K':
            return WHITE_KING;
        case 'p':
            return  BLACK_PAWN;
        case 'n':
            return BLACK_KNIGHT;
        case 'b':
            return BLACK_BISHOP;
        case 'r':
            return BLACK_ROOK;
        case 'q':
            return BLACK_QUEEN;
        case 'k':
            return BLACK_KING;
        default:
            return NO_PIECE;
    }
}

inline int castlingCharToIndex(int charInt) {
    switch (charInt) {
        case WHITE_KING:
            return 0;
        case WHITE_QUEEN:
            return 1;
        case BLACK_KING:
            return 2;
        default:
            return 3;

    }
}

inline char pieceToChar(int piece) {
    switch (piece) {
        case WHITE_PAWN:
            return 'P';
        case WHITE_KNIGHT:
            return 'N';
        case WHITE_BISHOP:
            return 'B';
        case WHITE_ROOK:
            return 'R';
        case WHITE_QUEEN:
            return 'Q';
        case WHITE_KING:
            return 'K';
        case BLACK_PAWN:
            return  'p';
        case BLACK_KNIGHT:
            return 'n';
        case BLACK_BISHOP:
            return 'b';
        case BLACK_ROOK:
            return 'r';
        case BLACK_QUEEN:
            return 'q';
        case BLACK_KING:
            return 'k';
        default:
            return ' ';
    }
}

inline u64 stringToSquare(std::string epSquare) {
    int file = 7 - (epSquare.at(0) - 'a');
    int  row = (epSquare.at(1) - '0') - 1;

    return 1ULL << (row * 8 + file);
}

inline int stringRoRule50(const std::string& rule50) {
    return std::stoi(rule50);
}

inline int typeOf(int piece) {
    return piece % 6;
}


#endif //MOLYBDENUM_POSITION_H
