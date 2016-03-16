#include "parallel.h"

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace spica {

inline uint64_t doubleToBits(double v) {
    uint64_t ui;
    memcpy(&ui, &v, sizeof(v));
    return ui;
}

inline double bitsToDouble(uint64_t b) {
    double v;
    memcpy(&v, &b, sizeof(v));
    return v;
}

AtomicDouble::AtomicDouble(double v) {
    bits = doubleToBits(v);
}

AtomicDouble::operator double() const {
    return bitsToDouble(bits);   
}

double AtomicDouble::operator=(double v) {
    bits = doubleToBits(v);
    return v;
}

void AtomicDouble::add(double v) {
    uint64_t oldBits = bits;
    uint64_t newBits;
    do {
        newBits = doubleToBits(bitsToDouble(oldBits) + v);
    } while (!bits.compare_exchange_weak(oldBits, newBits));    
}

}  // namespace spica

class WorkerTask;

static thread_local int threadID;
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
    threadID = threadIndex;
    std::unique_lock<std::mutex> lock(workerListMutex);
    while (!workerTask->finished()) {
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

            if (workerTask->finished()) {
                condval.notify_all();
            }
        }
    }
}

void parallel_for(int start, int end, const std::function<void(int)>& func,
                  ParallelSchedule schedule) {
    const int nTasks = (end - start);
    const int nThreads = numSystemThreads();
    const int chunkSize = (nTasks + nThreads - 1) / nThreads;
    WorkerTaskManager manager(func, chunkSize, nTasks);    

    std::vector<std::thread> threads;
    threadID = 0;
    for (int i = 0; i < nThreads - 1; i++) {
        threads.emplace_back(workerThreadFunc, i + 1);
    }

    workerListMutex.lock();
    threadTasks.resize(nTasks);
    for (int i = 0; i < nTasks; i++) {
        threadTasks[i] = start + i;
    }
    workerListMutex.unlock();

    {
        std::unique_lock<std::mutex> lock(workerListMutex);
        condval.notify_all();

        while (!workerTask->finished()) {
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

    for (auto& t : threads) {
        t.join();
    }
}

int numSystemThreads() {
    return std::max(1u, std::thread::hardware_concurrency());
}

int getThreadID() {
    return threadID;
}
