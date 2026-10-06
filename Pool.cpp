#include "Pool.h"

Pool::Pool() {
    for (uint32_t i = 0; i < THREAD_COUNT; ++i) {
        assignments.push_back(new std::deque<Task>());
        workers.push_back(new Worker(*assignments[i], globalMutex, cv));
    }
}

Pool::~Pool() {
    std::unique_lock<std::mutex> lock(globalMutex);
    state = PoolState::CANCELLED;
    lock.unlock();
    cv.notify_all();
    for (uint32_t i = 0; i < THREAD_COUNT; ++i) {
        delete workers[i];
    }
}

bool Pool::submit(const Task &task) {
    std::unique_lock<std::mutex> lock(globalMutex);
    uint32_t currentWorker = getCurrentWorker();
    assignments[currentWorker] -> push_back(task);
    lock.unlock();
    cv.notify_all();
    return true;
}

void Pool::run() {
    for (uint32_t i = 0; i < THREAD_COUNT; i++) {
        workers[i] -> run();
    }
}

void Pool::cancel() {
    state = PoolState::CANCELLED;
}

// Side effect: increments next worker index
uint32_t Pool::getCurrentWorker() {
    return nextWorker.fetch_add(1) % THREAD_COUNT;
}