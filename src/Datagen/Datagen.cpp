#include "../search.h"

#ifdef DATAGEN
#include <fstream>
#include <iostream>
#include "Datagen.h"
#include "../Position.h"

[[noreturn]] void start(Position &pos, const std::string& filename) {
    init();
    u64 gameCount = 0;
    u64 fenCount = 0;
    u64 lastFenCount = 0;
    auto lastTime = std::chrono::steady_clock::now();
    std::cout << "Starting Data Generation\n";
    while (true) {
        createExit(pos);
        playGame(pos, filename, fenCount);
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
    searchTime st;
    st.limit = Nodes; st.nodeLimit[Soft] = 5000; st.nodeLimit[Hard] = 5000000;

    return abs(startSearch(pos, st)) < 150;
}

void playGame(Position &pos, const std::string& filename, u64 &fenCount) {
    int adjCounter = 0;
    int wadjCounter = 0;
    int wadjReq = int(seedDataGen % 20) + 5;
    std::string result;
    Stack<int> scores;
    Stack<std::string> fens;
    Stack<Move> bestMoves;
    std::ofstream output;
    output.open(filename, std::ios::app);

    clearHistory();

    while (true) {
        Move bestMove;
        searchTime st;
        st.nodeLimit[Soft] = 5000; st.nodeLimit[Hard] = 5000000;
        st.limit = Nodes;

        int score = startSearch(pos, st, MAXDEPTH, bestMove);

        if (abs(score) > 300)
            wadjCounter++;
        else
            wadjCounter = 0;

        if (abs(score) >= MAXMATE || wadjCounter > wadjReq) {
            result = (score > 0 == pos.sideToMove) ? "1.0" : "0.0";
            break;
        }

        if (abs(score) <= 0)
            adjCounter++;
        else
            adjCounter = 0;

        if (adjCounter > 7 || pos.plys50moveRule >= 100 || bestMove == NO_MOVE) {
            result = "0.5";
            break;
        }

        if (!pos.isCapture(bestMove) && extract<FLAG>(bestMove) != PROMOTION) {
            fens.push(pos.fen());
            scores.push(score * (pos.sideToMove ? 1 : -1));
            bestMoves.push(bestMove);
        }
        pos.makeMove(bestMove);
        pos.movecount++;
    }

    while (fens.getSize()) {
        std::string fen = fens.pop();
        std::string score = std::to_string(scores.pop());
        std::string bestMove = moveToString(bestMoves.pop());

        fen += " | ";
        fen += score;
        fen += " | ";
        fen += result;
        fen += "\n";
        output << fen;
        fenCount++;
    }

    output.close();
}
#endif