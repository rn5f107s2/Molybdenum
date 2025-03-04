#ifndef MOLYBDENUM_MCTS_H
#define MOLYBDENUM_MCTS_H

#include "Position.h"
#include "Move.h"
#include "timemanagement.h"
#include "Movegen.h"

#include <cstdint>

class NodePool;

#pragma pack(push, 1)

class Node {
public:
    Node* children = nullptr;

    Move     move   = NO_MOVE;
    uint8_t  cCount = 0;
    uint32_t visits = 0;
    float    result = 0.0f;
    float    policy = 0.0f;

    float search(Position &pos, NodePool &pool, int ply);
    float rollout(Position &pos);
    Node* select(bool root);
    Node* find(Move move);
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

    NodePool();
    NodePool(int sizeMB);
    Node* allocate(int nNodes);
    void clear();
    void freeMemory();
    void resize(int newMB);
    bool isInPool(Node* adress);
    bool isFull();
    void cleanChildren();
};

struct MCTSState {
    NodePool* activeHalf;
    NodePool  left, right;
    Position* prevPos;
    Node      root;
    Move      lastBest = NO_MOVE;

    MCTSState(int size) {
        left  = NodePool(size / 2);
        right = NodePool(size / 2);

        activeHalf = &right;
        prevPos    = new Position();
    }

    ~MCTSState() {
        left .freeMemory();
        right.freeMemory();
    }

    void fallback() {
        root = Node();
        root.visits = 1;
    }

    void initRoot(Position &pos) {
        if (!lastBest)
            return fallback();

        Node* state = root.find(lastBest);
        prevPos->makeMove(lastBest);

        fallback();

        u64 ksq = pos.getPieces(pos.sideToMove, KING);
        u64 checkers = attackersTo<false, false>(lsb(ksq),pos.getOccupied(), pos.sideToMove ? BLACK_PAWN : WHITE_PAWN, pos);

        MoveList ml; generateMoves<false>(*prevPos, ml, checkers);

        for (int i = 0; i < ml.length; i++) {
            prevPos->makeMove(ml.moves[i].move);

            if (prevPos->key() == pos.key()) {
                state = state->find(ml.moves[i].move);

                if (state) {
                    memcpy(&root, state, sizeof(Node));

                    float sum = 0.0f;

                    if (state->children) {
                        for (int i = 0; i < state->cCount; i++)
                            sum += state->children[i].policy / 3.88f;

                        for (int i = 0; i < state->cCount; i++)
                            state->children[i].policy = (state->children[i].policy / 3.88f) / sum;
                    }
                }
            }

            prevPos->unmakeMove(ml.moves[i].move);
        }

        prevPos->unmakeMove(lastBest);
    }
};

float uct(uint32_t pVisits, uint32_t visits, float score, float policy, bool root, float pq, float bestq);
float fpu(float pq, float bq);
void rootSearch(MCTSState &state, Position &pos, SearchTime &st);

#endif