#ifdef DATAGEN

#include "Newgen.h"
#include "../thread.h"

void start(const std::string &filePrefix, u64 initialSeed, int batchSize) {
    Position pos;
    Thread t(nullptr, 0);
    SearchState state;
    std::mt19937 random(initialSeed);

    state.si.st.limit = Nodes;
    state.thread = &t;
    pos.net.loadDefaultNet();

    loop(state, pos, filePrefix, random, batchSize);
}

void loop(SearchState &state, Position &pos, const std::string &filePrefix, std::mt19937 &random, int batchSize) {
    int batch = 1;

    while (true) {
        std::string fileName = filePrefix + "_b" + std::to_string(batch);
        std::ofstream out(fileName);
        auto begin = std::chrono::steady_clock::now();
        int fens = 0;

        std::cout << "Starting Batch " << batch << std::endl;

        for (int i = 1; i <= batchSize; i++) {
            createExit(state, pos, random);
            fens += playGame(state, pos, out);

            u64 timeTaken = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin).count();
            u64 fps = (fens * 1000) / std::max(timeTaken, uint64_t(1));

            std::cout << "\r[" << i << " / " << batchSize << "] Games played at " << fps << " fens/second"  << std::flush;
        }

        out.close();
        batch++;
        
        std::cout << std::endl;
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
            u64 ksq = pos.getPieces(pos.sideToMove, KING);
            u64 checkers = attackersTo<false, false>(lsb(ksq),pos.getOccupied(), pos.sideToMove ? BLACK_PAWN : WHITE_PAWN, pos);
            generateMoves<false>(pos, ml, checkers);

            if (!ml.length)
                break;

            Move randomMove = ml.moves[randomSegements[i % 4] % ml.length].move;

            pos.makeMove(randomMove);
            pos.movecount++;
        }

    } while(!verifyExit(state, pos));
}

bool verifyExit(SearchState &state, Position &pos) {
    state.si.st.nodeLimit = VERIF_NODES;
    return abs(state.startSearch(pos, state.si.st, MAXDEPTH)) <= MAX_UNBALANCE;
}

int playGame(SearchState &state, Position &pos, std::ofstream &out) {
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

        if (   pos.plys50moveRule >= 100 
            || pos.hasRepeated(0)
            || (pos.phase <= 3 && !(pos.getPieces(PAWN)))) 
        {
            result = 1;
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

        if (adjCounter[1] >= DADJ_MOVECOUNT) {
            result = 1;
            break;
        }

        pos.makeMove(bestMove);
        pos.net.initAccumulator(pos.bitBoards);
        pos.movecount++;
    }

    std::string resultString = result == 1 ? "0.5" : (result == 2 ? "1.0" : "0.0"); 

    for (int i = 0; i < fens.getSize(); i++)
        out << fens.at(i)   << " | " 
            << scores.at(i) << " | " 
            << resultString << " | " 
            << moveToString(bestMoves.at(i)) << "\n";

    out << std::flush;

    return fens.getSize();
}

#endif