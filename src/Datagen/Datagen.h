#ifndef MOLYBDENUM_DATAGEN_H
#define MOLYBDENUM_DATAGEN_H

#include <chrono>
#include "../Constants.h"
#include "../Position.h"

static u64 seedDataGen = 0;

static void init() {
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto epoch = now_ms.time_since_epoch();
    auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
    seedDataGen = value.count();
}

bool verifyExit(Position &pos);
void createExit(Position &pos);
void playGame(Position &pos, const std::string& filename, u64 &fenCount);

[[noreturn]] void start(Position &pos, const std::string& filename);

const int DRAW_ADJ = 0;
const int WIN_ADJ  = 1;

#endif //MOLYBDENUM_DATAGEN_H