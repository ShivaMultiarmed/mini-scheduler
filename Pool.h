#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include "Worker.h"

enum class PoolState {
    ACTIVE, CANCELLED
};

class Pool {
    uint32_t THREAD_COUNT = 4;
    std::condition_variable cv;
    std::mutex globalMutex;
    std::vector<Worker*> workers;
    std::vector<std::deque<Task>*> assignments;
    std::atomic<int32_t> nextWorker{0};
    uint32_t getCurrentWorker();
    PoolState state = PoolState::ACTIVE;
public:
    Pool();
    ~Pool();
    bool submit(const Task& task);
    void run();
    void cancel();
};
