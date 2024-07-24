#ifndef MOLYBDENUM_UCIOPTIONS_H
#define MOLYBDENUM_UCIOPTIONS_H

#include <utility>

#include "string"
#include "Utility.h"

class UCI;
class UCIOptions;

class UCIOptionSpin {
    public:
        explicit UCIOptionSpin(std::string name = "NONE", int min = 0, int max = 0, int dfault = 0, void (UCIOptions::*setter)(int) = nullptr) {
            this->name = std::move(name);
            this->min = min;
            this->max = max;
            this->dfault = dfault;
            this->setter = setter;
            this->value  = dfault;
        }

        std::string name;
        int min;
        int max;
        int dfault;
        int value;
        void (UCIOptions::*setter)(int);
};

class UCIOptions {
    public:
        virtual void init();
        virtual bool setOption(const std::string& name, int value);
        void printOptions();

        UCIOptions(UCI* u) {
            uci = u;
        }

        UCIOptions() {}

    protected:
        Stack<UCIOptionSpin> spinOptions;
        UCIOptionSpin* findOptionSpin(const std::string& name);

    private:
        UCI* uci;

        void setHash(int val);
        void setThreads(int val);
        void setMoveOverhead(int val);
};


#endif //MOLYBDENUM_UCIOPTIONS_H
