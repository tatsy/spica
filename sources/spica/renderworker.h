#ifdef _MSC_VER
#pragma once
#endif

#ifndef _RENDER_WORKER_H_
#define _RENDER_WORKER_H_

#include <string>
#include <thread>

#include "sceneparser.h"

namespace spica {

//! Render workder
class RenderWorker {
public:
    RenderWorker(const std::string &sceneFile);

    virtual ~RenderWorker();

    void start();

private:
    SceneParser parser_;
    std::thread thread_;
};

}  // namespace spica

#endif  // _RENDER_WORKER_H_
