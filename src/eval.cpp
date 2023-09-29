#include "Position.h"
#include "eval.h"
#include "PSQT.h"

int evaluate(Position &pos) {
    int gamePhase = std::min(pos.phase, maxPhase);
    return evalPSQT(pos, gamePhase) * (pos.sideToMove ? 1 : -1);
}

int evalPSQT(Position &pos, int gamePhase) {
    int mgEval = pos.psqtMG + (pos.sideToMove ? 1 : -1) * TempoMG;
    int egEval = pos.psqtEG + (pos.sideToMove ? 1 : -1) * TempoEG;
    return (mgEval * gamePhase + egEval * (maxPhase - gamePhase)) / maxPhase;
}

int getGamePhase(Position &pos) {
    int phase = 0;
    phase += __builtin_popcountll(pos.bitBoards[WHITE_KNIGHT] | pos.bitBoards[BLACK_KNIGHT]) * gamePhaseValues[KNIGHT];
    phase += __builtin_popcountll(pos.bitBoards[WHITE_BISHOP] | pos.bitBoards[BLACK_BISHOP]) * gamePhaseValues[BISHOP];
    phase += __builtin_popcountll(pos.bitBoards[WHITE_ROOK  ] | pos.bitBoards[BLACK_ROOK  ]) * gamePhaseValues[ROOK  ];
    phase += __builtin_popcountll(pos.bitBoards[WHITE_QUEEN ] | pos.bitBoards[BLACK_QUEEN ]) * gamePhaseValues[QUEEN ];
    return std::min(phase, maxPhase);
};