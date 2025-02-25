#include "nnue.h"
#include "Constants.h"
#include "BitStuff.h"
#include "Utility.h"
#include <cstring>
#include <fstream>
#include <tuple>
#include <cmath>

#ifdef _MSC_VER
#define PUSHED_MACRO
#pragma push_macro("_MSC_VER")
#undef _MSC_VER
#endif

#include "incbin/incbin.h"

#ifdef PUSHED_MACRO
#pragma pop_macro("_MSC_VER")
#undef PUSHED_MACRO
#endif

#ifndef EVALFILE
#define EVALFILE // silence syntax highlighting
#endif

#ifndef WDLFILE
#define WDLFILE // silence syntax highlighting
#endif

INCBIN(network, EVALFILE);
INCBIN(wdlHead, WDLFILE);

const Weights defaultWeights = *reinterpret_cast<const Weights*>(gnetworkData);
const WDLHead defaultWdl     = *reinterpret_cast<const WDLHead*>(gwdlHeadData);

void Net::loadDefaultNet() {
    weights0 = defaultWeights.weights0;
    weights1 = defaultWeights.weights1;
    bias0 = defaultWeights.bias0;
    bias1 = defaultWeights.bias1;

    wdlWeights = defaultWdl.weights1;
    wdlBias    = defaultWdl.bias1;

    const std::string weightString = "POPPRWUTS`T]`bTLbZ\\fR\\`[_WRac^]bfR\\Qo~ns~m`kWLJOVJJVQbSJWWG\\PGMOAONKRDL@RNTMMOKRAPERROTUHTIFOPMPRMQRMRRTO\\OTPPONPLOROV^Q[HWQ[T^eYXKZU`bdccZ`cjkdr|ukqnt}uKVFJBWDEOMKHJMGQEOPNMOOPNPRQKQLRLNHQVTVS;STLFPPMPIOMHLNNQOSNRQQKVLLVKGQHMSNOMPOPOKQIO;OLUMOMPNKLP?JQPQSJRORNRRORSPNMMVVU]XZ\\^WT[Sg^YWP^ZRTXc`T[PUe\\kidWX`j~Z~~~ZtbYHQVWTabQUUXYSWRVVLfKONNU_VK_KNNRSaVXaRPcL^fKcx\\]~[p~oKLKNYWRFPJXKDXDGZIOPQQFM>WNNUQT@IBNN=:LOLNIRNBDMXMROPROOQNGINQP]UX][aMMHKIO]ONJRONMKSgNTIQV\\a[\\YN^QKSMc~jOqM\\TF^EF D$REGTTaIE9WKXMXTZ_ZQONRPKVJcEQWWHKVJZ]IKOA<iB_MFKUWKISVNANTCTP>QROPTOYMHLOSMOONPIOOFPP;POWPOGNMAMNOLORPSOMOORWPURRUPTPR_WW]YUhVV]ZX`XV_QVgZclcgm\\e~]s~qg~fIDUNDHLDRTROSPXWOXON<>OJONNPNRKPBQNTMNOLYMNLETONNdLPOQSOQDP]QYSTONINXUJSVW^\\ZNVTb`Za\\R\\V_^ffi]Pag^b|oztWx~~~PLZPWKO^N_ReWRTWPPZZZ\\dXZ\\\\]QYSU]YTYUJ\\kf`YK\\hJc~mpkMaNOSLNPMQQTRTNJPRWTORHSQRIQPOCSMOOCQKLZKMLNNPON^MLKNILLF4NKH4STMTYVSYZUT]_\\^\\T\\_b_Y[\\VT^\\VPVVRSU_[Y\\^WYad\\Z^^]Z``YXY[VY]ZNY^XZQWWTVa\\R[T`W\\Z][]^\\Z\\^^[baYVUWWUYYa^]]\\Zb`\\YU^XU^_XQLQQLSUfaX]_Y\\g__KRXPW]*14208,/7?F?8B@6@IMDEJEF;EH>?EA@=EGA@GE?CJMIJOJLCDKFJLC@:<H@=E>AZRPTRRYc_]U\\]Z\\\\\\YSVXVUY^WUZ]WV\\_TXZXYVa]YW\\YWX[da\\\\[[[_`R][WYP_e\\Y[XZ_hPQPPMIPYNKLIJJHVNKLKNLJQRNOMNPOWUSONNQT[\\RSRSPYe]Y[^`Yak=BGGAF?9;@E@ADD=?DGGEKD>>EDBFIC;?CFACGF>BJJHCJJD;FE=:D@A7??""<<E>8\\ENIMPCWE>GACJDBHDPGGSIMC>I@@H;AC=I@?H;AEBRHGPCH=8F=>F92OCKFHN<KY\\R`[Z_ODXPVTQWDNZNTSNYPV]UYZS[YT[U\\ZSXROXRZWQYKW\\U^[W_SggZab]m`M";

    for (int i = 0; i < 1385; i++)
        weights[i] = float(int8_t(weightString[i]) - 79) / 48.0f;
}

void Net::initAccumulator(std::array<u64, 13> &bitboards, Color c) {
    accumulatorStack.clear();

    //memcpy(&accumulator[WHITE], &bias0[0], sizeof(int16_t) * L1_SIZE);
    //memcpy(&accumulator[BLACK], &bias0[0], sizeof(int16_t) * L1_SIZE);

    //for (int pc = WHITE_PAWN; pc != NO_PIECE; pc++) {
    //    u64 pieceBB = bitboards[pc];

    //    while (pieceBB) {
    //        int square = popLSB(pieceBB);

    //        toggleFeature<On>(pc, square);
    //    }
    //}

    const int IN_CHANNEL = 12;
    const int OUT_CHANNEL = 8;
    const int KERNEL_SIZE = 3;
    const int PADDING = 1;
    const int DIM1 = 8, DIM2 = 8;

    int stm = !c * 6;

    for (int i = 0; i < 512; i++)
        accumulatorConv[i] = weights[864 + i / 64];

    for (int i = 0; i < IN_CHANNEL; i++)
        for (int j = 0; j < OUT_CHANNEL; j++)
            for (int k = PADDING; k < DIM1 + PADDING; k++)
                for (int l = PADDING; l < DIM2 + PADDING; l++)
                    for (int m = -PADDING; m < KERNEL_SIZE - PADDING; m++)
                        for (int n = -PADDING; n < KERNEL_SIZE - PADDING; n++)
                            if (k + m != 0 && k + m != 9 && l + n != 0 && l + n != 9)
                                accumulatorConv[j * 64 + (k - 1) * 8 + (l - 1)] += weights[j * 108 + i * 9 + (m + 1) * 3 + (n + 1)] * bool(bitboards[(i + stm) % 12] & (1ULL << ((k + m - 1) * 8 + (l + n - 1))));

    pushAccToStack();
}

int Net::calculate(Color c) {
    float output = weights[1384];

   //for (int n = 0; n != L1_SIZE; n++) {
    //     output += screlu(accumulator[ c][n]) * weights1[n          ];
    //     output += screlu(accumulator[!c][n]) * weights1[n + L1_SIZE];
    //}

    //return ((output / 255) + bias1[0]) * 133 / (64 * 255);

    for (int i = 0; i < 512; i++) {
        output += std::max(accumulatorConv[i], 0.0f) * weights[872 + i];
    }

    return output * 133;
}

std::tuple<float, float, float> Net::getWDL(Color c) {
    int output[3] = {0, 0, 0};
    std::tuple<float, float, float> tpl;

    for (int n = 0; n < L1_SIZE; n++) {
        for (int n2 = 0; n2 < 3; n2++) {
            output[n2] += screlu(accumulator[ c][n]) * wdlWeights[n * 3 + n2              ];
            output[n2] += screlu(accumulator[!c][n]) * wdlWeights[n * 3 + n2 + L1_SIZE * 3];
        }
    }

    float sum = 0.0f;
    float scores[3];

    for (int i = 0; i < 3; i++)
        sum += (scores[i] = std::exp(double((output[i] / 255) + wdlBias[i]) / double(64 * 255)));

    std::get<0>(tpl) = scores[0] / sum;
    std::get<1>(tpl) = scores[1] / sum;
    std::get<2>(tpl) = scores[2] / sum;

    return tpl;
}
