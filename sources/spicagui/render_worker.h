#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_RENDER_WORKER_H_
#define _SPICA_RENDER_WORKER_H_

#include <memory>
#include <functional>

#include <QtCore/qobject.h>
#include <QtCore/qthread.h>

namespace spica {

class RenderWorker : public QObject {
    Q_OBJECT

public:
    explicit RenderWorker(QObject* parent = nullptr);
    virtual ~RenderWorker();

    void setTask(const std::shared_ptr<std::function<void()>>& task);

public slots:
    void startRender();

signals:
    void renderFinished();    

private:
    std::shared_ptr<std::function<void()>> task_ = nullptr;
};

class RenderController : public QObject {
    Q_OBJECT

public:
    RenderController(const std::shared_ptr<std::function<void()>>& task) {
        RenderWorker* worker = new RenderWorker();
        worker->setTask(task);

        printf("Hoge\n");

        worker->moveToThread(&workerThread);
        connect(&workerThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
        connect(this, SIGNAL(operate()), worker, SLOT(startRender()));
        connect(worker, SIGNAL(renderFinished()), this, SLOT(OnRenderFinished()));

        workerThread.start();
    }

    ~RenderController() {
        workerThread.quit();
        workerThread.wait();
    }

public slots:
    void OnRenderFinished() {
        printf("Finished thread!!");
    }

signals:
    void operate();

private:
    QThread workerThread;
};

}  // namespace spica

#endif  // _SPICA_RENDER_WORKER_H_
