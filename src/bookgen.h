#pragma once

#include "Position.h"
#include "Movegen.h"

#include <string>

struct Node {
    bool visited = false;

    MoveList ml;
    Node* children = nullptr;

    void expand(Position& pos);
    Node* findChild(Move m);
};

struct FilterSettings {
    double randomFilter = 0.90;
    
    int filterDepth = 15;
    int filterNodes = 5000000;

    int filterLowerBound = 50;
    int filterUpperBound = 75;
};


void generate(Position& pos, const std::string& bookName, int positions);