#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <optional>

using Task = std::function<void()>;

enum class WorkerState {
    IDLE, STEALING, WORKING, CANCELLED
};

class Worker {
    WorkerState state = WorkerState::IDLE;
    std::deque<Task> taskDeque;
    std::mutex &globalMutex, mutex;
    std::condition_variable &cv;

    bool runTask();

    std::thread workerThread;

    std::optional<Task> popFront();
    std::optional<Task> popBack();
    std::vector<Worker*>* siblings;

public:
    void connectWithSiblings(std::vector<Worker*>* siblings);
    void run();
    void join();
    void cancel();

    void submit(const Task& task);
    std::optional<Task> steal(Worker* worker);

    Worker(
        std::mutex &globalMutex,
        std::condition_variable &cv
    );

    Worker(Worker &&a) = delete;

    bool hasWork();
    bool anyWork(); // not only in the current worker but stolen as well

    ~Worker();
};
