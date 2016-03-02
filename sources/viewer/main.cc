#include <QtWidgets/qapplication.h>

#include "../../include/spica.h"
using namespace spica;

#include "scene_viewer.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    const int width  = 600;
    const int height = 400;

    // Define camera
    std::unique_ptr<Filter> filter =
        std::make_unique<BoxFilter>(Vector2d(0.5, 0.5));
    auto film = std::make_unique<Film>(Point2i(width, height),
                                       filter,
                                       kOutputDirectory + "pathtrace_%03d.png");

    RectF screen(-2.5, -2.5, 5.0, 5.0);
    double fov = PI / 24.0;

    Point3d  eye(0.0, 0.0, 5.0 / tan(fov / 2.0));
    Point3d  look(0.0, 0.0, 0.0);
    Vector3d up(0.0, 1.0, 0.0);

    double focal = std::abs((look - eye).z());
    double lensR = 0.5;

    Scene scene;
    std::shared_ptr<Camera> camera = std::make_shared<PerspectiveCamera>(
        Transform::lookAt(eye, look, up),
        screen, lensR, focal, fov, film.get());

    cornellBox(&scene, nullptr, 640, 480);

    SceneViewer viewer;
    viewer.showMaximized();

    viewer.setScene(scene, camera);

    return app.exec();
}
