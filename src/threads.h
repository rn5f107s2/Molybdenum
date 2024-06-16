#ifndef THREADS_H_INCLUDED
#define THREADS_H_INCLUDED

#include <thread>
#include <vector>

#include "search.h"
#include "Position.h"
#include "timemanagement.h"

class ThreadPool {
    std::vector<std::thread> threads;

    int threadCount = 1;

public:
    void join();
    void start(Position pos, searchTime st, int depth);

    void setCount(int count) {
        threadCount = count;
    }
};

extern ThreadPool threads;

void setCount(int count);

#endif