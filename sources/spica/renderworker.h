#ifdef _MSC_VER
#pragma once
#endif

#ifndef _RENDER_WORKER_H_
#define _RENDER_WORKER_H_

#include <string>

class RenderWorker {
public:
    RenderWorker(const std::string &sceneFile);
    virtual ~RenderWorker();

    void start();

private:
    std::string sceneFile_;
};

#endif  // _RENDER_WORKER_H_
