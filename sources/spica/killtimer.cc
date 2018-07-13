#include "killtimer.h"

#include <cstdio>
#include <chrono>
#include <thread>

#include "core/timer.h"

namespace {

void killer(uint64_t expire) {
    using spica::Timer;
    Timer timer;
    timer.start();
    while (true) {
        if (timer.stop() > expire) {
            std::exit(1);
        }
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
}

}  // anonymous namespace

KillTimer::KillTimer() {
    thread_.join();
}

KillTimer::KillTimer(int hour, int minute, int second) {
    setExpire(hour, minute, second);
}

void KillTimer::setExpire(int hour, int minute, int second) {
    expire_ = (uint64_t)((hour * 60 + minute) * 60 + second) * 1000;
}

void KillTimer::start() {
    thread_ = std::thread(killer, expire_);
}
