#include "threads.h"
#include "threadpool.h"

ThreadPool threads;

void ThreadPool::join() {
    for (auto &t : threads)
        t.join();
}

void ThreadPool::init() {
    threads.clear();

    for (int i = 0; i < threadCount; i++)
        threads.push_back(Thread(i));
}

void ThreadPool::start(Position pos, searchTime st, int depth) {
    for (auto &t : threads)
        t.start(pos, st, depth);
}

void ThreadPool::stop() {
    for (auto &t : threads)
        t.stop();
}

u64 ThreadPool::nodes() {
    u64 nodes = 0;

    for (auto &t : threads)
        nodes += t.nodes();

    return nodes;
}

void setCount(int count) {
    threads.setCount(count);
}