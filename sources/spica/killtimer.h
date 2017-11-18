#ifdef _MSC_VER
#pragma once
#endif

#ifndef KILL_TIMER_H
#define KILL_TIMER_H

#include <memory>

#include <QtCore/qtimer.h>
#include <QtCore/qthread.h>
#include <QtCore/qelapsedtimer.h>

class KillTimer : public QThread {
    Q_OBJECT

public:
    KillTimer(QObject *parent = nullptr);
    KillTimer(int hour, int minute, int second, QObject *parent = nullptr);

    void run() override;
    void setExpire(int hour, int minute, int second);

private:
    QElapsedTimer timer;
    uint64_t expire;
};

#endif  // KILL_TIMER_H
