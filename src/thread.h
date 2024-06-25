#ifndef MOLYBDENUM_THREAD_H
#define MOLYBDENUM_THREAD_H

#include "search.h"

#include <thread>
#include <vector>
#include <atomic>

class ThreadPool;

class Thread {
    
    std::thread thread;
    int         threadId;

public:
    ThreadPool* threads;
    std::atomic_bool searching;
    SearchState state;

    void start(Position &pos, SearchTime &st, int depth);
    void join();
    void stop();
    void clear();
    void initSearchInfo(SearchTime st);
    void detach();
    bool done();
    int id() const;
    u64  nodes();

    Thread(ThreadPool* t, int i) {
        threadId     = i;
        threads      = t;
        searching.store(false);
        state.clearHistory();
    }

    Thread(const Thread &t) {
        threadId = t.id();
        threads  = t.threads;
        searching.store(false);
        state.clearHistory();
    }
};

class ThreadPool {
    std::vector<Thread> threads;

public:
    void start(Position &pos, SearchTime &st, int depth);
    void join();
    void stop();
    void set(int count);
    void clear();
    bool done();

    u64 nodes();
};

#endif