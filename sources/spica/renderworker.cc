#include "renderworker.h"

#include <thread>

#include "sceneparser.h"
using namespace spica;

RenderWorker::RenderWorker(const std::string &sceneFile)
    : sceneFile_(sceneFile) {
}

RenderWorker::~RenderWorker() {
}

void RenderWorker::start() {
    printf("Process started!\n");
    SceneParser parser(sceneFile_);

    std::thread([&]() {
        parser.parse();
    });
}
