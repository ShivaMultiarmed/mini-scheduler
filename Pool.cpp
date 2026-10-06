#include "Pool.h"

Pool::Pool(uint32_t workerCount)
    : workerCount(workerCount) {
}

Pool::~Pool() {
    cancel();
}

bool Pool::submit(const Task &task) {
    {
        std::unique_lock<std::mutex> lock(mutex);
        uint32_t currentWorker = nextWorker.fetch_add(1) % workerCount;
        workers[currentWorker]->submit(task);
    }
    cv.notify_one();
    return true;
}

void Pool::run() {
    for (uint32_t i = 0; i < workerCount; ++i) {
        workers.push_back(new Worker(mutex, cv));
    }
    for (uint32_t i = 0; i < workerCount; ++i) {
        workers[i]->connectWithSiblings(&workers);
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
        worker -> cancel();
    }
    for (auto worker : workers) {
        delete worker;
    }
}