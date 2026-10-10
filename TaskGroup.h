#include "Pool.h"

class TaskGroup {
    std::shared_ptr<Pool> pool;
    std::atomic<uint32_t> childrenCount{0};
public:
    TaskGroup(std::shared_ptr<Pool> pool);
    ~TaskGroup();

    void submit(const Task &task);
    void wait() const;
};
