#include "scene_viewer.h"

namespace spica {

    SceneViewer::SceneViewer(QWidget *parent)
        : QMainWindow(parent)
        , qglWidget(0)
    {
        qglWidget = new QGLRenderWidget(this);
        qglWidget->show();

        this->setCentralWidget(qglWidget);
    }

    SceneViewer::~SceneViewer()
    {
        delete qglWidget;
    }

    void SceneViewer::setScene(const Scene& scene, const Camera& camera) {
        this->resize(camera.imageW(), camera.imageH());
        qglWidget->setScene(scene, camera);
    }

}  // namespace spica
