#ifndef MOLYBDENUM_UCI_H
#define MOLYBDENUM_UCI_H

#include "Move.h"

void uciCommunication(int argc, const std::string &in = "");

inline bool contains(const std::string& input, const std::string& searchedTerm) {
    return input.find(searchedTerm) != std::string::npos;
}

inline std::string extractFEN(const std::string& input) {
    std::string fen;
    int startPoint = (int) input.find("fen ") + 4;
    int endPoint = contains(input, "moves") ? (int) input.find("moves") - 14 : (int) input.length();

    return input.substr(startPoint, endPoint);
}

#endif //MOLYBDENUM_UCI_H
