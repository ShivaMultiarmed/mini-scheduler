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
    const uint32_t workerCount;
    std::condition_variable cv;
    std::mutex mutex;
    std::vector<Worker*> workers;
    std::atomic<int32_t> nextWorker{0};
    PoolState state = PoolState::ACTIVE;
public:
    Pool(const uint32_t workerCount = 4);
    ~Pool();
    bool submit(const Task& task);
    void run();
    void cancel();

    bool runTask();
};
