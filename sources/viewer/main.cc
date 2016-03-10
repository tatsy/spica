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
                                       std::move(filter),
                                       kOutputDirectory + "pathtrace_%03d.png");

    Bounds2d screen(-1.0, -1.0, 1.0, 1.0);
    double fov = PI / 3.0;

    Point3d  eye(0.0, 0.0, 5.0 / tan(fov / 2.0));
    Point3d  look(0.0, 0.0, 0.0);
    Vector3d up(0.0, 1.0, 0.0);

    double focal = std::abs((look - eye).z());
    double lensR = 0.5;

    Scene scene;
    std::shared_ptr<Camera> camera = std::make_shared<PerspectiveCamera>(
        Transform::lookAt(eye, look, up),
        screen, lensR, focal, fov, film.get());

    std::vector<Triangle> tris;
    std::vector<Spectrum> Kd;
    cornellBox(&scene, &tris, &Kd);

    SceneViewer viewer;
    viewer.showMaximized();

    viewer.setScene(tris, Kd, camera);

    return app.exec();
}
