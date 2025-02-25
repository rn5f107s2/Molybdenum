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

    const std::string weightString = "XMUXRWXUTXRVZSVPZOWcNVYa]^W^_\\Y]cWYUguirumfh[TSP_TPRQ[RLQSROJLMOLKPON MMSOQJRMMMQNMCTOTQQRDEFGEIUMPLMLW]JVP\\PUQQTPRXQSXSULTNYPW_QXMZT]]`b]Ya`djTiysgonntmLFQGBZEIVNDPOCOO@PPJOQLOSQQONONNKQQOW;ZX:XPCQNNPTGRPMTNMSOMPPTSMULMWIESFPRPQNQOOOKSJP/QMWNMNNLKKM6HPPQSKTNPNQLOQMSPNMYYY[X\\V]VOUW^XTXNZSWXS]XWXUUW]cdaSUa_kTq~t_tiQLSOUU\\[GUZZ\\W\\VX[R]JMPUTTWOUOOOQTUSZYQL\\NJWOgj``{X\\n[JJPJKJZJ[@XHDYFHYKQPSTQNQVP8WMV&UMLB57DLNPMOOEBLRLWNQROQMNMNPNNV\\;bf`POIHH\\VIJFOENONT[MLLJNJgX^SU]NKHM[}c^mQbUI]GA+ @NIGMYcICGPMVJeTP^YONJPMRUO[GVZTLGPLUKKLLC>hK_OPMQXUO0SP<OT?SM<MOOVQOSPGMRRONMNTIOQAQR@NQQQQEPI@KPPLOWTUPPPXPWRUSSTQUQJZMPGOXYURXWSZSZYRY_b]gahcY^neh~pf{iCFFQCOKCWPOOQLRUMVNN>APRNB@HOJMMPOOOQTQPXPMKAWONMVSHGPTQQP9ULMURORVS]X[YP^[ZYXVVV^TXS_VX\\X^WSVRX]UVqzifXctjiVN]^WJJ[OcKcMPGVPIM_PXQOWPZZFOISXRPQPJMh[XWFYREO|OnUFRZRLEGKHTTDOMSfVSXTNQMJMPGTSMXOLJOLTJPkGJMJISTJ~IIHLH\\NU/MRNQSRJUYWTW[T[Z^X[XS\\^^[WYZVX_ZZTZWTWW[ZYYZWW^^ZSY]WXY[ZUUYSYSYRX\\[WVT]Y]`_[\\[\\W]^`[\\^ZYZ^]\\`^WWUVYUY[]ZWYYSZ\\[YW\\YV^fZTWWXTWd\\bVWVUVZURNPSNQU8?<>?@9@DBHD>CC=DFHDEBCF@EF@@BBB>FFAACD?BHJJIJJHBBGDJIA@A@GA@E?BVZRUSPTV^YUV[ZYT[ZUUXWXWYXWZ]XXZ_[\\]ZZZ]ZZZZ[YYW^\\[Z]Z\\[`WY\\[SS\\e\\X\\YY_fLTNMMKPSQLNKLLKTMMLKMMJNRPMNMNORUSPQPRR_[VRSRXYfc\\]\\`a_t?CFGAAA?CAHEEHED>CGIHLDA@DFCGGB=BFEBDHCACHFHEIFC>DH@<DAA9CB>@E?7PBEEFHCLCHD>DFIFFJLGJOKKDDIDGKEKJEKFEJHJKINKKOJKI=KCEIACMFOHIPEIHWJUTOQE1EBGGDD1BSORRPTFG[RTVRVJIWQVVRXLITMUTRVJMZOWXVZJWf[^]]fZM";

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

    skip = 0;

    float pieceWeights[] = { -0.300395, 0.759094, 0.56186, 0.8178, 1.3259 };

    for (int i = PAWN; i < KING; i++)
        skip += (__builtin_popcountll(bitboards[i]) - __builtin_popcountll(bitboards[i + 6])) * pieceWeights[i];

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

    return (skip + output) * 133;
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
