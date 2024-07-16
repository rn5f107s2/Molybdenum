#include <string>
#include <iostream>
#include <chrono>
#include <sstream>
#include <vector>

#include "tune.h"
#include "UCI.h"
#include "Position.h"
#include "BitStuff.h"
#include "Perft.h"
#include "search.h"
#include "eval.h"
#include "timemanagement.h"
#include "UCIOptions.h"
#include "bench.h"
#include "nnue.h"
#include "Datagen/Datagen.h"
#include "thread.h"
#include "mcts.h"

void UCI::d([[maybe_unused]] const std::string &args) {
    internalBoard.printBoard();
}

void UCI::eval([[maybe_unused]] const std::string &args) {
    std::cout << evaluate(internalBoard) << std::endl;
}

void UCI::uci([[maybe_unused]] const std::string &args) {
        std::cout << "id name " << name << " " << version << "\n";
        std::cout << "id author rn5f107s2\n";
        options.printOptions();

#ifdef TUNE
        tuneOptions.printOptions();
        tuneOptions.init();
#endif

        std::cout << "uciok" << std::endl;
        return;
}

void UCI::isready([[maybe_unused]] const std::string &args) {
    std::cout << "readyok" << std::endl;
}

void UCI::ucinewgame([[maybe_unused]] const std::string &args) {
    if (!threads.done())
        return;

    threads.clear();
}

void UCI::goPerft([[maybe_unused]] const std::string &args) {
    int depth = std::stoi(args);
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    u64 nodeCount = startPerft(depth, internalBoard, true);

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds> (end - begin).count();

    std::cout << "______________\n";
    std::cout << "Nodes searched: " << nodeCount << "\n";
    std::cout << "Took: " << milliseconds << " milliseconds" << "\n";
    std::cout << "Speed: " << (nodeCount / (milliseconds + 1)) / 1000 << " mnps" << std::endl;
    return;
}

void UCI::bench([[maybe_unused]] const std::string &args) {
    if (!threads.done())
        return;

    SearchTime st;
    st.limit = Depth;
    benchNodes = 1;
    threads.clear();

    for (int i = 0; i != BENCH_SIZE; i++) {
        internalBoard.setBoard(positions[i]);
        go("depth " + std::to_string(BENCH_DEPTH));
        while (!threads.done()) {}
        threads.clear();

        std::cout << "\n";
    }

    auto milliseconds = int(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - st.searchStart).count());
    auto nps = (benchNodes * 1000) / (milliseconds + 1);

    std::cout << "_____________" << "\n";
    std::cout << benchNodes << " Nodes" << "\n";
    std::cout << 1 << " nps" << std::endl;
}

void UCI::setoption([[maybe_unused]] const std::string &args) {
    if (!threads.done())
        return;

    std::vector<std::string> splitArgs = split(args);
    bool found = options.setOption(splitArgs[1], std::stoi(splitArgs[3]));

#ifdef TUNE
        found |= tuneOptions.setOption(optionName, value);
        tuneOptions.init();
#endif

    if (!found)
        std::cout << "No such option: " << splitArgs[1] << std::endl;
}

void UCI::position([[maybe_unused]] const std::string &args) {
    if (!threads.done())
        return;

    const std::vector<std::string> argsSplit = split(args);
    unsigned int offset = 1;
    std::string fen     =  argsSplit[0] == "startpos"  ? defaultFEN : "";

    while (   argsSplit[0] == "fen" 
           && offset < argsSplit.size() 
           && argsSplit[offset] != "moves")
        fen += argsSplit[offset++] + " ";

    offset++;
    internalBoard.setBoard(fen);

    for (unsigned long i = offset; i < argsSplit.size(); i++) {
        std::string move = argsSplit[i];

        int from       = lsb(stringToSquare(move.substr(0, 2)));
        int to         = lsb(stringToSquare(move.substr(2, 4)));
        int flag       = (move.size() == 5) ? PROMOTION : NORMAL;
        int promoPiece = flag ? charIntToPiece(toupper(move.at(4)) - '0') - 1 : PROMO_KNIGHT;

        Move m = internalBoard.fromToToMove(from, to, promoPiece, flag);
        internalBoard.makeMove(m);
        internalBoard.net.initAccumulator(internalBoard.bitBoards);
    }
}

void UCI::go([[maybe_unused]] const std::string &args) {
    if (!threads.done())
        return;

    std::array<int, 2> time      = {};
    std::array<int, 2> increment = {};
    int depth     = MAXDEPTH;
    int movesToGo = 6000;

    SearchTime st;
    std::vector<std::string> splitTime = split(args);

    if (splitTime[0] == "infinite" || splitTime[0] == "depth") {
        st.limit = Depth;

        if (splitTime[0] == "depth")
            depth = std::stoi(splitTime[1]) + 1; 

    } else if (splitTime[0] == "nodes") {
        st.limit     = Nodes;
        st.nodeLimit = std::stoull(splitTime[1]);

    } else if (splitTime[0] == "movetime") {
        st.limit = Time;
        st.thinkingTime[Hard] = st.thinkingTime[Soft] = std::chrono::milliseconds(std::stoi(splitTime[1]) - moveOverHead);

    } else {
        st.limit = Time;

        for (unsigned long i = 0; i < splitTime.size(); i += 2) {
            if (contains(splitTime[i], "time"))
                time[contains(splitTime[i], "w")] = std::stoi(splitTime[i + 1]);
            else if (contains(splitTime[i], "inc"))
                increment[contains(splitTime[i], "w")] = std::stoi(splitTime[i + 1]);
            else if (contains(splitTime[i], "movestogo"))
                movesToGo = std::stoi(splitTime[i + 1]);
        }

        st.calcThinkingTime(time[internalBoard.sideToMove], increment[internalBoard.sideToMove], movesToGo);
    }

    //threads.start(internalBoard, st, depth);
    rootSearch(internalBoard, st);
}

void UCI::stop([[maybe_unused]] const std::string &args) {
    threads.stop();
}

void UCI::start(int argc, char** argv) {
    if (argc == 1)
        return loop();

    for (int i = 1; i < argc; i++) {
        std::string in = argv[i];

        if (in == "quit")
            return;

        handleInput(in);
    }
}

void UCI::loop() {    
    while (true) {
        std::string input;
        std::getline(std::cin, input);

        if (input == "quit") {
            threads.stop();
            while (!threads.done()) {}
            return;
        }

        handleInput(input);
    }
}

void UCI::handleInput(const std::string &in) {
    const std::string name = in.substr(0, in.find(' '));
    auto iter = commands.find(name);

    if (iter == commands.end()) {
        std::cout << "No such command: " << in << std::endl;
        return;
    }
    
    std::string args = in.substr(std::min(in.length(), name.length() + 1));
    (*this.*(iter->second))(args);
}
