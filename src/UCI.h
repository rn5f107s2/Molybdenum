#ifndef MOLYBDENUM_UCI_H
#define MOLYBDENUM_UCI_H

#include "Move.h"
#include "Position.h"
#include "search.h"
#include "tune.h"
#include "UCIOptions.h"
#include "thread.h"
#include "mcts.h"   

#include <unordered_map>
#include <vector>
#include <sstream>

class UCI {

private:
    Position   internalBoard;
    MCTSState  state;
    UCIOptions options = UCIOptions(this);
    std::unordered_map<std::string, void(UCI::*)(const std::string&)> commands;

    const std::string name       = "Molybdenum";
    const std::string version    = "4.0";
    const std::string defaultFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

#ifdef TUNE
    TuneOptions tuneOptions;
#endif

public:
    ThreadPool threads;

    UCI() : state(512) {
        internalBoard.net.loadDefaultNet();
        internalBoard.setBoard(defaultFEN);

        commands["d"] = &UCI::d;
        commands["go"] = &UCI::go;
        commands["uci"] = &UCI::uci;
        commands["eval"] = &UCI::eval;
        commands["stop"] = &UCI::stop;
        commands["bench"] = &UCI::bench;
        commands["policy"] = &UCI::policy;
        commands["isready"] = &UCI::isready;
        commands["goPerft"] = &UCI::goPerft;
        commands["position"] = &UCI::position;
        commands["setoption"] = &UCI::setoption;
        commands["ucinewgame"] = &UCI::ucinewgame;


        options.init();
        threads.set(1);
        threads.clear();

#ifdef TUNE
        tuneOptions.init();
        tuneOptions.printSPSAConfig();
#endif
    }

    void d(const std::string &args);
    void go(const std::string &time);
    void uci(const std::string &args);
    void eval(const std::string &args);
    void stop(const std::string &args);
    void bench(const std::string &args);
    void policy(const std::string &args);
    void goPerft(const std::string &args);
    void isready(const std::string &args);
    void position(const std::string &args);
    void setoption(const std::string &args);
    void ucinewgame(const std::string &args);

    void start(int argc, char** argv);
    void loop();
    void handleInput(const std::string &in);
};

inline std::vector<std::string> split(const std::string &s, char delim = ' ') {
    std::stringstream stream(s);
    std::vector<std::string> out;
    std::string sample;

    out.reserve(10);

    while (std::getline(stream, sample, delim)) { out.push_back(sample); }

    return out;
}

inline bool contains(const std::string& input, const std::string& searchedTerm) {
    return input.find(searchedTerm) != std::string::npos;
}

#endif //MOLYBDENUM_UCI_H
