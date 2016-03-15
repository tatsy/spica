#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_PARALLEL_H_
#define _SPICA_PARALLEL_H_

#include <functional>

enum class ParallelSchedule {
    Static = 0x01,
    Dynamic = 0x02
};

void parallel_for(int start, int end, const std::function<void(int)>& func,
                  ParallelSchedule schedule = ParallelSchedule::Dynamic);

int numAvailableThreads();

#endif  // _SPICA_PARALLEL_H_
