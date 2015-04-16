#include <iostream>

#include "../include/spica.h"
using namespace spica;

int main(int argc, char **argv) {
    std::cout << "Path tracing" << std::endl << std::endl;

    Scene scene;
    scene.addSphere(Sphere(1.0e5, Vector3(1.0e5 + 1.0, 40.8, 81.6),   Color(),           Color(0.75, 0.25, 0.25), REFLECTION_DIFFUSE));        // Left wall (red)
    scene.addSphere(Sphere(1.0e5, Vector3(-1.0e5+99.0, 40.8, 81.6),   Color(),           Color(0.25, 0.25, 0.75), REFLECTION_DIFFUSE));        // Right wall (blue)
    scene.addSphere(Sphere(1.0e5, Vector3(50.0, 40.8, 1.0e5),         Color(),           Color(0.75, 0.75, 0.75), REFLECTION_DIFFUSE));        // Back wall (white)
    scene.addSphere(Sphere(1.0e5, Vector3(50.0, 40.8, -1.0e5 + 250),  Color(),           Color(),                 REFLECTION_DIFFUSE));        // Front wall (black)
    scene.addSphere(Sphere(1.0e5, Vector3(50.0, 1.0e5, 81.6),         Color(),           Color(0.75, 0.75, 0.75), REFLECTION_DIFFUSE));        // Floor (white)
    scene.addSphere(Sphere(1.0e5, Vector3(50.0, -1.0e5 + 81.6, 81.6), Color(),           Color(0.75, 0.75, 0.75), REFLECTION_DIFFUSE));        // Ceil (white)
    scene.addSphere(Sphere(20.0,  Vector3(65.0, 20.0, 20.0),          Color(),           Color(0.25, 0.75, 0.25), REFLECTION_DIFFUSE));        // Green ball
    scene.addSphere(Sphere(16.5,  Vector3(27.0, 16.5, 47.0),          Color(),           Color(0.99, 0.99, 0.99), REFLECTION_SPECULAR));       // Mirror
    scene.addSphere(Sphere(16.5,  Vector3(77.0, 16.5, 78),            Color(),           Color(0.99, 0.99, 0.99), REFLECTION_REFRACTION));     // Glass ball
    scene.addSphere(Sphere(15.0,  Vector3(50.0, 90.0, 81.6),          Color(36, 36, 36), Color(),                 REFLECTION_DIFFUSE), true);  // Light

    Renderer renderer(128, 128, 8, 4);
    renderer.render(scene);

    return 0;
}
