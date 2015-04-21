#include <iostream>

#include "../include/spica.h"
using namespace spica;


int main(int argc, char **argv) {
	std::cout << "Metropolis light transport" << std::endl;

	Scene scene;
	scene.addSphere(Sphere(5.0, Vector3(50.0, 75.0, 81.6), Color(12, 12, 12), Color(), REFLECTION_DIFFUSE), true);		 // Light
	scene.addSphere(Sphere(1e5, Vector3(1e5 + 1, 40.8, 81.6), Color(), Color(0.75, 0.25, 0.25), REFLECTION_DIFFUSE));    // Left
	scene.addSphere(Sphere(1e5, Vector3(-1e5 + 99, 40.8, 81.6), Color(), Color(0.25, 0.25, 0.75), REFLECTION_DIFFUSE));  // Right
	scene.addSphere(Sphere(1e5, Vector3(50, 40.8, 1e5), Color(), Color(0.75, 0.75, 0.75), REFLECTION_DIFFUSE));			 // Back
	scene.addSphere(Sphere(1e5, Vector3(50, 40.8, -1e5 + 170), Color(), Color(), REFLECTION_DIFFUSE));					 // Front
	scene.addSphere(Sphere(1e5, Vector3(50, 1e5, 81.6), Color(), Color(0.75, 0.75, 0.75), REFLECTION_DIFFUSE));			 // Floor
	scene.addSphere(Sphere(1e5, Vector3(50, -1e5 + 81.6, 81.6), Color(), Color(0.75, 0.75, 0.75), REFLECTION_DIFFUSE));  // Ceil
	scene.addSphere(Sphere(16.5, Vector3(27, 16.5, 47), Color(), Color(1, 1, 1)*.99, REFLECTION_SPECULAR));				 // Mirror
	scene.addSphere(Sphere(16.5, Vector3(73, 16.5, 78), Color(), Color(1, 1, 1)*.99, REFLECTION_REFRACTION));			 // Glass

	const int width = 320;
	const int height = 240;
	const int mutation = 32 * width * height;
	const int mlt_num = 32;
	const int maxDepth = 5;
	Random rng = Random::getRNG();

	Ray camera(Vector3(50.0, 52.0, 295.6), Vector3(0.0, -0.042612, -1.0).normalize());
	Vector3 cx = Vector3(width * 0.5135 / height, 0.0, 0.0);
	Vector3 cy = cx.cross(camera.direction()).normalize() * 0.5135;
	Image image(width, height);

	MLTRenderer renderer;
	renderer.render(scene, mlt_num, mutation, image, camera, cx, cy, width, height, maxDepth, rng);

	image.savePPM("simplemlt.ppm");
}