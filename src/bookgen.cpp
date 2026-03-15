#include "bookgen.h"

#include "Movegen.h"
#include "thread.h"

#include <fstream> 

void Node::expand(Position& pos) {
    u64 ksq = pos.getPieces(pos.sideToMove, KING);
    u64 checkers = attackersTo<false, false>(lsb(ksq),pos.getOccupied(), pos.sideToMove ? BLACK_PAWN : WHITE_PAWN, pos);

    generateMoves<false>(pos, ml, checkers);

    children = (Node*) malloc(sizeof(Node) * ml.length);

    for (int i = 0; i < ml.length; i++) {
        children[i].visited = children[i].children = 0;
        children[i].ml.currentIdx = 0; children[i].ml.length = 0;
    }
}

Node* Node::findChild(Move m) {
    int idx = 0;

    for (; idx < ml.length; idx++)
        if (ml.moves[idx].move == m)
            break;

    std::cout << idx << " " << ml.length << std::endl;

    return children + idx;
}

void generateSingle(Position& pos, Node* current, ThreadPool& tp, SearchTime& st) {
    if (current->visited) {
        bool split = !current->children;

        if (!current->children)
            current->expand(pos);

        for (int i = 0; i < 1 + split && current->ml.length; i++) {
            tp.start(pos, st, MAXDEPTH);
            tp.join();

            Move m = tp.threads[0].state.si.bestRootMove;

            pos.makeMove(m);

            generateSingle(pos, current->findChild(m), tp, st);

            pos.unmakeMove(m);
        }
    } else
        current->visited = true;
}

void collect(Position& pos, std::ofstream& outfile, Node* current) {
    if (!current->visited)
        return;

    if (!current->children) {
        outfile << pos.fen() << "\n";
        return;
    }

    for (int i = 0; i < current->ml.length; i++) {
        pos.makeMove(current->ml.moves[i].move); pos.movecount++;

        collect(pos, outfile, current->children + i);

        pos.unmakeMove(current->ml.moves[i].move); pos.movecount--;
    }
}

void generate(Position& pos, const std::string& bookName, int positions) {
    ThreadPool tp; tp.set(1); tp.clear();
    SearchTime st;
    Node root;

    st.limit = Nodes;
    st.nodeLimit = 5000;

    for (int count = 0; count < positions; count++)
        generateSingle(pos, &root, tp, st);

    std::ofstream outfile(bookName);

    collect(pos, outfile, &root);

    std::cout << "Done!" << std::endl;
}