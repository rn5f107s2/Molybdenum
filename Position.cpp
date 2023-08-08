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
            castlingRights[idx] = true;
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
    plys50moveRule  = stringRoRule50(plys50mr);
}

Move Position::fromToToMove(int from, int to, int promotionPiece) {
    int flag = NORMAL;
    int movingPiece = pieceLocations[from];
    bool captured = pieceLocations[to] != NO_PIECE;

    if (typeOf(movingPiece) == PAWN) {
        if (fileOf(from) != fileOf(to) && !captured) {
            flag = ENPASSANT;
            enPassantSquare = createSquare(fileOf(to), rankOf(from));
        }

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

    plys50moveRule++;
    pieceLocations[from] = NO_PIECE;
    bitBoards[movedPiece] ^= 1ULL << from;

    if (flag == PROMOTION) {
        movedPiece = extract<PROMOTIONTYPE>(move);
        plys50moveRule = 0;
    }

    if (flag == ENPASSANT) {
        int capturedPawn = movedPiece == WHITE_PAWN ? BLACK_PAWN : WHITE_PAWN;
        bitBoards[capturedPawn] ^= enPassantSquare;
        pieceLocations[lsb(enPassantSquare)] = NO_PIECE;
    }

    if (flag == CASTLING) {
        int rookFrom = from > to ? to - 1 : to + 2;
        int rookTo   = from > to ? to + 1 : to - 1;
        int rook     = pieceLocations[rookFrom];

        pieceLocations[rookFrom] = NO_PIECE;
        pieceLocations[rookTo  ] = rook;
        bitBoards[rook] ^= (1ULL << rookFrom) | (1ULL << rookTo);
    }

    if (movedPiece == PAWN) {
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

void Position::clearBoard() {
    for (int i = 0; i != 64; i++)
        pieceLocations[i] = NO_PIECE;

    memset(&bitBoards, 0ULL, bitBoards.size() * sizeof(typeof(bitBoards[0])));
    memset(&castlingRights, false, castlingRights.size() * sizeof(typeof(castlingRights[0])));
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

    if (castlingRights[0]) castling += "K";
    if (castlingRights[1]) castling += "Q";
    if (castlingRights[2]) castling += "k";
    if (castlingRights[3]) castling += "q";

    std::cout << "\n";
    std::cout << "50 move rule counter: " << plys50moveRule << "\n";
    std::cout << "side to move        : " << (sideToMove ? "w" : "b") << "\n";
    std::cout << "en passant square   : " << (enPassantSquare) << "\n";
    std::cout << "castling rights     : " << castling << "\n";
}

