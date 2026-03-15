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


void generate(Position& pos, const std::string& bookName, int positions);