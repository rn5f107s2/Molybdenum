#include "../search.h"
#include "../mcts.h"

//#ifdef DATAGEN
#include <fstream>
#include <iostream>
#include "Datagen.h"
#include "../Position.h"

[[noreturn]] void start(Position &pos, const std::string &valueFile, const std::string &policyFile) {
    init();
    u64 gameCount = 0;
    u64 fenCount = 0;
    u64 lastFenCount = 0;
    auto lastTime = std::chrono::steady_clock::now();
    std::cout << "Starting Data Generation\n";
    while (true) {
        createExit(pos);
        playGame(pos, valueFile, policyFile, fenCount);
        gameCount++;

        if (gameCount % 100 == 0) {
            int searchTime = int(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - lastTime).count());
            std::cout << gameCount << " Games done\n";
            std::cout << "at " << ((fenCount - lastFenCount) * 1000) / std::max(searchTime, 1) << " Fens per second\n";
            lastFenCount = fenCount;
            lastTime = std::chrono::steady_clock::now();
        }
    }
}

void createExit(Position &pos) {
    bool valid = false;
    int exitDepth = int(seedDataGen % 4) + 6;

    while (!valid) {
        pos.setBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        for (int i = 0; i <= exitDepth; i++) {
            MoveList legalMoves;
            u64 ksq = pos.getPieces(pos.sideToMove, KING);
            u64 checkers = attackersTo<false, false>(lsb(ksq), pos.getOccupied(), pos.sideToMove ? BLACK_PAWN : WHITE_PAWN, pos);

            generateMoves<false>(pos, legalMoves, checkers);

            if (!legalMoves.length)
                break;

            int rand = int(seedDataGen % legalMoves.length);
            seedDataGen = randomULL(seedDataGen);

            pos.makeMove(legalMoves.moves[rand].move);
            pos.movecount++;
        }

        valid = verifyExit(pos);
    }
}

bool verifyExit(Position &pos) {
    SearchTime st;
    st.limit = Nodes; st.nodeLimit = 1000;

    return abs(rootSearch(pos, st)) < 150;
}

void playGame(Position &pos, const std::string &valueFile, const std::string &policyFile, u64 &fenCount) {
    int adjCounter = 0;
    std::string result;
    Stack<int> scores;
    Stack<std::string> fens;
    Stack<PolicyGenInfo> visits;
    std::ofstream valueOutput;
    std::ofstream policyOutput;
    valueOutput.open(valueFile, std::ios::app);
    policyOutput.open(policyFile, std::ios::app);

    while (true) {
        Move bestMove;
        PolicyGenInfo pgi;
        SearchTime st;
        st.nodeLimit = 5000;
        st.limit = Nodes;

        int score = rootSearch(pos, st, &pgi, &bestMove);

        if (abs(score) <= 0)
            adjCounter++;
        else
            adjCounter = 0;

        if (bestMove == NO_MOVE) {
            u64 ksq = pos.getPieces(pos.sideToMove, KING);
            u64 checkers = attackersTo<false, false>(lsb(ksq),pos.getOccupied(), pos.sideToMove ? BLACK_PAWN : WHITE_PAWN, pos);

            result = !checkers ? "0.5" : (pos.sideToMove ? "0.0" : "1.0");
            break;
        }

        if (adjCounter > 10 || pos.plys50moveRule >= 100) {
            result = "0.5";
            break;
        }

        fens.push(pos.fen());
        scores.push(score * (pos.sideToMove ? 1 : -1));
        visits.push(pgi);

        pos.makeMove(bestMove);
        pos.movecount++;
    }

    while (fens.getSize()) {
        std::string fen = fens.pop();
        std::string score = std::to_string(scores.pop());
        PolicyGenInfo pgi = visits.pop();

        std::string policy = fen;
        std::string value  = fen;

        value += " | ";
        value += score;
        value += " | ";
        value += result;
        
        int pgiIndex = 0;

        while (pgi.moves[pgiIndex]) {
            policy += " | ";
            policy += moveToString(pgi.moves[pgiIndex]);
            policy += " ";
            policy += std::to_string(pgi.visits[pgiIndex]);
            
            pgiIndex++;
        }
        

        value += "\n";
        policy += "\n";
        valueOutput << value;
        policyOutput << policy;
        fenCount++;
    }

    valueOutput.close();
    policyOutput.close();
}
//#endif