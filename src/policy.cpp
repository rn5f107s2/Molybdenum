#include "policy.h"

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

#ifndef POLICYFILE
#define POLICYFILE // silence syntax highlighting
#endif

INCBIN(policy, POLICYFILE);

void Network::loadDefault() {
    size_t size = 1365504;
    std::string w(reinterpret_cast<const char*>(gpolicyData), size);
    std::stringstream weights(w);
    loadWeights(weights);
    std::cout << weights.eof() << std::endl;
}