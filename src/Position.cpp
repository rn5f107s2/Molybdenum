#include <iostream>
#include <cstring>
#include "Position.h"
#include "Transpositiontable.h"
#include "PSQT.h"
#include "nnue.h"
#include "Movegen.h"

void Position::setBoard(std::string fen) {
    clearBoard();

    u64 bitToSet = 1ULL << 63;
    std::string plys50mr;
    std::string epSquare;
    std::string moveCount;
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

        if (spaceCount == 5) {
            moveCount += currentChar;
            continue;
        }

        if (charAsInt < 10) {
            bitToSet = bitToSet >> charAsInt;
            continue;
        }

        if (spaceCount)
            continue;

        Piece piece = charIntToPiece(charAsInt);
        phase += gamePhaseValues[typeOf(piece)];
        bitBoards[piece] ^= bitToSet;
        pieceLocations[lsb(bitToSet)] = piece;
        bitToSet = bitToSet >> 1;
    }

    enPassantSquare = !epSquare.empty()  ? stringToSquare(epSquare)         : 0ULL;
    plys50moveRule  = !plys50mr.empty()  ? stringRoRule50(plys50mr)  : 0;
    movecount       = !moveCount.empty() ? stringRoRule50(moveCount) : 0;
    keyHistory.push(positionToKey(bitBoards, castlingRights, enPassantSquare, sideToMove));
    net.initAccumulator(bitBoards);
}

Move Position::fromToToMove(int from, int to, int promotionPiece, int flag) {
    int movingPiece = pieceLocations[from];
    bool captured = pieceLocations[to] != NO_PIECE;

    if (typeOf(movingPiece) == PAWN && !flag) {
        if (fileOf(from) != fileOf(to) && !captured)
            flag = ENPASSANT;
    }

    if (typeOf(movingPiece) == KING && std::abs(fileOf(from) - fileOf(to)) == 2 && !flag)
        flag = CASTLING;

    movecount++;
    return createMove(from, to, promotionPiece, flag);
}

void Position::makeMove(Move move) {
    int from = extract<FROM>(move);
    int to   = extract<TO>(move);
    int flag = extract<FLAG>(move);
    Piece movedPiece    = pieceLocations[from];
    Piece capturedPiece = pieceLocations[to];
    u64 key             = keyHistory.top();

    plys50mrHistory.push(plys50moveRule);
    castlingHistory.push(castlingRights);
    capturedHistory.push(capturedPiece);
    enPassantHistory.push(enPassantSquare);

    if (enPassantSquare)
        updateKey(fileOf(lsb(enPassantSquare)), key);

    plys50moveRule++;
    pieceLocations[from] = NO_PIECE;
    bitBoards[movedPiece] ^= 1ULL << from;
    updateKey(movedPiece,  from, key);
    //net.toggleFeature<Off>(movedPiece, from);

    if (flag == PROMOTION) {
        movedPiece = makePromoPiece(extract<PROMOTIONTYPE>(move), sideToMove);
        phase += gamePhaseValues[typeOf(movedPiece)];
        plys50moveRule = 0;
    }

    if (flag == ENPASSANT) {
        int capturedPawn = movedPiece == WHITE_PAWN ? BLACK_PAWN : WHITE_PAWN;
        u64 captureSquare = movePawn(enPassantSquare, !sideToMove);
        bitBoards[capturedPawn] ^= captureSquare;
        pieceLocations[lsb(captureSquare)] = NO_PIECE;
        //net.toggleFeature<Off>(capturedPawn, lsb(captureSquare));
    }

    if (flag == CASTLING) {
        int rookFrom = from > to ? to - 1 : to + 2;
        int rookTo   = from > to ? to + 1 : to - 1;
        Piece rook     = pieceLocations[rookFrom];

        pieceLocations[rookFrom] = NO_PIECE;
        pieceLocations[rookTo  ] = rook;
        bitBoards[rook] ^= (1ULL << rookFrom) | (1ULL << rookTo);
        updateKey(rook, rookFrom, key);
        updateKey(rook, rookTo, key);
        //net.moveFeature(rook, rookFrom, rookTo);
    }

    enPassantSquare = 0ULL;

    if (typeOf(movedPiece) == PAWN) {
        if ((from ^ to) == 16) {
            enPassantSquare = 1ULL << (to - (from > to ? -8 : 8));
            updateKey(fileOf(from), key);
        }

        plys50moveRule = 0;
    }

    updateKeyCastling((castlingMask[from] | castlingMask[to]) & castlingRights, key);
    castlingRights &= ~(castlingMask[from] | castlingMask[to]);

    if (capturedPiece != NO_PIECE) {
        bitBoards[capturedPiece] ^= 1ULL << to;
        phase -= gamePhaseValues[typeOf(capturedPiece)];
        //net.toggleFeature<Off>(capturedPiece, to);
        updateKey(capturedPiece, to, key);
        plys50moveRule = 0;
    }

    pieceLocations[to] = movedPiece;
    //net.toggleFeature<On>(movedPiece, to);
    bitBoards[movedPiece] ^= 1ULL << to;
    updateKey(movedPiece, to, key);
    updateKey(key);
    keyHistory.push(key);
    sideToMove = !sideToMove;
    //net.pushAccToStack();
}

void Position::unmakeMove(Move move) {
    int from   = extract<FROM>(move);
    int to     = extract<TO  >(move);
    int flag = extract<FLAG>(move);
    Piece movingPiece = pieceLocations[to];
    Piece capturedPiece = capturedHistory.pop();

    plys50moveRule = plys50mrHistory.pop();
    castlingRights = castlingHistory.pop();
    enPassantSquare = enPassantHistory.pop();
    keyHistory.pop();

    pieceLocations[to] = capturedPiece;
    bitBoards[movingPiece] ^= 1ULL << to;
    bitBoards[capturedPiece] ^= 1ULL << to;

    if (capturedPiece != NO_PIECE)
        phase += gamePhaseValues[typeOf(capturedPiece)];

    if (flag == PROMOTION) {
        phase -= gamePhaseValues[typeOf(movingPiece)];
        movingPiece = sideToMove ? BLACK_PAWN : WHITE_PAWN;
    }

    pieceLocations[from] = movingPiece;
    bitBoards[movingPiece] ^= 1ULL << from;

    if (flag == CASTLING) {
        int rookFrom = from > to ? to - 1 : to + 2;
        int rookTo   = from > to ? to + 1 : to - 1;
        Piece rook   = pieceLocations[rookTo];

        pieceLocations[rookTo  ] = NO_PIECE;
        pieceLocations[rookFrom] = rook;
        bitBoards[rook] ^= (1ULL << rookFrom) | (1ULL << rookTo);
    }

    if (flag == ENPASSANT) {
        Piece capturedPawn = movingPiece == WHITE_PAWN ? BLACK_PAWN : WHITE_PAWN;
        u64 captureSquare = movePawn(enPassantSquare, sideToMove);
        bitBoards[capturedPawn] ^= captureSquare;
        pieceLocations[lsb(captureSquare)] = capturedPawn;
    }

    sideToMove = !sideToMove;
    //net.popAccStack();
}

void Position::clearBoard() {
    for (int i = 0; i != 64; i++)
        pieceLocations[i] = NO_PIECE;

    memset(&bitBoards, 0ULL, bitBoards.size() * sizeof(decltype(bitBoards)::value_type));

    phase = 0;
    enPassantHistory.clear();
    castlingHistory.clear();
    capturedHistory.clear();
    plys50mrHistory.clear();
    keyHistory.clear();
    castlingRights = 0;
    plys50moveRule = 0;
    enPassantSquare = 0;
    movecount = 0;
    sideToMove = WHITE;
}

void Position::printBoard() {
    std::string board;
    std::string castling;

    for (int square = 63; square != -1; square--) {
        for (int piece = 0; piece != 12; piece++) {
            if (bitBoards[piece] & (1ULL << square)) {
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
    std::cout << "key                 : " << key() << "\n";
    std::cout << "FEN                 : " << fen() << "\n";
}

u64 Position::key() {
    return keyHistory.top();
}

bool Position::hasRepeated(int plysInSearch) {
    int currentIdx = keyHistory.getSize() - 1;
    int repetitions = 0;
    u64 currentKey = keyHistory.top();

    for (int idx = currentIdx - 2; idx >= currentIdx - plys50moveRule; idx -= 2) {
        if(keyHistory.at(idx) == currentKey) {
            repetitions++;

            if (repetitions > static_cast<int>((currentIdx - idx) > plysInSearch))
                return true;
        }
    }

    return false;
}

void Position::makeNullMove() {
    enPassantHistory.push(enPassantSquare);
    u64 key = keyHistory.top();

    if (enPassantSquare)
        updateKey(fileOf(lsb(enPassantSquare)), key);

    updateKey(key);
    keyHistory.push(key);
    enPassantSquare = 0ULL;
    plys50moveRule++;
    sideToMove = !sideToMove;
}

void Position::unmakeNullMove() {
    enPassantSquare = enPassantHistory.pop();
    keyHistory.pop();
    plys50moveRule--;
    sideToMove = !sideToMove;
}

std::string Position::fen() {
    std::string fen;
    std::string castling;
    int emptyCounter = 0;
    int epSquare = enPassantSquare ? lsb(enPassantSquare) : 0;

    for (int square = 63; square >= 0; square--) {
        if (pieceLocations[square] == NO_PIECE)
            emptyCounter++;
        else {
            if (emptyCounter)
                fen += std::to_string(emptyCounter);

            emptyCounter = 0;
            fen += pieceToChar(pieceLocations[square]);
        }

        if (square % 8 == 0) {
            if (emptyCounter)
                fen += std::to_string(emptyCounter);

            emptyCounter = 0;

            if (square != 0)
                fen += "/";
        }
    }

    fen += " ";
    fen += sideToMove ? "w " : "b ";

    if (castlingRights & WHITE_CASTLE_KINGSIDE ) castling += "K";
    if (castlingRights & WHITE_CASTLE_QUEENSIDE) castling += "Q";
    if (castlingRights & BLACK_CASTLE_KINGSIDE ) castling += "k";
    if (castlingRights & BLACK_CASTLE_QUEENSIDE) castling += "q";
    if (castling.empty()) castling += "-";

    fen += castling;
    fen += " ";

    if (enPassantSquare) {
        fen += char('a' + (fileOf(epSquare)));
        fen += char('1' + (rankOf(epSquare)));
        fen += " ";
    }else {
        fen += "- ";
    }

    fen += std::to_string(plys50moveRule);
    fen += " ";
    fen += std::to_string(movecount);

    return fen;
}
