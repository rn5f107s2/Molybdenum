#include "thread.h"

void Thread::start(Position &pos, SearchTime &st, int depth) {
    state.thread = this;
    thread = std::thread{[this, pos, st, depth]() mutable { state.startSearch(pos, st, depth); }};
}

void Thread::join() {
    if (thread.joinable())
        thread.join();
}

void Thread::stop() {
    state.si.stop.store(true, std::memory_order_relaxed);
}

void Thread::clear() {
    state.clearHistory();
}

void Thread::initSearchInfo(SearchTime st) {
    state.si.clear();
    state.si.st = st;
}

void Thread::detach() {
    while (!thread.joinable()) {}

    thread.detach();
}

int Thread::id() const {
    return threadId;
}

bool Thread::done() {
    return !searching.load(std::memory_order_relaxed);
}

u64 Thread::nodes() {
    return state.si.nodeCount.load(std::memory_order_relaxed);
}

void ThreadPool::start(Position &pos, SearchTime &st, int depth) {
    // Before starting the threads set all of them to searching, to avoid the mainthread stopping search
    // while other threads havent been started yet, also reset SearchInfo here, to avoid reading old nodecounts
    for (auto &t : threads) {
        t.searching.store(true, std::memory_order_relaxed);
        t.initSearchInfo(st);
    }

    for (auto &t : threads)
        t.start(pos, st, depth);
}

void ThreadPool::join() {
    for (auto &t : threads)
        t.join();
}

void ThreadPool::stop() {
    for (auto &t : threads)
        t.stop();
}

void ThreadPool::set(int count) {
    join();
    threads.clear();
    int id = 0;

    for (int i = 0; i < count; i++)
        threads.push_back(Thread(this, id++));
}

void ThreadPool::clear() {
    for (auto &t : threads)
        t.clear();
}

u64 ThreadPool::nodes() {
    u64 sum = 0;

    for (auto &t : threads)
        sum += t.nodes();

    return sum;
}

bool ThreadPool::done() {
    for (auto &t : threads)
        if (!t.done())
            return false;

    return true;
}