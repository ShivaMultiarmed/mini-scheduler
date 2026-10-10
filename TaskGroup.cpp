#include "TaskGroup.h"

TaskGroup::TaskGroup(std::shared_ptr<Pool> pool) {
    this -> pool = pool;
}

TaskGroup::~TaskGroup() {
    wait();
};

void TaskGroup::submit(const Task &task) {
    childrenCount.fetch_add(1);
    pool -> submit([this, task](){
        task();
        childrenCount.fetch_sub(1);
    });
}

void TaskGroup::wait() const {
    while (childrenCount.load() > 0) {
        if (!pool -> runTask()) {
            std::this_thread::yield();
        }
    }
}
