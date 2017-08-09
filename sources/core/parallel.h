#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_PARALLEL_H_
#define _SPICA_PARALLEL_H_

#include <atomic>
#include <functional>

#include "common.h"

namespace spica {

class SPICA_EXPORTS AtomicDouble {
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

SPICA_EXPORTS void parallel_for(int start, int end, const std::function<void(int)>& func,
                                ParallelSchedule schedule = ParallelSchedule::Dynamic);

SPICA_EXPORTS int numSystemThreads();
SPICA_EXPORTS int getThreadID();
SPICA_EXPORTS void setNumThreads(uint32_t n);

#endif  // _SPICA_PARALLEL_H_
