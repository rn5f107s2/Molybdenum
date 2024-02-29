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
    int LMRDepth, LMRMovecount, LMRHistDivPos, LMRHistDivNeg;
    int AspiBase, AspiLo, AspiHi, AspiDepth;
    float AspiWide;
    int RFPBase, RFPImproving, RFPDepth, RFPOppWorsening;
    int RFPCBase, RFPCImproving, RFPCDepth, RFPCOppWorsening, RFPCMargin;
    int NMPDepth, NMPSeThreshold, NMPDepthThreshold, NMPBaseRed;
    int MCPDepth, MCPMultiplier, MCPFractDepthMult;
    int FPDepth, FPBase, FPMult, FPFractDepthMult;
    int HistDepth, HistMult, HistFractDepthMult;
    int SeDepth, SeTTDepth, SbMargin;
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


#define RANGE(x, y, z) int(y), int(z), int(x), reinterpret_cast<void (*)(int)>(x)

#define UPDATEFLOAT(x) tune.x = float(tuneOptions.getValue(#x)) / 100;
#define UPDATEINT(x) tune.x = tuneOptions.getValue(#x);
#define TUNEFLOAT(w, x, y, z) if (!initialized) {spinOptions.push(UCIOptionSpin(#w, RANGE(int(x * 100), int(y * 100), int(z * 100))));} UPDATEFLOAT(w)
#define TUNEINT(w, x, y, z) if (!initialized) {spinOptions.push(UCIOptionSpin(#w, RANGE(x, y, z)));} UPDATEINT(w)

extern TuneOptions tuneOptions;
extern Tune tune;

inline void TuneOptions::init() {
    TUNEFLOAT(LMRBase, 0.66, 0.01, 1)
    TUNEFLOAT(LMRDiv, 2.02, 0.01, 3)
    TUNEFLOAT(LMRImproving, 0.49, 0, 1)
    TUNEINT(LMRDepth, 2, 1, 4)
    TUNEINT(LMRMovecount, 2, 1, 4)
    TUNEINT(LMRHistDivPos, 4085, 1000, 20000)
    TUNEINT(LMRHistDivNeg, 25329, 10000, 30000)

    TUNEINT(AspiBase, 81, 1, 160)
    TUNEINT(AspiLo, 28, 1, 100)
    TUNEINT(AspiHi, 34, 1, 100)
    TUNEINT(AspiDepth, 2, 2, 10)
    TUNEFLOAT(AspiWide, 1.24, 1.01, 2)

    TUNEINT(RFPCBase, 101, 1, 300)
    TUNEINT(RFPCImproving, 180, 1, 400)
    TUNEINT(RFPCDepth, 10, 3, 15)
    TUNEINT(RFPCOppWorsening, 40, 0, 100)
    TUNEINT(RFPCMargin, 200, 0, 400)

    TUNEINT(NMPDepth, 2, 1, 10)
    TUNEINT(NMPSeThreshold, 276, 1, 500)
    TUNEINT(NMPDepthThreshold, 6, 1, 12)
    TUNEINT(NMPBaseRed, 4, 1, 6)

    TUNEINT(MCPDepth, 4, 1, 24)
    TUNEINT(MCPMultiplier, 11, 1, 24)
    TUNEINT(MCPFractDepthMult, 11, 1, 24)

    TUNEINT(FPDepth, 7, 1, 24)
    TUNEINT(FPMult, 203, 1, 400)
    TUNEINT(FPBase, 188, 1, 350)
    TUNEINT(FPFractDepthMult, 203, 1, 400)

    TUNEINT(HistDepth, 5, 1, 20)
    TUNEINT(HistMult, -6009, -9000, -1)
    TUNEINT(HistFractDepthMult, -6009, -9000, -1)

    TUNEINT(SeDepth, 8, 4, 12)
    TUNEINT(SeTTDepth, 3, 1, 5)
    TUNEINT(SbMargin, 25, 1, 100)

    TUNEINT(QsSEEMargin, -96, -200, -1)
    TUNEINT(QsDeltaMargin, 137, 1, 300)

    TUNEINT(HistDepthMult, 16, 1, 32)
    TUNEINT(HistMax, 1638, 1, 3000)
    TUNEINT(HistLimit, 13, 1, 20)

    TUNEINT(SEEPawn, 81, 1, 300)
    TUNEINT(SEEKnight, 257, 100, 500)
    TUNEINT(SEEBishop, 325, 100, 500)
    TUNEINT(SEERook, 491, 300, 700)
    TUNEINT(SEEQueen, 972, 700, 1200)

    initialized = true;
}

#endif

#endif //MOLYBDENUM_TUNE_H
