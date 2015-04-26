#include <QtWidgets/qapplication.h>

#include "../scenes/cornell_box.h"
#include "scene_viewer.h"
using namespace spica;

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    Scene scene;
    Camera camera;
    cornellBox(scene, camera, 640, 480);

    SceneViewer viewer;
    viewer.show();
    viewer.setScene(scene, camera);

    return app.exec();
}
