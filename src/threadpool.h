#ifndef THREADPOOL_H_INCLUDED
#define THREADPOOL_H_INCLUDED

#include <thread>
#include <vector>

#include "search.h"
#include "Position.h"
#include "timemanagement.h"
#include "threads.h"

class ThreadPool {
    std::vector<Thread> threads;

    int threadCount = 1;

public:
    void join();
    void start(Position pos, searchTime st, int depth);
    void stop();
    void init();
    u64 nodes();

    void setCount(int count) {
        threadCount = count;
        init();
    }

    ThreadPool() {
        init();
    }
};

extern ThreadPool threads;

void setCount(int count);

#endif