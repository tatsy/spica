#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_PARALLEL_H_
#define _SPICA_PARALLEL_H_

#include <atomic>
#include <functional>

namespace spica {

class AtomicDouble {
public:
    explicit AtomicDouble(double v = 0.0);
    operator double() const;
    double operator=(double v);
    void add(double v);

private:
    std::atomic<uint64_t> bits;
};

}  // namespace spica

enum class ParallelSchedule {
    Static = 0x01,
    Dynamic = 0x02
};

void parallel_for(int start, int end, const std::function<void(int)>& func,
                  ParallelSchedule schedule = ParallelSchedule::Dynamic);

int numSystemThreads();
int getThreadID();

#endif  // _SPICA_PARALLEL_H_
