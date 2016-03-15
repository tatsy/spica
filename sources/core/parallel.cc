#include "parallel.h"

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

class WorkerTask;

static std::vector<int> threadTasks;
static std::mutex workerListMutex;
static std::condition_variable condval;
static WorkerTask* workerTask;

class WorkerTask {
public:
    WorkerTask(const std::function<void(int)>& f, int csize, int tasks)
        : func{ f }
        , chunkSize{ csize }
        , nTasks { tasks } {
    }

    bool finished() const {
        return currentIndex >= nTasks;
    }

    const std::function<void(int)>& func;
    const int chunkSize;
    const int nTasks;
    int currentIndex = 0;
};

class WorkerTaskManager {
public:
    WorkerTaskManager(const std::function<void(int)>& f,
                      int chunkSize, int nTasks) {
        if (workerTask == nullptr) {
            workerTask = new WorkerTask(f, chunkSize, nTasks);
        }
    }

    ~WorkerTaskManager() {
        delete workerTask;
        workerTask = nullptr;
    }
};

static void workerThreadFunc(int threadIndex) {
    std::unique_lock<std::mutex> lock(workerListMutex);
    if (!workerTask->finished()) {
        if (threadTasks.empty()) {
            condval.wait(lock);
        } else {
            int indexStart = workerTask->currentIndex;
            int indexEnd   = std::min(workerTask->nTasks, indexStart + workerTask->chunkSize);
            workerTask->currentIndex = indexEnd;
            lock.unlock();

            for (int i = indexStart; i < indexEnd; i++) {
                workerTask->func(threadTasks[i]);
            }
            lock.lock();
        }
    }
}

void parallel_for(int start, int end, const std::function<void(int)>& func,
                  ParallelSchedule schedule) {
    const int nTasks = (end - start);
    const int nThreads = numAvailableThreads();
    const int chunkSize = (nTasks + nThreads - 1) / nThreads;
    WorkerTaskManager manager(func, chunkSize, nTasks);    

    std::vector<std::thread> threads;
    for (int i = 0; i < nThreads; i++) {
        threads.emplace_back(workerThreadFunc, i + 1);
    }

    workerListMutex.lock();
    threadTasks.resize(nTasks);
    for (int i = 0; i < nTasks; i++) {
        threadTasks[i] = start + i;
    }
    workerListMutex.unlock();
    condval.notify_all();

    for (auto& t : threads) {
        t.join();
    }
}

int numAvailableThreads() {
    return std::max(1u, std::thread::hardware_concurrency());
}
