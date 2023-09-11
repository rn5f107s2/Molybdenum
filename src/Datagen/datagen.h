#ifndef MOLYBDENUM_DATAGEN_H
#define MOLYBDENUM_DATAGEN_H

#include "../search.h"
#include "../Position.h"

//#define datagen
#ifdef datagen

struct gameInfo{
    std::string currentPos = "position startpos ";
    Stack<std::string> fens;
    int timeW;
    int timeB;
    int increment;
};

void datagenLoop(Position &pos, int gamesToPlay, int timeControl, const std::string& outputFileName, int exitDepth = 8, int reportingFrequency = 50);
int simpleQ(int alpha, int beta, Position &pos, SearchInfo &si, int depth = 0);
int pieceEval(Position &pos);
bool okExit(Position &pos);
void createExit(Position &pos, gameInfo &gi, int exitDepth = 8);
std::string playGame(gameInfo &gi, Position &pos);
int startSearching(SearchInfo &si, Position &pos);

#endif
#endif //MOLYBDENUM_DATAGEN_H
