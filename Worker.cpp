#include "Worker.h"

#include <algorithm>

Worker::Worker(
    std::deque<Task> &taskDeque,
    std::mutex &globalMutex,
    std::condition_variable &cv
) : taskDeque(taskDeque),
    globalMutex(globalMutex),
    cv(cv) {
}

Worker::~Worker() {
    state = WorkerState::CANCELLED;
    workerThread.join();
}

void Worker::run() {
    workerThread = std::thread([this]() {
        for (;;) {
            std::unique_lock<std::mutex> lock(globalMutex);
            cv.wait(lock, [this]() {
                return state == WorkerState::CANCELLED || state == WorkerState::WORKING
                       || !taskDeque.empty();
            });
            switch (state) {
                case WorkerState::WORKING:
                    if (taskDeque.empty()) {
                        state = WorkerState::IDLE;
                    }
                    break;
                case WorkerState::CANCELLED:
                    return;
            }
            lock.unlock();
            runTask();
        }
    });
}

bool Worker::runTask() {
    std::unique_lock<std::mutex> lock(mutex);
    if (taskDeque.empty()) {
        return false;
    }
    Task task = std::move(taskDeque.front());
    taskDeque.pop_front();
    lock.unlock();
    task();
    return true;
}

void Worker::wake() {
    state = WorkerState::WORKING;
}
