#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>

using Task = std::function<void()>;

class Pool {
    uint32_t THREAD_COUNT = 4;
    std::vector<std::thread> threads;
    std::condition_variable cv;
    std::deque<Task> taskDeque;
    std::mutex synchroMutex;
    std::atomic<int64_t> pendingCount{0};
    bool cancelled = false;
    bool tryLaunchNext();
    void runOneThread();
public:
    Pool();
    ~Pool();
    bool submit(const Task& task);
    void run();
    void cancel();
};
