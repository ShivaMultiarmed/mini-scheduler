#include "Pool.h"

thread_local Worker* currWorker = nullptr;

Pool::Pool(uint32_t workerCount)
    : workerCount(workerCount) {
}

Pool::~Pool() {
    cancel();
}

bool Pool::submit(const Task &task) {
    {
        std::unique_lock<std::mutex> lock(mutex);
        Worker* currentWorker = currWorker ? currWorker : workers[nextWorker.fetch_add(1) % workerCount];
        currentWorker -> submit(task);
    }
    cv.notify_one();
    return true;
}

void Pool::run() {
    for (uint32_t i = 0; i < workerCount; ++i) {
        workers.push_back(new Worker(mutex, cv, &workers));
    }
    for (uint32_t i = 0; i < workerCount; i++) {
        workers[i]->run();
    }
}

void Pool::cancel() {
    {
        std::unique_lock<std::mutex> lock(mutex);
        state = PoolState::CANCELLED;
    }
    cv.notify_all();
    for (auto worker : workers) {
        worker -> join();
    }
    for (auto worker : workers) {
        delete worker;
    }
}

bool Pool::runTask() {
    Worker* currentWorker = currWorker ? currWorker : workers[nextWorker.fetch_add(1) % workerCount];
    return currentWorker->runTask();
}
