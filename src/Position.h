#ifndef MOLYBDENUM_POSITION_H
#define MOLYBDENUM_POSITION_H

#include <string>
#include <array>

#include "Constants.h"
#include "BitStuff.h"
#include "Move.h"
#include "Utility.h"
#include "Transpositiontable.h"
#include "nnue.h"

class Position {
    public:
        Net net;
        void setBoard(std::string fen);
        void clearBoard();
        void printBoard();
        void makeMove(Move move);
        void unmakeMove(Move move);
        Move fromToToMove(int from, int to, int promotionPiece, int flag = NORMAL);
        std::array<u64, 13> bitBoards{};
        std::array<Piece, 64> pieceLocations{};
        int castlingRights{};
        u64 enPassantSquare{};
        int plys50moveRule{};
        int phase{};
        int movecount{};
        bool sideToMove{};
        u64 key();
        void makeNullMove();
        void unmakeNullMove();
        bool hasRepeated(int plysInSearch);
        bool isCapture(Move move);
        std::string fen();
        template<Color c> u64 getPieces(PieceType pt);
        template<PieceType pt> u64 getPieces(Color c);
        inline u64 getPieces(Color c, PieceType pt);
        inline u64 getPieces(PieceType pt);
        inline Piece pieceOn(int sq);
        inline u64 getOccupied();
        template<Color c> u64 getOccupied();
        inline std::string moveToSAN(Move move, u64 attacks);
        inline int ambigious(Move move, u64 attacks);
    private:
        Stack<Piece>  capturedHistory;
        Stack<int>  plys50mrHistory;
        Stack<int>  castlingHistory;
        Stack<u64> enPassantHistory;
        Stack<u64> keyHistory;
};

template<Color c> inline
u64 Position::getPieces(PieceType pt) {
    int idx = pt + (6 * !c);
    return bitBoards[idx];
}

template<PieceType pt> inline
u64 Position::getPieces(Color c) {
    int idx = pt + (6 * !c);
    return bitBoards[idx];
}

inline u64 Position::getPieces(Color c, PieceType pt) {
    int idx = pt + (6 * !c);
    return bitBoards[idx];
}

inline u64 Position::getPieces(PieceType pt) {
    return bitBoards[pt] | bitBoards[pt + 6];
}

inline Piece Position::pieceOn(int sq) {
    return pieceLocations[sq];
}

template<Color c> inline
u64 Position::getOccupied() {
    return   getPieces<c>(PAWN)
           | getPieces<c>(KNIGHT)
           | getPieces<c>(BISHOP)
           | getPieces<c>(ROOK)
           | getPieces<c>(QUEEN)
           | getPieces<c>(KING);
}

inline u64 Position::getOccupied() {
    return getOccupied<WHITE>() | getOccupied<BLACK>();
}

inline bool Position::isCapture(Move move) {
    int to = extract<TO>(move);
    return pieceLocations[to] != NO_PIECE;
}

inline std::string Position::moveToSAN(Move move, u64 attacks) {
    int from   = extract<FROM>(move);
    int to     = extract<TO  >(move);

    if (extract<FLAG>(move) == CASTLING)
        return from > to ? "0-0 " : "0-0-0 ";

    std::string piece;
    std::string toSquare;

    if (typeOf(pieceOn(from)) != PAWN) {
        piece += std::toupper(pieceToChar(typeOf(pieceOn(from))));

        int a = ambigious(move, attacks);

        if (a & 1)
            piece += char('a' + (fileOf(from)));

        if (a & 2)
            piece += char('a' + (rankOf(from)));

        if (isCapture(move))
            piece += "x";
    } else {
        if (isCapture(move) || extract<FLAG>(move) == ENPASSANT) {
            piece += char('a' + (fileOf(from)));
            piece += 'x';
        }
    }
   

    return piece + char('a' + (fileOf(to))) + char('1' + (rankOf(to))) + " ";
}

inline int Position::ambigious(Move move, u64 attacks) {
    int from = extract<FROM>(move);
    int to   = extract<TO  >(move);
    Piece pc = pieceOn(from);

    u64 pieces  = getPieces(sideToMove, PieceType(typeOf(pc))) ^ (1ULL << from);
    u64 overlap = attacks & pieces;

    if (!overlap)
        return 0;

    if (!(lFIleOf(to) & overlap))
        return 1;

    if (!(lRankOf(to) & overlap))
        return 2;

    return 3;
}

#endif //MOLYBDENUM_POSITION_H
