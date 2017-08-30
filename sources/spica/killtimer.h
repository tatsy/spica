#ifdef _MSC_VER
#pragma once
#endif

#ifndef _KILL_TIMER_H_
#define _KILL_TIMER_H_

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

#endif  _KILL_TIMER_H_
