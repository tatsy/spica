#ifdef _MSC_VER
#pragma once
#endif

#ifndef KILL_TIMER_H
#define KILL_TIMER_H

#include <cstdint>
#include <thread>

class KillTimer {
public:
    KillTimer();
    KillTimer(int hour, int minute, int second);

    void start();
    void setExpire(int hour, int minute, int second);

private:
    uint64_t expire_;
    std::thread thread_;
};

#endif  // KILL_TIMER_H
