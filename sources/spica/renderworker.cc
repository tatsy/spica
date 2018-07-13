#include "renderworker.h"

#include <thread>

#include "sceneparser.h"

namespace spica {

RenderWorker::RenderWorker(const std::string &sceneFile)
        : parser_(sceneFile) {
}

RenderWorker::~RenderWorker() {
    //thread_.join();
}

void RenderWorker::start() {
    printf("Process started!\n");
    parser_.parse();
//    thread_ = std::thread([&]() {
//        parser_.parse();
//    });
}

}  // namespace spica
