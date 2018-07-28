#define SPICA_API_EXPORT
#include "parallel.h"

#include <cstring>
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

AtomicDouble::AtomicDouble(double v)
    : bits(doubleToBits(v)) {
}

AtomicDouble::operator double() const {
    return bitsToDouble(bits.load());
}

double AtomicDouble::operator=(double v) {
    bits.store(doubleToBits(v));
    return v;
}

void AtomicDouble::add(double v) {
    uint64_t oldBits = bits.load();
    uint64_t newBits;
    do {
        newBits = doubleToBits(bitsToDouble(oldBits) + v);
    } while (!bits.compare_exchange_weak(oldBits, newBits));    
}

void Barrier::wait() {
    std::unique_lock<std::mutex> lock(mutex);
    if (--count == 0) {
        cv.notify_all();
    } else {
        cv.wait(lock, [this] { return count == 0; });
    }
}

}  // namespace spica

// ---------------------------------------------------------------------------------------------------------------------
// parallel_for definition
// ---------------------------------------------------------------------------------------------------------------------

static std::vector<std::thread> threads;
static bool shutdownThreads = false;
class ParallelForLoop;
static ParallelForLoop *workList = nullptr;
static std::mutex workListMutex;

static int threadCount = std::thread::hardware_concurrency();

static thread_local int threadID;
static std::condition_variable condval;

class ParallelForLoop {
public:
    ParallelForLoop(std::function<void(int64_t)> func, int64_t maxIndex, int chunkSize)
        : func{ std::move(func) }
        , maxIndex{ maxIndex }
        , chunkSize{ chunkSize } {
    }

    bool finished() const {
        return nextIndex >= maxIndex && activeWorkers == 0;
    }

public:
    std::function<void(int64_t)> func;
    const int64_t maxIndex;
    const int chunkSize;

    int64_t nextIndex = 0;
    int activeWorkers = 0;
    ParallelForLoop *next = nullptr;
};


static std::condition_variable workListCondition;

static void workerThreadFunc(int threadIndex, std::shared_ptr<spica::Barrier> barrier) {
    // Initialize worker
    threadID = threadIndex;
    barrier->wait();
    barrier.reset();

    std::unique_lock<std::mutex> lock(workListMutex);
    while (!shutdownThreads) {
        if (!workList) {
            // Sleep thread while no task found.
            workListCondition.wait(lock);
        } else {
            // Get task and evaluate a function
            ParallelForLoop &loop = *workList;

            int64_t indexStart = loop.nextIndex;
            int64_t indexEnd = std::min(indexStart + loop.chunkSize, loop.maxIndex);

            // Update loop state
            loop.nextIndex = indexEnd;
            if (loop.nextIndex == loop.maxIndex) {
                workList = loop.next;
            }
            loop.activeWorkers++;

            // Run loop
            lock.unlock();
            for (int64_t index = indexStart; index < indexEnd; index++) {
                if (loop.func) {
                    loop.func(index);
                }
            }
            lock.lock();

            // Finish working for a function
            loop.activeWorkers--;
            if (loop.finished()) {
                workListCondition.notify_all();
            }
        }
    }
}

class ParallelManager {
public:
    ParallelManager() {
        // Initialize threads
        threadID = 0;
        const int nThreads = numSystemThreads();
        auto barrier = std::make_shared<spica::Barrier>(nThreads);
        for (int i = 0; i < nThreads - 1; i++) {
            threads.emplace_back(workerThreadFunc, i + 1, barrier);
        }
        barrier->wait();
    }

    ~ParallelManager() {
        // Clean up threads
        if (threads.empty()) return;

        {
            std::lock_guard<std::mutex> lock(workListMutex);
            shutdownThreads = true;
            workListCondition.notify_all();
        }

        for (auto &thread: threads) {
            thread.join();
        }
        threads.erase(threads.begin(), threads.end());
        shutdownThreads = false;
    }
};

static std::unique_ptr<ParallelManager> manager = nullptr;

void parallel_for(int64_t start, int64_t end, const std::function<void(int)>& func, ParallelSchedule schedule) {
    if (!manager) {
        manager = std::make_unique<ParallelManager>();
    }

    // Size parameters
    const int64_t count = end - start;
    const int nThreads = numSystemThreads();
    const int chunkSize = schedule == ParallelSchedule::Dynamic ? 1 : (count + nThreads - 1) / nThreads;

    // Create and enqueue "ParallelForLoop" object.
    ParallelForLoop loop(std::move(func), count, chunkSize);
    workListMutex.lock();
    loop.next = workList;
    workList = &loop;
    workListMutex.unlock();

    // Notify worker threads of work to be done.
    std::unique_lock<std::mutex> lock(workListMutex);
    workListCondition.notify_all();

    // Make each worker thread active from a main thread
    while (!loop.finished()) {
        int64_t indexStart = loop.nextIndex;
        int64_t indexEnd   = std::min(indexStart + loop.chunkSize, loop.maxIndex);

        loop.nextIndex = indexEnd;
        if (loop.nextIndex == loop.maxIndex) {
            workList = loop.next;
        }
        loop.activeWorkers++;

        lock.unlock();
        for (int64_t index = indexStart; index < indexEnd; index++) {
            if (loop.func) {
                loop.func(index);
            }
        }
        lock.lock();

        loop.activeWorkers--;
    }
}

int numSystemThreads() {
    return std::max(1, threadCount);
}

int getThreadID() {
    return threadID;
}

void setNumThreads(uint32_t n) {
    if (n == 0) {
        threadCount = std::thread::hardware_concurrency();
    } else {
        threadCount = std::max(1u, std::min(n, std::thread::hardware_concurrency()));
    }
}