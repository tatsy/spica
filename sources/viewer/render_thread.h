#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_RENDER_THREAD_H_
#define _SPICA_RENDER_THREAD_H_

#include <functional>

#include <QtCore/qthread.h>

namespace spica {

    class RenderThread : public QThread {
        Q_OBJECT

    private:
        const std::function<void()>* _task;

    public:
        RenderThread();
        virtual ~RenderThread();    
        void doTask(const std::function<void()>& task);

    protected:
        void run() final override;
    };

}  // namespace spica

#endif  // _SPICA_RENDER_THREAD_H_

