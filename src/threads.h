#ifndef THREADS_H_INCLUDED
#define THREADS_H_INCLUDED

#include "search.h"

#include <thread>

class Thread {
    std::thread thread;
    SearchState state;

public:
    void start(Position& pos, searchTime& st, int depth);
    void stop();
    void join();
    u64 nodes();

    Thread(int id) {
        state.id = id;
    }
};

#endif
