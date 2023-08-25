#ifndef MOLYBDENUM_MOVEGEN_H
#define MOLYBDENUM_MOVEGEN_H

#include "BitStuff.h"
#include "MagicBitboards.h"
#include "Move.h"
#include "Position.h"

std::array<std::array<u64, 64>, 64> initMaskBB();
inline std::array<std::array<u64, 64>, 64> initExtendedMaskBB();

constexpr std::array<std::array<u64, 64>, 2> pawnMasks = initPawnMasks();
constexpr std::array<u64, 64> knightMasks = initKnightMasks();
constexpr std::array<u64, 64> kingMasks = initKingMasks();
const std::array<std::array<u64, 64>, 64> masksBBs = initMaskBB();
const std::array<std::array<u64, 512>, 64> bishopLookUpTable = initBishopLookUpTable();
const std::array<std::array<u64, 4096>, 64> rookUpTable = initRookUpTable();
const std::array<std::array<u64, 64>, 64> extendedMasksBBs = initExtendedMaskBB();

struct ScoredMove {
    Move move;
    int score;
};

struct MovegenVariables {
    u64 white;
    u64 black;
    u64 occupied;
    u64 empty;
    u64 stm;
    u64 enemy;
    u64 enemyOrEmpty;
    u64 checkMask;
    u64 pinnedPieces;
    u64 ksSquares;
    u64 qsSquares;
    int castlingKs;
    int castlingQs;
    int pawnIdx;
    int oppPawnIdx;
    int kingSquare;
};

struct MoveList {
    std::array<ScoredMove, 218> moves = {};
    int currentIdx = 0;
    int length = 0;
};

template<PieceType TYPE> inline
u64 getAttacks(int square, u64 blockers = 0ULL, bool white = true) {
    [[maybe_unused]] u64 squareL = 1ULL << square;
    switch (TYPE) {
        case PAWN:
            return pawnMasks[white][square];
        case KNIGHT:
            return knightMasks[square];
        case BISHOP:
            return bishopLookUpTable[square][((blockers & bishopMask[square]) * bishopMagics[square]) >> bishopShifts[square]];
        case ROOK:
            return rookUpTable[square][((blockers & rookMask[square]) * rookMagics[square]) >> rookShifts[square]];
        case QUEEN:
            return getAttacks<ROOK>(square, blockers) | getAttacks<BISHOP>(square, blockers);
        default:
            return kingMasks[square];
    }
}

inline std::array<std::array<u64, 64>, 64> initMaskBB() {
    std::array<std::array<u64, 64>, 64> masksBB = {{{0ULL}}};

    for(int square = 0; square != 64; square++) {
        for(int square2 = 0; square2 != 64; square2++) {
            u64 squareL  = 1ULL << square;
            u64 square2L = 1ULL << square2;
            u64 blockers = squareL | square2L;
            u64 mask = 0;

            if (getAttacks<BISHOP>(square, 0ULL) & square2L)
                mask = getAttacks<BISHOP>(square, blockers) & getAttacks<BISHOP>(square2, blockers);
            else if  (getAttacks<ROOK>(square, 0ULL) & square2L)
                mask = getAttacks<ROOK>(square, blockers) & getAttacks<ROOK>(square2, blockers);

            mask |= square2L;

            masksBB[square][square2] = mask;
        }
    }

    return masksBB;
}

inline std::array<std::array<u64, 64>, 64> initExtendedMaskBB() {
    std::array<std::array<u64, 64>, 64> masksBB = {{{0ULL}}};

    for(int square = 0; square != 64; square++) {
        for(int square2 = 0; square2 != 64; square2++) {
            u64 square2L = 1ULL << square2;
            u64 mask = 0;

            if (getAttacks<BISHOP>(square, 0ULL) & square2L)
                mask = getAttacks<BISHOP>(square) & getAttacks<BISHOP>(square2);
            else if  (getAttacks<ROOK>(square, 0ULL) & square2L)
                mask = getAttacks<ROOK>(square) & getAttacks<ROOK>(square2);


            masksBB[square][square2] = mask;
        }
    }

    return masksBB;
}

inline u64 generatePinnedPieces(Position &pos, MovegenVariables &mv) {
    u64 pinnedPieces = 0ULL;
    u64 pinnedPiece  = 0ULL;
    u64 pieces = mv.occupied & ~(pos.bitBoards[mv.pawnIdx + KING]);
    u64 possiblePinners  = getAttacks<BISHOP>(mv.kingSquare) & (pos.bitBoards[mv.oppPawnIdx + BISHOP] | pos.bitBoards[mv.oppPawnIdx + QUEEN]);
    possiblePinners     |= getAttacks<ROOK  >(mv.kingSquare) & (pos.bitBoards[mv.oppPawnIdx + ROOK] | pos.bitBoards[mv.oppPawnIdx + QUEEN]);

    while (possiblePinners) {
        int pinnerSquare = popLSB(possiblePinners);
        if (!multipleBits(pinnedPiece = masksBBs[pinnerSquare][mv.kingSquare] & pieces))
            pinnedPieces |= pinnedPiece;
    }

    return pinnedPieces;
}



template<bool INCLUDEKING, bool SLIDERSONLY> inline
u64 attackersTo(int square, u64 blockers, int pawnIdx, Position &pos) {
    u64 attacks = 0ULL;

    if constexpr (!SLIDERSONLY) {
        attacks |= getAttacks<PAWN>(square, blockers, pawnIdx == 6) & pos.bitBoards[pawnIdx];
        attacks |= getAttacks<KNIGHT>(square) & pos.bitBoards[pawnIdx + KNIGHT];
    }

    attacks |= getAttacks<BISHOP>(square, blockers) & pos.bitBoards[pawnIdx + BISHOP];
    attacks |= getAttacks<ROOK>(square, blockers)   & pos.bitBoards[pawnIdx + ROOK  ];
    attacks |= getAttacks<QUEEN>(square, blockers)  & pos.bitBoards[pawnIdx + QUEEN ];

    if constexpr (INCLUDEKING && !SLIDERSONLY) {
        attacks |= getAttacks<KING>(square) & pos.bitBoards[pawnIdx + KING];
    }

    return attacks;
}

inline void pushBack(MoveList &ml, Move move) {
    ScoredMove m = {};
    m.move = move;
    ml.moves[ml.currentIdx] = m;
    ml.length++;
    ml.currentIdx++;
}

template<bool WHITE> inline
u64 getOccupied(Position &pos) {
    u64 occ = 0;
    int start = 0 + (!WHITE * 6);
    for (int i = start; i != start + 6; i++)
        occ |= pos.bitBoards[i];

    return occ;
}

template<bool ISKING, MoveFlags FLAG, bool CAPTURESONLY>
inline void pushTargetsToMoveList(int fromSquare, u64 possibleTargets, MoveList &ml, MovegenVariables &mv, Position &pos) {
    [[maybe_unused]] u64 blockers = mv.occupied ^ (1ULL << fromSquare);
    while (possibleTargets) {
        int toSquare = popLSB(possibleTargets);

        if constexpr (ISKING) {
            if (attackersTo<true, false>(toSquare, blockers, mv.oppPawnIdx, pos))
                continue;
        }

        if constexpr (FLAG == ENPASSANT) {
            u64 capturedPawn = createSquare(7 - fileOf(toSquare), rankOf(fromSquare));
            if (attackersTo<false, true>(mv.kingSquare, (blockers ^ capturedPawn) | (1ULL << toSquare), mv.oppPawnIdx, pos)) {
                return;
            }
        }


        Move move = createMove(fromSquare, toSquare, PROMO_QUEEN, FLAG);
        pushBack(ml, move);

        if constexpr (FLAG == PROMOTION && !CAPTURESONLY)
            for(int promoPiece = PROMO_BISHOP; promoPiece != PROMO_QUEEN + 1; promoPiece++) {
                move = createMove(fromSquare, toSquare, promoPiece, FLAG);
                pushBack(ml, move);
            }
    }
}

template<PieceType PIECE, bool CAPTURESONLY>
inline void generateMoves(Position &pos ,MovegenVariables &mv, MoveList &ml) {
    int piece = mv.pawnIdx + PIECE;
    u64 pieceBB = pos.bitBoards[piece];
    u64 target = CAPTURESONLY ? mv.enemy : mv.enemyOrEmpty;

    while (pieceBB) {
        int fromSquare = popLSB(pieceBB);
        u64 possibleTargets = getAttacks<PIECE>(fromSquare, mv.occupied);
        possibleTargets &= target;

        if constexpr (PIECE != KING) {
            possibleTargets &= mv.checkMask;
            if ((1ULL << fromSquare) & mv.pinnedPieces)
                possibleTargets &= extendedMasksBBs[fromSquare][mv.kingSquare];
        }

        pushTargetsToMoveList<PIECE == KING, NORMAL, CAPTURESONLY>(fromSquare, possibleTargets, ml, mv, pos);
    }
}

template<bool CAPTURESONLY>
inline void generatePawnMoves(Position &pos ,MovegenVariables &mv, MoveList &ml) {
    u64 pieceBB = pos.bitBoards[mv.pawnIdx];

    while (pieceBB) {
        int fromSquare = popLSB(pieceBB);
        u64 fromSquareL = 1ULL << fromSquare;
        u64 possibleTargets  = 0;


        if constexpr (!CAPTURESONLY) {
            possibleTargets |= movePawn(fromSquareL, pos.sideToMove) & mv.empty;

            if (rankOf(fromSquare) == (pos.sideToMove ? 1 : 6))
                possibleTargets |= movePawn(possibleTargets, pos.sideToMove) & mv.empty;
        }

        possibleTargets |= getAttacks<PAWN>(fromSquare, 0ULL, pos.sideToMove) & mv.enemy;
        possibleTargets &= mv.checkMask;

        if (fromSquareL & mv.pinnedPieces)
            possibleTargets &= extendedMasksBBs[fromSquare][mv.kingSquare];

        if (fromSquareL & (pos.sideToMove ? RANK7 : RANK2))
            pushTargetsToMoveList<false, PROMOTION, CAPTURESONLY>(fromSquare, possibleTargets, ml, mv, pos);
        else
            pushTargetsToMoveList<false, NORMAL, CAPTURESONLY>(fromSquare, possibleTargets, ml, mv, pos);
    }
}

inline void generateEnPassant(Position &pos, MovegenVariables &mv, MoveList &ml) {
    if (!pos.enPassantSquare)
        return;

    u64 possiblePawns = getAttacks<PAWN>(lsb(pos.enPassantSquare), 0ULL,!pos.sideToMove) & pos.bitBoards[mv.pawnIdx];

    while (possiblePawns)
        pushTargetsToMoveList<false, ENPASSANT, false>(popLSB(possiblePawns), pos.enPassantSquare, ml, mv, pos);
}

inline void generateCastling(Position &pos, MovegenVariables &mv, MoveList &ml) {
    u64 possibleTargets = 0ULL;

    if (pos.castlingRights & mv.castlingKs && !(mv.occupied & mv.ksSquares)) {
        if (!attackersTo<true, false>(mv.kingSquare - 1, mv.occupied, mv.oppPawnIdx, pos))
            possibleTargets |= pos.bitBoards[mv.pawnIdx + KING] >> 2;
    }

    if (pos.castlingRights & mv.castlingQs && !(mv.occupied & mv.qsSquares)) {
        if (!attackersTo<true, false>(mv.kingSquare + 1, mv.occupied, mv.oppPawnIdx, pos))
            possibleTargets |= pos.bitBoards[mv.pawnIdx + KING] << 2;
    }

    pushTargetsToMoveList<true, CASTLING, false>(mv.kingSquare, possibleTargets, ml, mv, pos);
}

template<bool CAPTURESONLY>
inline bool generateMoves(Position &pos, MoveList &ml) {
    MovegenVariables mv = {};

    mv.white = getOccupied<WHITE>(pos);
    mv.black = getOccupied<BLACK>(pos);
    mv.occupied = mv.white | mv.black;
    mv.empty = ~mv.occupied;
    mv.stm = pos.sideToMove ? mv.white : mv.black;
    mv.enemy = pos.sideToMove ? mv.black : mv.white;
    mv.castlingKs = pos.sideToMove ? WHITE_CASTLE_KINGSIDE : BLACK_CASTLE_KINGSIDE;
    mv.castlingQs = pos.sideToMove ? WHITE_CASTLE_QUEENSIDE : BLACK_CASTLE_QUEENSIDE;
    mv.ksSquares  = pos.sideToMove ? WHITE_CASTLING_SQUARES_KINGSIDE : BLACK_CASTLING_SQUARES_KINGSIDE;
    mv.qsSquares  = pos.sideToMove ? WHITE_CASTLING_SQUARES_QUEENSIDE : BLACK_CASTLING_SQUARES_QUEENSIDE;
    mv.enemyOrEmpty = mv.enemy | mv.empty;
    mv.pawnIdx = pos.sideToMove ? WHITE_PAWN : BLACK_PAWN;
    mv.oppPawnIdx = pos.sideToMove ? BLACK_PAWN : WHITE_PAWN;
    mv.kingSquare = lsb(pos.bitBoards[mv.pawnIdx + KING]);
    mv.pinnedPieces = generatePinnedPieces(pos, mv);
    bool doubleCheck;

    u64 checkers = attackersTo<false, false>(mv.kingSquare, mv.occupied, mv.oppPawnIdx, pos);
    mv.checkMask = checkers ? masksBBs[mv.kingSquare][lsb(checkers)] : -1ULL;
    doubleCheck = checkers && multipleBits(checkers);

    if (!doubleCheck) {
        generatePawnMoves<CAPTURESONLY>(pos, mv, ml);
        generateMoves<KNIGHT, CAPTURESONLY>(pos, mv, ml);
        generateMoves<BISHOP, CAPTURESONLY>(pos, mv, ml);
        generateMoves<ROOK, CAPTURESONLY>(pos, mv, ml);
        generateMoves<QUEEN, CAPTURESONLY>(pos, mv, ml);
    }

    generateMoves<KING, CAPTURESONLY>(pos, mv, ml);
    generateEnPassant(pos, mv, ml);

    if (!checkers)
        generateCastling(pos, mv, ml);

    return checkers;
}

#endif //MOLYBDENUM_MOVEGEN_H
