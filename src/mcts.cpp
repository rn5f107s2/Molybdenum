#include "mcts.h"
#include "Position.h"
#include "eval.h"
#include "Movegen.h"
#include "timemanagement.h"
#include "search.h"

#include <stdlib.h>
#include <iostream>
#include <cmath>

void rootSearch(Position &pos, SearchTime &st) {
    Node       root;
    NodePool   pool(256);
    SearchInfo si;
    si.clear();
    si.st = st;

    while ((int((++si.nodeCount) + 218) < pool.limit) && ((si.nodeCount & 511) || !stop<Soft>(st, si)))
        root.search(pos, pool);

    benchNodes += si.nodeCount;

    float bestRes;
    Move  bestMove;

    for (int j = 0; j < root.cCount; j++) {
        float thisRes = root.children[j].result / root.children[j].visits;

        std::cout << "info string " << thisRes << " " << moveToString(root.children[j].move) << std::endl;

        if (thisRes <= bestRes)
            continue;

        bestRes  = thisRes;
        bestMove = root.children[j].move;
    }

    std::cout << "info depth 1 nodes " << si.nodeCount << " score cp " << (bestRes - 0.5f) * 1200 << std::endl;
    std::cout << "bestmove " << moveToString(bestMove) << std::endl;

    pool.freeMemory();
}

NodePool::NodePool(int mb) : sizeMB(mb), 
                             limit((mb * 1024 * 1024) / sizeof(Node)),
                             currIdx(0) {
    memory = reinterpret_cast<Node*>(malloc(limit * sizeof(Node)));
    std::cout << limit << std::endl;
}

Node* NodePool::allocate(int nNodes) {
    Node* ret = &memory[currIdx];
    currIdx  += nNodes;

    return currIdx > limit ? nullptr : ret;
}

void NodePool::clear() {
    free(memory);
    memory = reinterpret_cast<Node*>(malloc(limit * sizeof(Node)));
}

void NodePool::freeMemory() {
    free(memory);
}

void NodePool::resize(int newMB) {
    free(memory);

    sizeMB  = newMB;
    limit   = (newMB * 1024 * 1024) / sizeof(Node);
    currIdx = 0;

    memory = reinterpret_cast<Node*>(malloc(limit * sizeof(Node)));
}

float uct(uint32_t pVisits, uint32_t visits, float score) {
    const float c = 1.414213562373095048801688f;

    if (!visits)
        return std::numeric_limits<float>::max();

    return (score / visits) + c * std::sqrt(std::log(pVisits) / visits);
}

float Node::search(Position &pos, NodePool &pool) {
    u64 ksq      = pos.getPieces(pos.sideToMove, KING);
    u64 checkers = attackersTo<false, false>(lsb(ksq),pos.getOccupied(), pos.sideToMove ? BLACK_PAWN : WHITE_PAWN, pos);

    if (!visits && !checkers)
        return rollout(pos);

    if (visits <= 1)
        expand(pos, pool);

    if (!cCount) {
        visits = std::numeric_limits<uint32_t>::max();
        result = (checkers ? 1.0f : 0.5f) * visits;

        return 1 - (checkers ? 1.0f : 0.5f);
    }

    Node* toSearch = select();
    pos.makeMove(toSearch->move);

    visits++;
    float res = toSearch->search(pos, pool);
    result += res;

    pos.unmakeMove(toSearch->move);
    return 1 - res;
}

void Node::expand(Position &pos, NodePool &pool) {
    MoveList ml;

    u64 ksq = pos.getPieces(pos.sideToMove, KING);
    u64 checkers = attackersTo<false, false>(lsb(ksq),pos.getOccupied(), pos.sideToMove ? BLACK_PAWN : WHITE_PAWN, pos);

    generateMoves<false>(pos, ml, checkers);
    cCount   = ml.length;
    children = pool.allocate(cCount);

    for (int i = 0; i < ml.length; i++)
        children[i].move = ml.moves[i].move;
}

float Node::rollout(Position &pos) {
    const float scale = 1.0f / 133.0f;

    int eval  = evaluate(pos);
    float res = 1 / (1 + std::exp(-eval * scale));

    visits++;
    result += 1 - res;

    return res;
}

Node* Node::select() {
    int   bestIndex = -1;
    float bestUCT   = 0.0f;

    for (int i = 0; i < cCount; i++) {
        float thisUCT = uct(visits, children[i].visits, children[i].result);

        if (thisUCT <= bestUCT)
            continue;

        bestUCT   = thisUCT;
        bestIndex = i;
    }

    return (bestIndex != -1 ? &children[bestIndex] : nullptr);
}
