#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_PARALLEL_H_
#define _SPICA_PARALLEL_H_

#include <iostream>
#include <atomic>
#include <mutex>
#include <functional>
#include <condition_variable>

#include "core/common.h"

namespace spica {

class SPICA_EXPORTS AtomicDouble {
public:
    explicit AtomicDouble(double v = 0.0);
    explicit operator double() const;
    double operator=(double v);
    void add(double v);

private:
    std::atomic<uint64_t> bits;
};

class SPICA_EXPORTS Barrier {
public:
    Barrier(int count)
        : count{ count } {
        Assertion(count >= 0, "# of threads you use should be positive!");
    }
    virtual ~Barrier() {
        Assertion(count == 0, "Not all the treads finish their task properly!");
    }

    void wait();

private:
    std::mutex mutex;
    std::condition_variable cv;
    int count;
};

}  // namespace spica

enum class ParallelSchedule {
    Static = 0x01,
    Dynamic = 0x02
};

SPICA_EXPORTS void parallel_for(int64_t start, int64_t end, const std::function<void(int)> &func,
                                ParallelSchedule schedule = ParallelSchedule::Dynamic);

SPICA_EXPORTS int numSystemThreads();
SPICA_EXPORTS int getThreadID();
SPICA_EXPORTS void setNumThreads(uint32_t n);

#endif  // _SPICA_PARALLEL_H_
