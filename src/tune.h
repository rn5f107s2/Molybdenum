#ifndef MOLYBDENUM_TUNE_H
#define MOLYBDENUM_TUNE_H

#define TUNE

#ifdef TUNE

#include <iomanip>
#include "iostream"
#include "UCIOptions.h"

struct Tune {
    float LMRDiv, LMRBase, LMRImproving;
    int LMRDepth, LMRMovecount;
    int AspiBase, AspiLo, AspiHi, AspiDepth, AspiWide;
    int RFPBase, RFPImproving, RFPDepth;
    int NMPDepth, NMPSeThreshold, NMPDepthThreshold, NMPBaseRed;
    int MCPDepth, MCPMultiplier;
    int FPDepth, FPBase, FPMult;
    int HistDepth, HistMult;
    int QsSEEMargin, QsDeltaMargin;
    int HistDepthMult, HistMax, HistLimit;
    int SEEPawn, SEEKnight, SEEBishop, SEERook, SEEQueen;

    float c, w;
};

class TuneOptions: public UCIOptions {
    public:
        bool setOption(const std::string &name, int v) override {
            UCIOptionSpin *option = findOptionSpin(name);

            if (!option)
                return false;

            option->value = v;
            return true;
        };

        int getValue(const char* name)  {
            UCIOptionSpin *option = findOptionSpin(name);

            if (!option) {
                std::cout << "No such option " << name << std::endl;
                return 0;
            }

            return option->value;
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


#define RANGE(x, y, z) int(y), int(z), int(x)

#define UPDATEFLOAT(x) tune.x = float(tuneOptions.getValue(#x)) / 100;
#define UPDATEINT(x) tune.x = tuneOptions.getValue(#x);
#define TUNEFLOAT(w, x, y, z) if (!initialized) {spinOptions.push(UCIOptionSpin(#w, RANGE(int(x * 100), int(y * 100), int(z * 100))));} UPDATEFLOAT(w)
#define TUNEINT(w, x, y, z) if (!initialized) {spinOptions.push(UCIOptionSpin(#w, RANGE(x, y, z)));} UPDATEINT(w)

extern TuneOptions tuneOptions;
extern Tune tune;

inline void TuneOptions::init() {
    /*
    TUNEFLOAT(LMRBase, 0.75, 0.01, 4)
    TUNEFLOAT(LMRDiv, 2.19, 0.01, 8)
    TUNEFLOAT(LMRImproving, 0.55, 0, 2)
    TUNEINT(LMRDepth, 2, 1, 4)
    TUNEINT(LMRMovecount, 2, 1, 4)

    TUNEINT(AspiBase, 83, 1, 160)
    TUNEINT(AspiLo, 24, 1, 200)
    TUNEINT(AspiHi, 49, 1, 300)
    TUNEINT(AspiDepth, 2, 2, 10)
    TUNEINT(AspiWide, 3, 1, 5)

    TUNEINT(RFPBase, 115, 1, 300)
    TUNEINT(RFPImproving, 203, 1, 400)
    TUNEINT(RFPDepth, 9, 3, 15)

    TUNEINT(NMPDepth, 2, 1, 10)
    TUNEINT(NMPSeThreshold, 274, 1, 500)
    TUNEINT(NMPDepthThreshold, 6, 1, 12)
    TUNEINT(NMPBaseRed, 3, 1, 6)

    TUNEINT(MCPDepth, 4, 1, 24)
    TUNEINT(MCPMultiplier, 12, 1, 24)

    TUNEINT(FPDepth, 7, 1, 24)
    TUNEINT(FPMult, 209, 1, 400)
    TUNEINT(FPBase, 179, 1, 350)

    TUNEINT(HistDepth, 5, 1, 20)
    TUNEINT(HistMult, -5460, -9000, -1)

    TUNEINT(QsSEEMargin, -94, -200, -1)
    TUNEINT(QsDeltaMargin, 159, 1, 300)

    TUNEINT(HistDepthMult, 16, 1, 32)
    TUNEINT(HistMax, 1638, 1, 3000)
    TUNEINT(HistLimit, 15, 1, 20)

    TUNEINT(SEEPawn, 100, 1, 1200)
    TUNEINT(SEEKnight, 299, 1, 1200)
    TUNEINT(SEEBishop, 281, 1, 1200)
    TUNEINT(SEERook, 538, 1, 1200)
    TUNEINT(SEEQueen, 972, 1, 1200)
    */

    TUNEFLOAT(c, 1.41, 0.01, 3)
    TUNEFLOAT(w,  3.0, 0.01, 6)

    initialized = true;
}

#endif

#endif //MOLYBDENUM_TUNE_H
