#ifdef DATAGEN

#ifndef MOLYBDENUM_DATAGEN_H_2
#define MOLYBDENUM_DATAGEN_H_2

#include <string>
#include <random>
#include <fstream>

#include "../Constants.h"
#include "../search.h"

static const int VERIF_NODES = 5000;
static const int SEARCH_NODES = 25000;

static const int MAX_UNBALANCE = 150;

static const int WADJ_SCORE = 1000;
static const int WADJ_MOVECOUNT = 8;
static const int DADJ_SCORE = 0;
static const int DADJ_MOVECOUNT = 20;

void start(const std::string &filePrefix, u64 initialSeed, int batchSize = 16384);
void loop(SearchState &st, Position &pos, const std::string &filePrefix, std::mt19937 &random, int batchSize);
void createExit(SearchState &state, Position &pos, std::mt19937 &random);
bool verifyExit(SearchState &state, Position &pos);
int playGame(SearchState &state, Position &pos, std::ofstream &out);

#endif

#endif