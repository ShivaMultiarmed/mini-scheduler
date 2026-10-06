#include "Worker.h"
#include <algorithm>
#include <optional>

Worker::Worker(
    std::mutex &globalMutex,
    std::condition_variable &cv
) : globalMutex(globalMutex),
    cv(cv) {
}

Worker::~Worker() {
    cancel();
}

void Worker::run() {
    workerThread = std::thread([this]() {
        for (;;) {
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait(lock, [this]() {
                return state == WorkerState::CANCELLED || anyWork();
            });
            if (state == WorkerState::CANCELLED || !anyWork()) {
                return;
            }
            lock.unlock();
            runTask();
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
    lock.unlock();
}

std::optional<Task> Worker::popBack() {
    std::unique_lock<std::mutex> lock(mutex);
    if (taskDeque.empty()) {
        return std::nullopt;
    }
    Task task = std::move(taskDeque.back());
    taskDeque.pop_back();
    lock.unlock();
    return std::optional(task);
}

std::optional<Task> Worker::steal(Worker *worker) {
    return worker -> popBack();
}

void Worker::connectWithSiblings(std::vector<Worker *>* siblings) {
    this -> siblings = siblings;
}

void Worker::cancel() {
    std::unique_lock<std::mutex> lock(globalMutex);
    state = WorkerState::CANCELLED;
    lock.unlock();
    cv.notify_all();
    workerThread.join();
}

bool Worker::hasWork() {
    std::unique_lock<std::mutex> lock(mutex);
    return !taskDeque.empty();
}

bool Worker::anyWork() {
    for (auto sibling : *siblings) {
        if (sibling->hasWork()) {
            return true;
        }
    }
    return false;
}
