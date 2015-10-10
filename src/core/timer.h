#ifndef _SPICA_TIMER_H_
#define _SPICA_TIMER_H_

#if __cplusplus > 199711L
#include <chrono>
typedef std::chrono::time_point<std::chrono::system_clock> time_type;
inline time_type tick() {
    return std::chrono::system_clock::now();
}
inline double to_duration(time_type start, time_type end) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0;
}
#else
#include <ctime>
typedef clock_t time_type;
inline time_type tick() {
    return clock();
}
inline double to_duration(time_type start, time_type end) {
    return (end - start) / 1000.0;
}
#endif

namespace spica {

    class Timer {
    private:
        time_type _start;
        time_type _end;

    public:
        Timer()
            : _start()
            , _end()
        {
        }

        void start() {
            _start = tick();
        }

        double stop() {
            _end = tick();
            return to_duration(_start, _end);
        }
    };
}

#endif  // _SPICA_TIMER_H_
