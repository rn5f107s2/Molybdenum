#include <iostream>
#include <cstring>
#include "Position.h"

void Position::setBoard(std::string fen) {
    clearBoard();

    u64 bitToSet = 1L << 63;
    std::string plys50mr;
    std::string epSquare;
    int spaceCount = 0;
    int fenLength = (int) fen.length();

    for (int i = 0; i != fenLength; i++) {
        char currentChar = fen.at(i);

        if (currentChar == ' ')
            spaceCount++;

        if (   currentChar == ' '
            || currentChar == '/'
            || currentChar == '-')
            continue;

        int charAsInt = currentChar - '0';

        if (spaceCount == 1) {
            sideToMove = currentChar == 'w';
            continue;
        }

        if (spaceCount == 2) {
            int idx = castlingCharToIndex(charIntToPiece(charAsInt));
            castlingRights |= 1 << idx;
            continue;
        }

        if (spaceCount == 3) {
            epSquare += currentChar;
            continue;
        }

        if (spaceCount == 4) {
            plys50mr += currentChar;
            continue;
        }

        if (charAsInt < 10) {
            bitToSet = bitToSet >> charAsInt;
            continue;
        }

        int piece = charIntToPiece(charAsInt);
        bitBoards[piece] ^= bitToSet;
        pieceLocations[lsb(bitToSet)] = piece;
        bitToSet = bitToSet >> 1;
    }

    enPassantSquare = !epSquare.empty() ? stringToSquare(epSquare) : 0ULL;
    plys50moveRule  = !plys50mr.empty() ? stringRoRule50(plys50mr) : 0;
}

Move Position::fromToToMove(int from, int to, int promotionPiece) {
    int flag = NORMAL;
    int movingPiece = pieceLocations[from];
    bool captured = pieceLocations[to] != NO_PIECE;

    if (typeOf(movingPiece) == PAWN) {
        if (fileOf(from) != fileOf(to) && !captured)
            flag = ENPASSANT;

        if ((1L << to) & promotionRanks)
            flag = PROMOTION;
    }

    if (typeOf(movingPiece) == KING && std::abs(fileOf(from) - fileOf(to)) == 2)
        flag = CASTLING;

    return createMove(from, to, promotionPiece, flag);
}

void Position::makeMove(Move move) {
    int from = extract<FROM>(move);
    int to   = extract<TO>(move);
    int flag = extract<FLAG>(move);
    int movedPiece    = pieceLocations[from];
    int capturedPiece = pieceLocations[to];

    plys50mrHistory.push(plys50moveRule);
    castlingHistory.push(castlingRights);
    capturedHistory.push(capturedPiece);
    enPassantHistory.push(enPassantSquare);

    plys50moveRule++;
    pieceLocations[from] = NO_PIECE;
    bitBoards[movedPiece] ^= 1ULL << from;

    if (flag == PROMOTION) {
        movedPiece = extract<PROMOTIONTYPE>(move);
        plys50moveRule = 0;
    }

    if (flag == ENPASSANT) {
        int capturedPawn = movedPiece == WHITE_PAWN ? BLACK_PAWN : WHITE_PAWN;
        u64 captureSquare = movePawn(enPassantSquare, !sideToMove);
        bitBoards[capturedPawn] ^= captureSquare;
        pieceLocations[lsb(captureSquare)] = NO_PIECE;
    }

    if (flag == CASTLING) {
        int rookFrom = from > to ? to - 1 : to + 2;
        int rookTo   = from > to ? to + 1 : to - 1;
        int rook     = pieceLocations[rookFrom];

        pieceLocations[rookFrom] = NO_PIECE;
        pieceLocations[rookTo  ] = rook;
        bitBoards[rook] ^= (1ULL << rookFrom) | (1ULL << rookTo);
    }

    enPassantSquare = 0ULL;

    if (typeOf(movedPiece) == PAWN) {
        if ((from ^ to) == 16)
            enPassantSquare = 1L << (to - (from > to ? -8 : 8));

        plys50moveRule = 0;
    }

    if (capturedPiece != NO_PIECE) {
        bitBoards[capturedPiece] ^= 1ULL << to;
        plys50moveRule = 0;
    }

    pieceLocations[to] = movedPiece;
    bitBoards[movedPiece] ^= 1L << to;
    sideToMove = !sideToMove;
}

void Position::unmakeMove(Move move) {
    int from = extract<FROM>(move);
    int to   = extract<TO  >(move);
    int flag = extract<FLAG>(move);
    int movingPiece = pieceLocations[to];
    int capturedPiece = capturedHistory.pop();

    plys50moveRule = plys50mrHistory.pop();
    castlingRights = castlingHistory.pop();
    enPassantSquare = enPassantHistory.pop();

    pieceLocations[from] = movingPiece;
    pieceLocations[to  ] = capturedPiece;
    bitBoards[movingPiece] ^= 1ULL << to;

    if (flag == PROMOTION)
        movingPiece = sideToMove ? WHITE_PAWN : BLACK_PAWN;

    bitBoards[movingPiece] ^= 1ULL << from;
    bitBoards[capturedPiece] ^= 1ULL << to;

    if (flag == CASTLING) {
        int rookFrom = from > to ? to - 1 : to + 2;
        int rookTo   = from > to ? to + 1 : to - 1;
        int rook     = pieceLocations[rookTo];

        pieceLocations[rookTo  ] = NO_PIECE;
        pieceLocations[rookFrom] = rook;
        bitBoards[rook] ^= (1ULL << rookFrom) | (1ULL << rookTo);
    }

    if (flag == ENPASSANT) {
        int capturedPawn  = movingPiece == WHITE_PAWN ? BLACK_PAWN : WHITE_PAWN;
        u64 captureSquare = movePawn(enPassantSquare, sideToMove);
        bitBoards[capturedPawn] ^= captureSquare;
        pieceLocations[lsb(captureSquare)] = capturedPawn;
    }

    sideToMove = !sideToMove;
}

void Position::clearBoard() {
    for (int i = 0; i != 64; i++)
        pieceLocations[i] = NO_PIECE;

    memset(&bitBoards, 0ULL, bitBoards.size() * sizeof(typeof(bitBoards[0])));
    castlingRights = 0;
    plys50moveRule = 0;
    enPassantSquare = 0;
    sideToMove = WHITE;
}

void Position::printBoard() {
    std::string board;
    std::string castling;

    for (int square = 63; square != -1; square--) {
        for (int piece = 0; piece != 12; piece++) {
            if (bitBoards[piece] & (1L << square)) {
                board += (pieceToChar(piece));
                break;
            }
            else if (piece == 11)
                board += ('-');
        }

        board += ' ';

        if (square % 8 == 0) {
            std::cout << board << "\n";
            board = "";
        }
    }

    if (castlingRights & WHITE_CASTLE_KINGSIDE ) castling += "K";
    if (castlingRights & WHITE_CASTLE_QUEENSIDE) castling += "Q";
    if (castlingRights & BLACK_CASTLE_KINGSIDE ) castling += "k";
    if (castlingRights & BLACK_CASTLE_QUEENSIDE) castling += "q";

    std::cout << "\n";
    std::cout << "50 move rule counter: " << plys50moveRule << "\n";
    std::cout << "side to move        : " << (sideToMove ? "w" : "b") << "\n";
    std::cout << "en passant square   : " << (enPassantSquare) << "\n";
    std::cout << "castling rights     : " << castling << "\n";
}

