#include <iostream>
#include <cstring>
#include <bitset>

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
    initAccumulator(bitBoards);
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
    toggleFeature<Off>(movedPiece, from);

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
        toggleFeature<Off>(capturedPawn, lsb(captureSquare));
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
        moveFeature(rook, rookFrom, rookTo);
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
        toggleFeature<Off>(capturedPiece, to);
        updateKey(capturedPiece, to, key);
        plys50moveRule = 0;
    }

    pieceLocations[to] = movedPiece;
    toggleFeature<On>(movedPiece, to);
    bitBoards[movedPiece] ^= 1ULL << to;
    updateKey(movedPiece, to, key);
    updateKey(key);
    keyHistory.push(key);
    sideToMove = !sideToMove;
    pushAccToStack();
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
        Piece rook     = pieceLocations[rookTo];

        pieceLocations[rookTo  ] = NO_PIECE;
        pieceLocations[rookFrom] = rook;
        bitBoards[rook] ^= (1ULL << rookFrom) | (1ULL << rookTo);
        moveFeature(rook, rookTo, rookFrom);
    }

    if (flag == ENPASSANT) {
        Piece capturedPawn = movingPiece == WHITE_PAWN ? BLACK_PAWN : WHITE_PAWN;
        u64 captureSquare = movePawn(enPassantSquare, sideToMove);
        bitBoards[capturedPawn] ^= captureSquare;
        pieceLocations[lsb(captureSquare)] = capturedPawn;
    }

    sideToMove = !sideToMove;
    popAccStack();
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

    int a;

    std::array<uint8_t, 32> mf = molyFormat(0, 32000, &a);

    std::cout << getData(mf) << "\n";
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

std::string Position::fen(const std::array<Piece, 64> *mailbox, int plys, int mc) {
    bool customBoard = mailbox != nullptr;
    std::array<Piece, 64> pieceLocs = !customBoard ? pieceLocations : *mailbox;  
    std::string fen;
    std::string castling;
    int emptyCounter = 0;
    int epSquare = enPassantSquare ? lsb(enPassantSquare) : 0;
    int plys50 = customBoard ? plys : plys50moveRule;
    int movCnt = customBoard ? mc   : movecount;

    for (int square = 63; square >= 0; square--) {
        if (pieceLocs[square] == NO_PIECE)
            emptyCounter++;
        else {
            if (emptyCounter)
                fen += std::to_string(emptyCounter);

            emptyCounter = 0;
            fen += pieceToChar(pieceLocs[square]);
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
    fen += (sideToMove || customBoard) ? "w " : "b ";

    if (castlingRights & WHITE_CASTLE_KINGSIDE  && !customBoard) castling += "K";
    if (castlingRights & WHITE_CASTLE_QUEENSIDE && !customBoard) castling += "Q";
    if (castlingRights & BLACK_CASTLE_KINGSIDE  && !customBoard) castling += "k";
    if (castlingRights & BLACK_CASTLE_QUEENSIDE && !customBoard) castling += "q";
    if (castling.empty()) castling += "-";

    fen += castling;
    fen += " ";

    if (enPassantSquare && !customBoard) {
        fen += char('a' + (fileOf(epSquare)));
        fen += char('1' + (rankOf(epSquare)));
        fen += " ";
    }else {
        fen += "- ";
    }

    fen += std::to_string(plys50);
    fen += " ";
    fen += std::to_string(movCnt);

    return fen;
}


// First 2 bits: wdl 10 = 1-0 01 == 1/2 - 1/2 00 = 0 - 1
// Next 7 bits: movecount
// Next 7 bits: 50mr 
// Next 2 bytes: stm relative eval
// total non board stuff 4 bytes

std::array<uint8_t, 32> Position::molyFormat(int wdlI, int evalI, int *lastIdx) {

    std::array<uint8_t, 32> out{};
    int outIdx = *lastIdx = 0;

    //eval should already bs stm relative
    int eval = std::clamp(evalI, -32768, 32767);

    uint8_t wdl = !sideToMove ? 2 - wdlI : wdlI;
    uint8_t mc = std::clamp(movecount, 0, 127);      //only keep track of movecounts up to 127
    uint8_t fiftyMR = std::clamp(plys50moveRule, 0, 127); //only keep track of 50mrs up to fifty, could be caused by faulty fens

    uint32_t nonBoardStuff = (eval << 16) | (fiftyMR << 9) | (mc << 2) | wdl;

    for (int i = 0; i != 4; i++)
        out[outIdx++] = (nonBoardStuff & (0xff << 8 * i)) >> 8 * i;

    std::array<std::array<int, 2>, 6> pieceCount{};
    std::array<int8_t, 32> pieceBits{};
    std::array<int8_t, 2> promotedSquare = {-1, -1};
    std::array<int, 6> maxPieces = {8, 2, 2, 2, 1, 1};
    //Queen is set as 1, additional queens are handled seperatly

    pieceCount[PAWN] = {popcount(getPieces(BLACK, PAWN)), popcount(getPieces(WHITE, PAWN))};

    // If a position has:
    //  - More than 8 pawns one color
    //  - More than 2 non pawn pieces of one color
    //  - or 2 Queens and atleast 7 Pawns of the same color
    // it can not be represented in Moly format

    int idx = 0;
    pieceBits.fill(-1);
    
    for (int pt = KING; pt >= PAWN; pt--) {
        for (Color c : {sideToMove, !sideToMove}) {
            u64 bb = getPieces(c, PieceType(pt));
            pieceCount[pt][c] = popcount(bb);

            if (pieceCount[pt][c] > (2 + 6 * (pt == PAWN)))
                return {};

            if (   pt == QUEEN 
                && pieceCount[pt][c] == 2 
                && pieceCount[PAWN][c] > 6)
                return {};

            if (   pt == QUEEN
                && pieceCount[pt][c] == 2)
                promotedSquare[c] = msb(bb) ^ (sideToMove ? 0 : 56);

            int neg1Count = 0;

            for (int i = 0; i != maxPieces[pt]; i++) {
                int square = bb ? (popLSB(bb) ^ (sideToMove ? 0 : 56)) : -1;
                neg1Count += square == -1;

                if (   pt == PAWN
                    && neg1Count == 2)
                    square = promotedSquare[c];

                pieceBits[idx++] = square;

                if (   (pt != PAWN || neg1Count == 2)
                    &&  square == -1)
                    break;
            }
        }
    }

    int usedBits = 0;
    uint8_t current = 0;

    for (int idx2 = 0; idx2 < idx; idx2++) {
        int usableBits = 8 - usedBits;
        int8_t usableMask = (1ULL << usableBits) - 1;
        int8_t nextMask   = ~usableMask & ~0x80;

        current |= (pieceBits[idx2] & usableMask) << usedBits;

        if (usableBits <= 7) {
            out[outIdx++] = current;
            current = 0;

            current |= (pieceBits[idx2] & nextMask) >> usableBits;
        }

        usedBits = (usedBits + 7) % 8;
    }

    out[outIdx++] = current;
    *lastIdx = outIdx;
    return out;
}

std::string Position::getData(const std::array<uint8_t, 32> &mf) {
    std::string out;
    std::array<Piece, 64> pieceLocs;
    pieceLocs.fill(NO_PIECE);

    out += "Eval: ";
    out +=  std::to_string(mf[2] | (mf[3] << 8)); 
    out += "\n";

    int wdlI = mf[0] & 0b11;
    std::string wdl = wdlI == 2 ? "1.0" : wdlI == 0 ? "0.0" : "0.5";

    out += "WDL : ";
    out += wdl;
    out += "\n";

    std::array<int, 6> maxPieces = {8, 2, 2, 2, 1, 1};

    int fiftyMR = mf[1] >> 1;
    int mc      = ((mf[0] | mf[1]) >> 2) & 0b1111111; 

    int inIdx = 4;
    int usedBits = 0;
    int data = 0;
    int next = 126;
    bool prevEmpty = false;

    std::cout << "\n";

    for (int pt = KING; pt >= PAWN; pt--) {
        for (Color c : {WHITE, BLACK}) {
            int count = 0;

            while (true) {
                count++;

                int sq;

                if (next == 126) { 
                    data |= int(mf[inIdx++]) << usedBits;

                    sq = data & 0b1111111;

                    usedBits = (usedBits + 8) % 7;

                    if (!usedBits)
                        next = (data >> 7) & 0b1111111;

                } else {
                    sq = next;
                    next = 126;
                }

                data >>= 7;

                if (    sq != 127
                    && !prevEmpty) {
                    pieceLocs[sq] = makePiece(pt, c);
                }

                if (   prevEmpty 
                    && sq != 127) {

                    pieceLocs[sq] = makePiece(QUEEN, c);
                }

                if (   (sq == 127 && (pt != PAWN || prevEmpty))
                    || count == maxPieces[pt])
                    break;

                prevEmpty = sq == 127;
            }
        }   
    }

    out += fen(&pieceLocs, fiftyMR, mc);

    return out;
}
