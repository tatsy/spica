#include "scene_viewer.h"

#include <functional>

#include <QtWidgets/qfiledialog.h>

namespace spica {

    void SceneViewer::onRenderButtonClicked() {
        if (!renderThread->isRunning()) {
            // TODO: implement scene graph class and load the graph from XML
            // renderer.reset(new PathIntegrator(camera));
        }
    }

    void SceneViewer::onLoadButtonClicked() {
        const QString filename = QFileDialog::getOpenFileName(
            this,
            tr("Open scene file"),
            "",
            tr("Scene files (*.yaml *.yml)")
        );

        if (filename == "") return;

        /*
        Scene scene;
        Camera camera;
        SceneLoader loader;
        loader.load(filename.toStdString(), &scene, &camera);
        setScene(scene, camera);
         */
    }

}  // namespace spica
