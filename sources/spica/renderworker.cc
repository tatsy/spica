#include "renderworker.h"

#include "sceneparser.h"
using namespace spica;

RenderWorker::RenderWorker(const std::string &sceneFile)
    : QThread()
    , sceneFile_(sceneFile) {
    connect(this, SIGNAL(started()), this, SLOT(process()));
}

RenderWorker::~RenderWorker() {
}

void RenderWorker::process() {
    printf("Process started!\n");
    SceneParser parser(sceneFile_);
    parser.parse();
}
