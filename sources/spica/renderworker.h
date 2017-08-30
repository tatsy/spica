#ifdef _MSC_VER
#pragma once
#endif

#ifndef _RENDER_WORKER_H_
#define _RENDER_WORKER_H_

#include <QtCore/qthread.h>

class RenderWorker : public QThread {
    Q_OBJECT

public:
    RenderWorker(const std::string &sceneFile);
    virtual ~RenderWorker();

public slots:
    void process();

private:
    std::string sceneFile_;
};

#endif  // _RENDER_WORKER_H_
