#include "UCIOptions.h"
#include "Transpositiontable.h"
#include "timemanagement.h"

bool UCIOptions::setOption(const std::string& name, int value) {
    UCIOptionSpin *option = findOptionSpin(name);

    if (!option)
        return false;

    if (value <= option->max && value >= option->min)
        option->setter(value);
    else
        std::cout << "Value " << value << " is not within bounds of option " << name << "\n";

    return true;
}

void foo([[maybe_unused]] int bar){}

void UCIOptions::init() {
    spinOptions.push(UCIOptionSpin("Hash", 1, 2047, 16, &resizeTT));
    spinOptions.push(UCIOptionSpin("Threads", 1, 1, 1, &foo));
    spinOptions.push(UCIOptionSpin("Move Overhead", 0, 2000, 10, &setOverhead));
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
