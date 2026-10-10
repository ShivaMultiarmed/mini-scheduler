#include "Worker.h"
#include <algorithm>
#include <optional>

extern thread_local Worker *currWorker;

Worker::Worker(
    std::mutex &globalMutex,
    std::condition_variable &cv,
    std::vector<Worker *> *siblings
) : globalMutex(globalMutex),
    cv(cv),
    siblings(siblings) {
}

Worker::~Worker() {
    std::unique_lock<std::mutex> lock(globalMutex);
    WorkerState curState = state;
    lock.unlock();
    if (curState != WorkerState::JOINED) {
        join();
    }
}

void Worker::run() {
    workerThread = std::thread([this]() {
        currWorker = this;
        for (;;) {
            if (runTask()) {
                continue;
            }
            {
                std::unique_lock<std::mutex> lock(globalMutex);
                cv.wait(lock, [this]() {
                    return state == WorkerState::JOINED || anyWork();
                });
                if (state == WorkerState::JOINED && !anyWork()) {
                    return;
                }
            }
        }
    });
}

std::optional<Task> Worker::popFront() {
    std::unique_lock<std::mutex> lock(mutex);
    if (taskDeque.empty()) {
        return std::nullopt;
    }
    Task task = std::move(taskDeque.front());
    taskDeque.pop_front();
    lock.unlock();
    return std::optional(task);
}

bool Worker::runTask() {
    std::optional<Task> task = popFront();
    if (!task.has_value()) {
        const uint32_t workerCount = siblings->size();
        for (uint32_t i = 0; i < workerCount; i++) {
            if (siblings->at(i) != this) {
                task = steal(siblings->at(i));
                if (task.has_value()) {
                    (*task)();
                    return true;
                }
            }
        }
        return false;
    }
    (*task)();
    return true;
}

void Worker::submit(const Task &task) {
    std::unique_lock<std::mutex> lock(mutex);
    taskDeque.push_back(task);
}

std::optional<Task> Worker::popBack() {
    std::unique_lock<std::mutex> lock(mutex);
    if (taskDeque.empty()) {
        return std::nullopt;
    }
    Task task = std::move(taskDeque.back());
    taskDeque.pop_back();
    return std::optional(task);
}

std::optional<Task> Worker::steal(Worker *worker) {
    return worker->popBack();
}

void Worker::join() {
    {
        std::unique_lock<std::mutex> lock(globalMutex);
        state = WorkerState::JOINED;
    }
    cv.notify_all();
    if (workerThread.joinable()) {
        workerThread.join();
    }
}

bool Worker::hasWork() {
    std::unique_lock<std::mutex> lock(mutex);
    return !taskDeque.empty();
}

bool Worker::anyWork() {
    for (auto sibling: *siblings) {
        if (sibling->hasWork()) {
            return true;
        }
    }
    return false;
}
