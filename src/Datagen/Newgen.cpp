#define DATAGEN

#ifdef DATAGEN

#include "Newgen.h"
#include "../thread.h"

void start(const std::string &filePrefix, u64 initialSeed, int batchSize) {
    Position pos;
    ThreadPool tp;
    SearchState state;
    std::mt19937 random(initialSeed);

    tp.set(1);

    state.si.st.limit = Nodes;
    state.thread = tp.get(0);

    loop(state, pos, filePrefix, random, batchSize);
}

void loop(SearchState &state, Position &pos, const std::string &filePrefix, std::mt19937 &random, int batchSize) {
    int batch = 0;

    while (true) {
        std::string fileName = filePrefix + "_b" + std::to_string(batch);
        std::ofstream out(fileName);

        for (int i = 0; i < batchSize; i++) {
            createExit(state, pos, random);
            playGame(state, pos, out);
        }

        out.close();
        batch++;
    }
    
}

void createExit(SearchState &state, Position &pos, std::mt19937 &random) {
    int exitDepth = 6 + (random() & 0b11);

    do {
        int seed = random();
        uint8_t* randomSegements = reinterpret_cast<uint8_t*>(&seed);

        pos.setBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

        for (int i = 0; i < exitDepth; i++) {
            MoveList ml; 
            generateMoves<false>(pos, ml);

            if (!ml.length)
                break;

            std::cout << "Alive" << std::endl;
            Move randomMove = ml.moves[randomSegements[i % 4] % ml.length].move;
            std::cout << "Still alive" << std::endl;

            pos.makeMove(randomMove);
            pos.movecount++;
        }

        pos.printBoard();

    } while(!verifyExit(state, pos));
}

bool verifyExit(SearchState &state, Position &pos) {
    state.si.st.nodeLimit = VERIF_NODES;
    return abs(state.startSearch(pos, state.si.st, MAXDEPTH)) <= MAX_UNBALANCE;
}

void playGame(SearchState &state, Position &pos, std::ofstream &out) {
    state.clearHistory();
    state.si.st.nodeLimit = SEARCH_NODES;

    Stack<int> scores;
    Stack<std::string> fens;
    Stack<Move> bestMoves;

    int result = 1;
    int adjCounter[2] = {0, 0};

    while (true) {
        Move bestMove;

        int score = state.startSearch(pos, state.si.st, MAXDEPTH, bestMove);

        // Stale- / Checkmate
        if (!bestMove) {
            result = score != 0 ? pos.sideToMove * 2 : 1;
            break;
        }

        fens.push(pos.fen());
        scores.push(score);
        bestMoves.push(bestMove);

        if (abs(score) >= WADJ_SCORE) {
            adjCounter[0]++;
        } else {
            adjCounter[0] = 0;
        }

        if (abs(score) <= DADJ_SCORE) {
            adjCounter[1]++;
        } else {
            adjCounter[1] = 0;
        }

        if (adjCounter[0] >= WADJ_MOVECOUNT || abs(score) >= MAXMATE) {
            result = (score >= WADJ_SCORE) ? (pos.sideToMove * 2) : (2 - pos.sideToMove * 2);
            break;
        }

        if (   adjCounter[1] >= DADJ_MOVECOUNT 
            || pos.plys50moveRule >= 100 
            || pos.hasRepeated(0)) 
        {
            result = 1;
            break;
        }

        pos.makeMove(bestMove);
        pos.movecount++;
    }

    std::string resultString = result == 1 ? "0.5" : (result == 2 ? "1.0" : "0.0"); 

    for (int i = 0; i < fens.getSize(); i++)
        out << fens.at(i)   << " | " 
            << scores.at(i) << " | " 
            << resultString << " | " 
            << moveToString(bestMoves.at(i)) << "\n";

    out << std::flush;
}

#endif