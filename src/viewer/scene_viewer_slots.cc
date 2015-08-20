#include "scene_viewer.h"

#include <functional>

namespace spica {

    void SceneViewer::onRenderButtonClicked() {
        if (!renderThread->isRunning()) {
            // TODO: implement scene graph class and load the graph from XML
            renderer.reset(new PathTracingRenderer());
        }
    }

}  // namespace spica
