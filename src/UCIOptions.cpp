#include "UCIOptions.h"
#include "Transpositiontable.h"
#include "timemanagement.h"
#include "UCI.h"

bool UCIOptions::setOption(const std::string& name, int value) {
    UCIOptionSpin *option = findOptionSpin(name);

    if (!option)
        return false;

    if (value <= option->max && value >= option->min)
        (*this.*(option->setter))(value);
    else
        std::cout << "Value " << value << " is not within bounds of option " << name << "\n";

    return true;
}

void UCIOptions::setHash(int val) {
    resizeTT(val);
}

void UCIOptions::setThreads(int val) {
    uci->threads.set(val);
}

void UCIOptions::setMoveOverhead(int val) {
    setOverhead(val);
}

void UCIOptions::init() {
    spinOptions.push(UCIOptionSpin("Hash", 1, 1024, 16, &UCIOptions::setHash));
    spinOptions.push(UCIOptionSpin("Threads", 1, 32, 1, &UCIOptions::setThreads));
    spinOptions.push(UCIOptionSpin("Move Overhead", 0, 2000, 10, &UCIOptions::setMoveOverhead));
}

UCIOptionSpin *UCIOptions::findOptionSpin(const std::string& name) {
    for (int i = 0; i < spinOptions.getSize(); i++) {
        UCIOptionSpin &option = spinOptions.at(i);

        if (option.name == name)
            return &option;
    }

    return nullptr;
}

void UCIOptions::printOptions() {
    for (int i = 0; i < spinOptions.getSize(); i++) {
        UCIOptionSpin &option = spinOptions.at(i);

        std::cout << "option name "
                  << option.name
                  << " type spin default "
                  << option.dfault
                  << " min "
                  << option.min
                  << " max "
                  << option.max
                  << "\n";
    }
}
