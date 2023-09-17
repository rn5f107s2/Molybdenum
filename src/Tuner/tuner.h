#ifndef MOLYBDENUM_TUNER_H
#define MOLYBDENUM_TUNER_H

#include "../Position.h"
#include "../PSQT.h"
#include "../eval.h"

//#define tuneDef
#ifdef tuneDef

void tune(Position &pos, const std::string& filename);
void calcK(Position &pos, const std::string& filename);
int tuneQ(int alpha, int beta, Position &pos);
int tuneEval(Position &pos);
double getWinProbability(double score);
double getErrorAllFens(const std::string& filename, Position &pos);
double getMeanSquaredError(int score, double correct);
double getErrorFen(const std::string& fen, Position &pos);
int tweakValues(int index, int amount);
void printPSQB();

static double K = 1.197;
static const std::array<int, 4> changeValues = {14, 7, 3, 1};
const int numParams = 780;
static std::array<int, 6> PieceValuesMGTune = PieceValuesMG;
static std::array<int, 6> PieceValuesEGTune = PieceValuesEG;
static std::array<std::array<std::array<int, 64>, 6>, 2> PieceSquareBonusesTune = {PieceSquareBonusesMG, PieceSquareBonusesEG};
static std::array<int, 6> gamePhaseValuesTune = gamePhaseValues;

#endif
#endif //MOLYBDENUM_TUNER_H
