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

    const std::string weightString = "UUUVU`XTWRB8SETFEGQG1FPPOFSSR[DKTG@F[HITOfIHJDSDWQP=<8UUVTSWNPOSX\\P`a]Q[XYY[W`U^WX_iW^Za_fotod~`t~|QWNXVNOOP_l`HQDIWEALEOSLIQLFOEOXTLZGP]>Fm[I`M@a9G~iDyDT\\ULGRNJBNKYSK[RPIiShETPSTIVT[O?PYIR]FmU@PZEOwFmY:He:[YkFOVW=ISYQQXT\\OSNJQMMIUNRLERQR-TLSNOMPKLOLYMNFPO?PN\\OMMPPAQOLLMIRVLXZtSPSU\\LTVDZTOSY]VONP`aY]jaP8[\\kl|~|PDZUOVRKTa\\AWT\\ZY[YWZSjPc\\a^ZXRaUaabUdOblhdflQja|~vy~xq~rGLKXPKOKORRQGUFMTKPDNHHKORQUBS@S>SAR>MMNMMQOMNFPNONSPQROSPPPQQUQKLVHSRGQJPMPNRNEODSMNLOFKPNJPNKRLTMNNNLHLMIOMXLPIOAB?SPPUVTPXU[^VdbZ`d_`bX]`^bX]]oTdfbaebz~g}~ux}tKGRJMJYGGCUDCSEGXHOTMQLSKKMLPISBUHSJLPMHKHLJJOLSPKPNILUYPQMLMTQSPTSMSKVO\\^U\\eYJgI^YdRdY[]Y_i`ejdhi^v~vp~vn~pHDLDDDDD\\HYIKIJKDOF[GOXWMJMI[PWZaKJPTWSfoYQPNPjWx~}M_WKMKIK[EFIGJH@^@E\\GOWOSRSR]RNRNPBPPGVZXO:MDOMMWWQX?QQRUPON]VTYYWZR\\YXYURR[Wa^daW^VW]]\\\\_\\f_^gecjccmfp~vw~u~~}[TNLROONNPDOPDPRLUPIOPOPBO@QKPLNKOPOINKOJNOGNOINKHKOJLR:PQPOOPQPgZSWZVWOWSSMNXSNSMNROIELJLKILGJDGFDBFDD=GCD@BC;@FECDHFA9=ACBG@:;BAFIJI:_NZUVTQfV?J@AIF[VGKIILL[XBIGGIIZWBLILNG[YHLLLMK\\bLPONPObbR^YWXZj21<77<1.@=C@AB:?HGNLLOIKLJNLNOKNMKMOPNLLONPQQRMPcK\\WXYN\\<EFFBDA@_`X\\]W`]b_]][W[`ZWVYZXTVb_]bc]__dbaba^_bb\\`]_^[ag_Y[[X\\aXRSVPQVV5EK:>D9;:?G@@FE@>CHACFEEACI@BFB@BANCCKGEBEKFGHHFB@CCAFBACGKCAFBB5HBDFLH>:<@B;B?9AFGDDFE>8DCBBEB:?DCBCGD;DGDEEECB:GD@>A@=BBF=?DC@g^X_d_edYUUW[W][ZXXYZ[[YZ[Z\\[Y]_XUWXWUVWUSSUVSRTTOOPNKPOn^cW]``ga[[a`Z]^b_^]aZacZXSZ[XY\\d^V\\[U^`_]Y[YX^`YXVZZXY^hg^ihahcagemlficQ";

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
