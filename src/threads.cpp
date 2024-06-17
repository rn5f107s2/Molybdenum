#include "threads.h"

#include <thread>

void foo(SearchState* state, Position pos, searchTime st, int depth) {
    state->startSearch(pos, st, depth);
}

void Thread::start(Position pos, searchTime st, int depth) {
    thread = std::thread(&SearchState::startSearch, &state, pos, st, depth);
    //thread = std::thread([&](){ state.startSearch(pos, st, depth); } );
    //thread = std::thread(foo, &state, pos, st, depth);
}

void Thread::stop() {
    state.si.stop.store(true, std::memory_order_relaxed);
}

u64 Thread::nodes() {
    return state.si.nodeCount.load(std::memory_order_relaxed);
}

void Thread::join() {
    thread.join();
}