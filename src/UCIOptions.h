#ifndef MOLYBDENUM_UCIOPTIONS_H
#define MOLYBDENUM_UCIOPTIONS_H

#include <utility>

#include "string"
#include "Utility.h"

class UCIOptionSpin {
    public:
        explicit UCIOptionSpin(std::string name = "NONE", int min = 0, int max = 0, int dfault = 0, void (*setter)(int) = nullptr) {
            this->name = std::move(name);
            this->min = min;
            this->max = max;
            this->dfault = dfault;
            this->setter = setter;
        }

        std::string name;
        int min;
        int max;
        int dfault;
        void (*setter)(int);
};

class UCIOptions {
    public:
        virtual void init();
        virtual bool setOption(const std::string& name, int value);
        void printOptions();

    protected:
        Stack<UCIOptionSpin> spinOptions;
        UCIOptionSpin* findOptionSpin(const std::string& name);
};


#endif //MOLYBDENUM_UCIOPTIONS_H
