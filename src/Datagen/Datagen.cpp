#ifdef DATAGEN
#include <fstream>
#include <iostream>
#include "Datagen.h"
#include "../Position.h"
#include "../search.h"

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
    st.limit = Nodes; st.nodeLimit = 25000;

    return abs(startSearch(pos, st)) < 250;
}

void playGame(Position &pos, const std::string& filename, u64 &fenCount) {
    int adjCounter[2];
    std::string result;
    Stack<int> scores;
    Stack<std::string> fens;
    Stack<Move> bestMoves;
    std::ofstream output;
    output.open(filename, std::ios::app);

    while (true) {
        Move bestMove;
        searchTime st;
        st.nodeLimit = 20000;
        st.limit = Nodes;

        int score = startSearch(pos, st, bestMove);

        if (abs(score) >= MAXMATE)
            break;

        if (abs(score) >= 400)
            adjCounter[WIN_ADJ]++;
        else
            adjCounter[WIN_ADJ] = 0;

        if (abs(score) <= 10)
            adjCounter[DRAW_ADJ]++;
        else
            adjCounter[DRAW_ADJ] = 0;

        if (adjCounter[WIN_ADJ] > 5) {
            result = (score > 0 == pos.sideToMove) ? "1.0" : "0.0";
            break;
        }

        if (adjCounter[DRAW_ADJ] > 7) {
            result = "0.5";
            break;
        }

        fens.push(pos.fen());
        scores.push(score);
        bestMoves.push(bestMove);
        pos.makeMove(bestMove);
        pos.movecount++;
    }

    output << "//newgame\n";

    while (fens.getSize()) {
        std::string fen = fens.pop();
        std::string score = std::to_string(scores.pop());
        std::string bestMove = moveToString(bestMoves.pop());

        fen += " [";
        fen += result;
        fen += "] | ";
        fen += score;
        fen += " | ";
        fen += bestMove;
        fen += "\n";
        output << fen;
        fenCount++;
    }

    output << "//gameend\n";
    output.close();
}
#endif