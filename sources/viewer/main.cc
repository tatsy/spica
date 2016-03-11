#include <QtWidgets/qapplication.h>

#include "../../include/spica.h"
using namespace spica;

#include "scene_viewer.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    const int width  = 600;
    const int height = 400;

    // Define camera
    Scene scene;
    std::shared_ptr<Camera> camera;

    std::vector<Triangle> tris;
    std::vector<Spectrum> Kd;
    cornellBox(&scene, &camera, Point2i(width, height), &tris, &Kd);

    SceneViewer viewer;
    viewer.showMaximized();

    viewer.setScene(tris, Kd, camera);

    return app.exec();
}
