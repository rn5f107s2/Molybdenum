#ifndef MOLYBDENUM_PERFT_H
#define MOLYBDENUM_PERFT_H

#include "Position.h"
#include "Movegen.h"


template <bool BULK, bool ROOT>
u64 perft(int depth, Position &pos) {
    if constexpr (!BULK) {
        if (depth <= 0)
            return 1;
    }

    MoveList ml;
    u64 nodeCountThis;
    u64 nodeCount = 0;

    if constexpr (BULK && !ROOT) {
        if (depth <= 0)
            return 1;
    }

    u64 checkers = attackersTo<false, false>(lsb(pos.bitBoards[pos.sideToMove ? WHITE_KING : BLACK_KING]),getOccupied<WHITE>(pos) | getOccupied<BLACK>(pos), pos.sideToMove ? BLACK_PAWN : WHITE_PAWN, pos);

    generateMoves<false>(pos, ml, checkers);

    if constexpr (BULK && !ROOT) {
        if (depth == 1)
            return ml.length;
    }

    while (ml.currentIdx > 0) {
        pos.makeMove(ml.moves[ml.currentIdx - 1].move);

        nodeCountThis = perft<BULK, false>(depth - 1, pos);
        nodeCount += nodeCountThis;

        if constexpr (ROOT)
            std::cout << moveToString(ml.moves[ml.currentIdx - 1].move) << ": " << nodeCountThis << std::endl;

        pos.unmakeMove(ml.moves[--ml.currentIdx].move);
    }

    return nodeCount;
}

u64 startPerft(int depth, Position &pos, bool bulk) {
    if (depth <= 0)
        return 1;

    return bulk ? perft<true, true>(depth, pos) : perft<false, true>(depth, pos);
}




#endif //MOLYBDENUM_PERFT_H