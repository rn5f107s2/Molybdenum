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
    float    policy = 0.0f;

    float search(Position &pos, NodePool &pool, int ply);
    float rollout(Position &pos);
    Node* select(bool root);
    void  expand(Position &pos, NodePool &pool, int ply);
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

float uct(uint32_t pVisits, uint32_t visits, float score, float policy, bool root, float pq, float bestq);
float fpu(float pq, float bq);
void rootSearch(Position &pos, SearchTime &st);

#endif