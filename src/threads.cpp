#include "threads.h"

ThreadPool threads;

void ThreadPool::join() {
    for (auto &t : threads)
        t.join();

    threads.clear();
}

void foo(SearchState state, Position pos, searchTime st, int d) {
    state.startSearch(pos, st, d);
}

void ThreadPool::start(Position pos, searchTime st, int depth) {
    int id = 0;

    for (int i = 0; i < threadCount; i++) {
        SearchState state;
        Position p = pos;

        state.id = id++;

        threads.push_back(std::thread(foo, state, p, st, depth));
    }
}

void setCount(int count) {
    threads.setCount(count);
}