#ifndef MOLYBDENUM_MCTS_H
#define MOLYBDENUM_MCTS_H

#include "Position.h"
#include "Move.h"
#include "timemanagement.h"

#include <cstdint>

class NodePool;

#pragma pack(push, 1)

class Node {
public:
    Node* children;

    Move     move   = NO_MOVE;
    uint8_t  cCount = 0;
    uint32_t visits = 0;
    float    result = 0.0f;

    float search(Position &pos, NodePool &pool, bool root = false);
    float rollout(Position &pos);
    Node* select(Position &pos, bool root = false);
    void  expand(Position &pos, NodePool &pool);
    float backpropagate();
};

#pragma pack(pop)

class NodePool {
public:
    Node* memory;
    int   sizeMB;
    int   limit;
    int   currIdx;

    NodePool(int sizeMB);
    Node* allocate(int nNodes);
    void clear();
    void freeMemory();
    void resize(int newMB);
};

float uct(uint32_t pVisits, uint32_t visits, float score, Move move, Position &pos, bool root = false);
float policy(Position &pos, Move move, bool root = false);
void rootSearch(Position &pos, SearchTime &st);

#endif