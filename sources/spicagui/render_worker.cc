#include "render_worker.h"

namespace spica {

RenderWorker::RenderWorker(QObject* parent)
    : QObject{ parent } {
}

RenderWorker::~RenderWorker() {
}

void RenderWorker::setTask(const std::shared_ptr<std::function<void()>>& task) {
    task_ = task;
}

void RenderWorker::startRender() {
    printf("Hogehoge\n");
    (*task_)();
    emit renderFinished();
}

}  // namespace spica
