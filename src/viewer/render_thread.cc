#include "render_thread.h"

#include <QtCore/qmutex.h>

namespace spica {

    namespace {

        static QMutex mutex;

    }

    RenderThread::RenderThread()
        : QThread()
        , _task(NULL)
    {
    }

    RenderThread::~RenderThread()
    {
    }

    void RenderThread::doTask(const std::function<void()>& task) {
        _task = &task;    
        run();
        _task = NULL;
    }

    void RenderThread::run() {
        if (_task != NULL) {
            mutex.lock();
            (*_task)();
            mutex.unlock();
        }
    }

}  // namespace spica
