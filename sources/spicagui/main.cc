#include <QtWidgets/qapplication.h>

#include "spica.h"
using namespace spica;

#include "scene_viewer.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    SceneViewer viewer;
    viewer.showMaximized();

    return app.exec();
}
