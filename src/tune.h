#ifndef MOLYBDENUM_TUNE_H
#define MOLYBDENUM_TUNE_H

#define TUNE

#ifdef TUNE

#include <iomanip>
#include "UCIOptions.h"
#include "iostream"
#include "UCIOptions.h"

struct Tune {
    float LMRDiv, LMRBase, LMRImproving;
    int LMRDepth, LMRMovecount;
    int AspiBase, AspiLo, AspiHi, AspiDepth;
    float AspiWide;
    int RFPBase, RFPImproving, RFPDepth, RFPWorse;
    int NMPDepth, NMPSeThreshold, NMPDepthThreshold, NMPBaseRed;
    int ExtKeep;
    int MCPDepth, MCPBase, MCPImproving, MCPQuarterMul, MCPDepthMul;
    int FPDepth, FPBase, FPMult, FPQuarter;
    int HistDepth, HistMult, HistQuarter;
    int SEDepth, SETTDepthDiff, SESBBase, SESBMC, SESBMCMax;
    int LMRHistPos, LMRHistNeg, LMRNightmareMargin, LMRNightmareMovecount;
    int QsSEEMargin, QsDeltaMargin;
    int HistDepthMult, HistMax, HistLimit;
    int SEEPawn, SEEKnight, SEEBishop, SEERook, SEEQueen;
};

class TuneOptions : public UCIOptions {
    public:
        bool setOption(const std::string &name, int value) override {
            UCIOptionSpin *option = findOptionSpin(name);

            if (!option)
                return false;

            option->current = value;

            return true;
        };

        int getValue(const char* name)  {
            UCIOptionSpin *option = findOptionSpin(name);

            if (!option) {
                std::cout << "No such option " << name << std::endl;
                return 0;
            }

            return int(option->current);
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

#define UPDATEFLOAT(x) tune.x = float(getValue(#x)) / 100;
#define UPDATEINT(x) tune.x = getValue(#x);
#define TUNEFLOAT(w, x, y, z) if (!initialized) {spinOptions.push(UCIOptionSpin(#w, RANGE(int(x * 100), int(y * 100), int(z * 100))));} UPDATEFLOAT(w)
#define TUNEINT(w, x, y, z) if (!initialized) {spinOptions.push(UCIOptionSpin(#w, RANGE(x, y, z)));} UPDATEINT(w)

extern TuneOptions tuneOptions;
extern Tune tune;

inline void TuneOptions::init() {
    TUNEFLOAT(LMRBase, 0.66, 0.01, 2)
    TUNEFLOAT(LMRDiv, 2.10, 0.01, 4)
    TUNEFLOAT(LMRImproving, 0.46, 0, 2)
    TUNEINT(LMRDepth, 1, 1, 4)
    TUNEINT(LMRMovecount, 2, 1, 4)

    TUNEINT(AspiBase, 79, 1, 160)
    TUNEINT(AspiLo, 23, 1, 200)
    TUNEINT(AspiHi, 34, 1, 300)
    TUNEINT(AspiDepth, 2, 2, 10)
    TUNEFLOAT(AspiWide, 1.23, 1.01, 3.00)

    TUNEINT(RFPBase, 100, 1, 300)
    TUNEINT(RFPImproving, 164, 1, 400)
    TUNEINT(RFPDepth, 10, 3, 15)
    TUNEINT(RFPWorse, 43, 1, 150)

    TUNEINT(NMPDepth, 2, 1, 10)
    TUNEINT(NMPSeThreshold, 290, 1, 500)
    TUNEINT(NMPDepthThreshold, 6, 1, 12)
    TUNEINT(NMPBaseRed, 4, 1, 6)

    TUNEINT(ExtKeep, 13, 1, 30)

    TUNEINT(MCPDepth, 4, 1, 24)
    TUNEINT(MCPBase, 768, 256, 1536)
    TUNEINT(MCPImproving, 256, 1, 768)
    TUNEINT(MCPQuarterMul, 10, 1, 20)
    TUNEINT(MCPDepthMul, 11, 5, 17)

    TUNEINT(FPDepth, 7, 1, 24)
    TUNEINT(FPMult, 212, 1, 400)
    TUNEINT(FPBase, 192, 1, 350)
    TUNEINT(FPQuarter, 198, 1, 350);

    TUNEINT(HistDepth, 6, 1, 20)
    TUNEINT(HistMult, -6011, -9000, -1)
    TUNEINT(HistQuarter, -6305, -9000, -1)

    TUNEINT(SEDepth, 8, 4, 12)
    TUNEINT(SETTDepthDiff, 3, 0, 5)
    TUNEINT(SESBBase, 12, 0, 24)
    TUNEINT(SESBMC, 2, 1, 3)
    TUNEINT(SESBMCMax, 12, 1, 24)

    TUNEINT(LMRHistPos, 4085, 1000, 20000)
    TUNEINT(LMRHistNeg, 25329, 10000, 30000)
    TUNEINT(LMRNightmareMargin, 100, 20, 180);
    TUNEINT(LMRNightmareMovecount, 3, 1, 6);

    TUNEINT(QsSEEMargin, -108, -200, -1)
    TUNEINT(QsDeltaMargin, 147, 1, 300)

    TUNEINT(HistDepthMult, 16, 8, 22)
    TUNEINT(HistMax, 1670, 1, 3000)
    TUNEINT(HistLimit, 15, 1, 20)

    TUNEINT(SEEPawn, 81, 1, 1200)
    TUNEINT(SEEKnight, 257, 1, 1200)
    TUNEINT(SEEBishop, 325, 1, 1200)
    TUNEINT(SEERook, 491, 1, 1200)
    TUNEINT(SEEQueen, 972, 1, 1200)

    initialized = true;
}

#endif

#endif //MOLYBDENUM_TUNE_H
