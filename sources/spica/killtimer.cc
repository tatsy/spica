#include "killtimer.h"

#include <cstdio>

KillTimer::KillTimer(QObject *parent)
    : QThread(parent) {
}

KillTimer::KillTimer(int hour, int minute, int second, QObject *parent)
    : KillTimer(parent) {
    setExpire(hour, minute, second);
}

void KillTimer::setExpire(int hour, int minute, int second) {
    expire = (quint64)((hour * 60 + minute) * 60 + second) * 1000;    
}

void KillTimer::run() {
    timer.start();
    while(true) {
        if (timer.elapsed() >= expire) {
            std::exit(1);
        }        
        this->msleep(10);
    }
}
