#include "Pool.h"

Pool::Pool() {
}

Pool::~Pool() {
    std::unique_lock<std::mutex> lock(synchroMutex);
    cancelled = true;
    lock.unlock();
    cv.notify_all();
    for (auto &thread: threads) {
        thread.join();
    }
}

bool Pool::submit(const Task &task) {
    pendingCount.fetch_add(1, std::memory_order::relaxed);
    std::unique_lock<std::mutex> lock(synchroMutex);
    taskDeque.push_back(task);
    lock.unlock();
    cv.notify_one();
    return true;
}

bool Pool::tryLaunchNext() {
    Task task;

    std::unique_lock<std::mutex> lock(synchroMutex);
    if (taskDeque.empty()) {
        return false;
    }
    task = std::move(taskDeque.front());
    taskDeque.pop_front();
    lock.unlock();

    task();
    pendingCount.fetch_sub(1, std::memory_order::release);
    return true;
}

void Pool::run() {
    for (uint32_t i = 0; i < THREAD_COUNT; i++) {
        threads.emplace_back([this]() {
            runOneThread();
        });
    }
}

void Pool::runOneThread() {
    for (;;) {
        std::unique_lock<std::mutex> lock(synchroMutex);
        cv.wait(lock, [this]() { return cancelled || !taskDeque.empty(); });
        if (taskDeque.empty()) {
            return;
        }
        lock.unlock();
        tryLaunchNext();
    }
}

void Pool::cancel() {
    cancelled = true;
}
