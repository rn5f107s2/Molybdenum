#include "UCIOptions.h"
#include "Transpositiontable.h"
#include "timemanagement.h"

void UCIOptions::setOption(const std::string& name, int value) {
    UCIOptionSpin *option = findOptionSpin(name);

    if (!option) {
        std::cout << "No such option: " << name << "\n";
        return;
    }

    if (value <= option->max && value >= option->min)
        option->setter(value);
    else
        std::cout << "Value " << value << " is not within bounds of option " << name << "\n";
}

void UCIOptions::init() {
    spinOptions.push(UCIOptionSpin("Hash", 1, 1024, 16, &resizeTT));
    spinOptions.push(UCIOptionSpin("Move Overhead", 0, 1000, 10, &setOverhead));
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
