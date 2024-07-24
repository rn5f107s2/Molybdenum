#include "mcts.h"
#include "Position.h"
#include "eval.h"
#include "Movegen.h"
#include "timemanagement.h"
#include "search.h"

#include <stdlib.h>
#include <iostream>
#include <cmath>
#include <iomanip>

#ifdef TUNE
    extern Tune tune;
#endif

void rootSearch(Position &pos, SearchTime &st) {
    Node       root;
    NodePool   pool(256);
    SearchInfo si;
    si.clear();
    si.st = st;
    pos.policyNet.loadDefault();
    root.visits = 1;

    while (   (int((pool.currIdx) + 218) < pool.limit) 
           && ((si.nodeCount & 511) || !stop<Soft>(st, si)))
    {
        if (   st.limit == Nodes
            && si.nodeCount >= st.nodeLimit)
            break;

        root.search(pos, pool, 0);
        si.nodeCount++;
    }

    benchNodes += si.nodeCount;

    float bestRes;
    Move  bestMove;

    for (int j = 0; j < root.cCount; j++) {
        float thisRes = root.children[j].result / root.children[j].visits;

        std::cout << "info string move: " << moveToString(root.children[j].move) 
                                          << " Q: " << std::setprecision(3) << thisRes
                                          << " Visits: " << root.children[j].visits << std::endl;

        if (thisRes <= bestRes)
            continue;

        bestRes  = thisRes;
        bestMove = root.children[j].move;
    }

    std::cout << "info depth 1 nodes " << si.nodeCount << " score cp " << int((bestRes - 0.5f) * 1200) << std::endl;
    std::cout << "bestmove " << moveToString(bestMove) << std::endl;

    pool.freeMemory();
}

NodePool::NodePool(int mb) : sizeMB(mb), 
                             limit((mb * 1024 * 1024) / sizeof(Node)),
                             currIdx(0) {
    memory = reinterpret_cast<Node*>(malloc(limit * sizeof(Node)));
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

float uct(uint32_t pVisits, uint32_t visits, float score, float policy, bool root) {
    float c = tune.c;

    float q                    = visits == 0 ? 1.0f : score / visits;
    float whateverThisIsCalled = policy * c * std::sqrt(pVisits) / (1 + visits);

    if (root)
        whateverThisIsCalled *= tune.w;

    return q + whateverThisIsCalled;
}

float Node::search(Position &pos, NodePool &pool, int ply) {
    if (!visits && ply)
        return rollout(pos);

    if (visits <= 1)
        expand(pos, pool);

    if (!cCount) {
        u64 ksq      = pos.getPieces(pos.sideToMove, KING);
        u64 checkers = attackersTo<false, false>(lsb(ksq),pos.getOccupied(), pos.sideToMove ? BLACK_PAWN : WHITE_PAWN, pos);

        visits = std::numeric_limits<uint32_t>::max() - 1;
        result = (checkers ? 1.0f : 0.5f) * visits;

        return 1 - (checkers ? 1.0f : 0.5f);
    }

    if (   pos.hasRepeated(ply)
        || pos.plys50moveRule > 99
        || (pos.phase <= 3 && !(pos.getPieces(PAWN)))) 
    {
        visits = std::numeric_limits<uint32_t>::max() - 1;
        result = 0.5f * visits;

        return 0.5f;
    }

    Node* toSearch = select(!ply);
    pos.makeMove(toSearch->move);

    visits++;
    float res = toSearch->search(pos, pool, ply + 1);
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

    float sum = 0;
    float scores[218];

    pos.policyNet.initAccumulator(pos.bitBoards, pos.sideToMove);

    for (int i = 0; i < ml.length; i++) {
        children[i].move = ml.moves[i].move;
        scores[i]        = std::exp(pos.policyNet.forward(ml.moves[i].move, pos.sideToMove));

        sum += scores[i];
    }

    for (int i = 0; i < ml.length; i++)
        children[i].policy = scores[i] / sum;
}

float Node::rollout(Position &pos) {
    const float scale = 1.0f / 133.0f;

    int eval  = evaluate(pos);
    float res = 1 / (1 + std::exp(-eval * scale));

    visits++;
    result += 1 - res;

    return res;
}

Node* Node::select(bool root) {
    int   bestIndex = -1;
    float bestUCT   = 0.0f;

    for (int i = 0; i < cCount; i++) {
        float thisUCT = uct(visits, children[i].visits, children[i].result, children[i].policy, root);

        if (thisUCT <= bestUCT)
            continue;

        bestUCT   = thisUCT;
        bestIndex = i;
    }

    return (bestIndex != -1 ? &children[bestIndex] : nullptr);
}
