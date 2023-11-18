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