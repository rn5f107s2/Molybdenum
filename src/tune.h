#ifndef MOLYBDENUM_TUNE_H
#define MOLYBDENUM_TUNE_H

#include <iomanip>
#include "UCIOptions.h"
#include "iostream"

struct Tune {
    float LMRDiv, LMRBase, LMRImproving;
    int LMRDepth, LMRMovecount, LMRHistDivPos, LMRHistDivNeg;
    int AspiBase, AspiLo, AspiHi, AspiDepth; float AspiWide;
    int RFPBase, RFPImproving, RFPDepth;
    int NMPDepth, NMPSeThreshold, NMPDepthThreshold, NMPBaseRed;
    int MCPDepth, MCPMultiplier;
    int FPDepth, FPBase, FPMult;
    int HistDepth, HistMult;
    int QsSEEMargin, QsDeltaMargin;
    int HistDepthMult, HistMax, HistLimit;
    int SEEPawn, SEEKnight, SEEBishop, SEERook, SEEQueen;
};

class TuneOptions: public UCIOptions {
    public:
        bool setOption(const std::string &name, int value) override {
            UCIOptionSpin *option = findOptionSpin(name);

            if (!option)
                return false;

            option->setter = reinterpret_cast<void (*)(int)>(value);
            return true;
        };

        int getValue(const char* name)  {
            UCIOptionSpin *option = findOptionSpin(name);

            if (!option) {
                std::cout << "No such option " << name << std::endl;
                return 0;
            }

            return int(reinterpret_cast<u64>(option->setter));
        }

        void printSPSAConfig() {
            for (int i = 0; i < spinOptions.getSize(); i++) {
                UCIOptionSpin &option = spinOptions.at(i);

                std::cout << option.name
                          << ", int, "
                          << option.dfault
                          << ", "
                          << option.min
                          << ", "
                          << option.max
                          << ", "
                          << std::setprecision(3)
                          << std::max(float(option.max - option.min) / 20, 0.500f)
                          << ", 0.0020\n";
            }
        }

        void init() override;
        bool initialized = false;
};

#endif //MOLYBDENUM_TUNE_H
