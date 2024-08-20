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

int rootSearch(Position &pos, SearchTime &st, PolicyGenInfo* pgi, Move* bm) {
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

    int pgiIndex = 0;
    float bestRes = -1.0f;
    Move  bestMove;

    for (int j = 0; j < root.cCount; j++) {
        float thisRes = root.children[j].result / root.children[j].visits;

//#ifndef DATAGEN
        std::cout << "info string move: " << moveToString(root.children[j].move) 
                                          << " Q: " << std::setprecision(3) << thisRes
                                          << " Visits: " << root.children[j].visits << std::endl;
//#endif

        if (pgi) {
            pgi->moves           [pgiIndex] = root.children[j].move;
            pgi->visitPecrcentage[pgiIndex] = float(root.children[j].visits) / float(si.nodeCount.load(std::memory_order_relaxed));
            pgiIndex++;
        }

        if (thisRes <= bestRes)
            continue;

        bestRes  = thisRes;
        bestMove = root.children[j].move;
    }

    int centipawns = std::log(bestRes / (1.0f - bestRes)) / (1.0f / 133.0f);

//#ifndef DATAGEN
    std::cout << "info depth 1 nodes " << si.nodeCount << " score cp " << centipawns << std::endl;
    std::cout << "bestmove " << moveToString(bestMove) << std::endl;
//#endif

    pool.freeMemory();

    if (bm)
        *bm = bestMove;

    return centipawns;
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

float uct(uint32_t pVisits, uint32_t visits, float score, float policy, bool root, float pq, float bq) {
    const float c = !root ? 1.32f : 2.35f;

    float q                    = visits == 0 ? fpu(pq, bq) : score / visits;
    float whateverThisIsCalled = policy * c * std::sqrt(pVisits) / (1 + visits);

    return q + whateverThisIsCalled;
}

float fpu(float pq, float bq) {
    float diff = (bq - (0.8f - pq));
    return 1.0f - std::clamp(diff * 3.09f, 0.0f, 1.0f);
}

float Node::search(Position &pos, NodePool &pool, int ply) {
    if (!visits && ply)
        return rollout(pos);

    if (visits <= 1)
        expand(pos, pool, ply);

    if (!cCount) {
        u64 ksq      = pos.getPieces(pos.sideToMove, KING);
        u64 checkers = attackersTo<false, false>(lsb(ksq),pos.getOccupied(), pos.sideToMove ? BLACK_PAWN : WHITE_PAWN, pos);

        visits++;
        result += (checkers ? 1.0f : 0.5f);

        return 1 - (checkers ? 1.0f : 0.5f);
    }

    if (   ply
        && (   pos.hasRepeated(ply)
            || pos.plys50moveRule > 99
            || (pos.phase <= 3 && !(pos.getPieces(PAWN))))) 
    {
        visits++;
        result += 0.5f;

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

void Node::expand(Position &pos, NodePool &pool, int ply) {
    MoveList ml;
    const float PST_VALUES[4] = {4.17f, 2.18f, 1.85f, 1.81f};

    u64 ksq = pos.getPieces(pos.sideToMove, KING);
    u64 checkers = attackersTo<false, false>(lsb(ksq),pos.getOccupied(), pos.sideToMove ? BLACK_PAWN : WHITE_PAWN, pos);

    generateMoves<false>(pos, ml, checkers);
    cCount   = ml.length;
    children = pool.allocate(cCount);

    float sum = 0;
    float scores[218];

    pos.policyNet.initAccumulator(pos.bitBoards, pos.sideToMove, getThreats(pos, !pos.sideToMove));

    for (int i = 0; i < ml.length; i++) {
        children[i].move = ml.moves[i].move;
        scores[i]        = std::exp(pos.policyNet.forward(ml.moves[i].move, pos.sideToMove) / PST_VALUES[std::min(ply, 3)]);

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
    float bestQ     = 0.0f;

    for (int i = 0; i < cCount; i++)
        if (children[i].visits)
            bestQ = std::max(bestQ, children[i].result / children[i].visits);

    for (int i = 0; i < cCount; i++) {
        float thisUCT = uct(visits, children[i].visits, children[i].result, children[i].policy, root, result / visits, bestQ);

        if (thisUCT <= bestUCT)
            continue;

        bestUCT   = thisUCT;
        bestIndex = i;
    }

    return (bestIndex != -1 ? &children[bestIndex] : nullptr);
}
