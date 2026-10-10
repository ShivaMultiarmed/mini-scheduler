#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <optional>

using Task = std::function<void()>;

enum class WorkerState {
    IDLE, STEALING, WORKING, JOINED
};

class Worker {
    WorkerState state = WorkerState::IDLE;
    std::deque<Task> taskDeque;
    std::mutex &globalMutex, mutex;
    std::condition_variable &cv;

    std::thread workerThread;

    std::optional<Task> popFront();
    std::optional<Task> popBack();
    std::vector<Worker*>* siblings;

public:
    void run();
    bool runTask();
    void join();

    void submit(const Task& task);
    std::optional<Task> steal(Worker* worker);

    Worker(
        std::mutex &globalMutex,
        std::condition_variable &cv,
        std::vector<Worker *> *siblings
    );

    Worker(Worker &&a) = delete;

    bool hasWork();
    bool anyWork(); // not only in the current worker but stolen as well

    ~Worker();
};
