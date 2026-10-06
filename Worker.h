#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>

using Task = std::function<void()>;

enum class WorkerState {
    IDLE, STEALING, WORKING, CANCELLED
};

class Worker {
    WorkerState state = WorkerState::IDLE;
    std::deque<Task> &taskDeque;
    std::mutex &globalMutex, mutex;
    std::condition_variable &cv;

    bool runTask();

    std::thread workerThread;

public:
    void run();

    void wake();

    Worker(
        std::deque<Task> &taskDeque,
        std::mutex &globalMutex,
        std::condition_variable &cv
    );

    Worker(Worker &&a) = delete;

    ~Worker();
};
