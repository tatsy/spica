#ifndef _SPICA_CORNELL_BOX_H_
#define _SPICA_CORNELL_BOX_H_

#include "../renderer/material.h"
#include "../renderer/scene.h"
#include "../renderer/camera.h"

namespace spica {
    inline void cornellBox(Scene& scene, Camera& camera, const int width, const int height) {
        scene.clear();    
        scene.addSphere(Sphere( 7.5,  Vector3(50.0, 72.5, 81.6),          Material(Color(16, 16, 16), Color(), REFLECTION_DIFFUSE)), true);        // Light
        scene.addSphere(Sphere(1.0e5, Vector3(1.0e5 + 1.0, 40.8, 81.6),   Material(Color(), Color(0.75, 0.25, 0.25), REFLECTION_DIFFUSE)));        // Left wall (red)
        scene.addSphere(Sphere(1.0e5, Vector3(-1.0e5+99.0, 40.8, 81.6),   Material(Color(), Color(0.25, 0.25, 0.75), REFLECTION_DIFFUSE)));        // Right wall (blue)
        scene.addSphere(Sphere(1.0e5, Vector3(50.0, 40.8, 1.0e5),         Material(Color(), Color(0.75, 0.75, 0.75), REFLECTION_DIFFUSE)));        // Back wall (white)
        scene.addSphere(Sphere(1.0e5, Vector3(50.0, 40.8, -1.0e5 + 250),  Material(Color(), Color(0.00, 0.00, 0.00), REFLECTION_DIFFUSE)));        // Front wall (black)
        scene.addSphere(Sphere(1.0e5, Vector3(50.0, 1.0e5, 81.6),         Material(Color(), Color(0.75, 0.75, 0.75), REFLECTION_DIFFUSE)));        // Floor (white)
        scene.addSphere(Sphere(1.0e5, Vector3(50.0, -1.0e5 + 81.6, 81.6), Material(Color(), Color(0.75, 0.75, 0.75), REFLECTION_DIFFUSE)));        // Ceil (white)
        scene.addSphere(Sphere(20.0,  Vector3(50.0, 20.0, 50.0),          Material(Color(), Color(0.25, 0.75, 0.25), REFLECTION_DIFFUSE)));        // Green ball
        scene.addSphere(Sphere(16.5,  Vector3(19.0, 16.5, 25.0),          Material(Color(), Color(0.99, 0.99, 0.99), REFLECTION_SPECULAR)));       // Mirror ball
        scene.addSphere(Sphere(16.5,  Vector3(77.0, 16.5, 78.0),          Material(Color(), Color(0.99, 0.99, 0.99), REFLECTION_REFRACTION)));     // Glass ball

        camera = Camera(width, height, 
                        Vector3(50.0, 40.8, 220.0),
                        Vector3(0.0, 0.0, -1.0),
                        Vector3(0.0, 1.0, 0.0),
                        30.0,
                        30.0,
                        140.0,
                        5.0,
                        28.0);
    }

    inline void cornellBoxBunny(Scene& scene, Camera& camera, const int width, const int height) {
        scene.clear();    
        scene.addSphere(Sphere( 7.5,  Vector3(50.0, 72.5, 81.6),          Material(Color(16, 16, 16), Color(), REFLECTION_DIFFUSE)), true);        // Light
        scene.addSphere(Sphere(1.0e5, Vector3(1.0e5 + 1.0, 40.8, 81.6),   Material(Color(), Color(0.75, 0.25, 0.25), REFLECTION_DIFFUSE)));        // Left wall (red)
        scene.addSphere(Sphere(1.0e5, Vector3(-1.0e5+99.0, 40.8, 81.6),   Material(Color(), Color(0.25, 0.25, 0.75), REFLECTION_DIFFUSE)));        // Right wall (blue)
        scene.addSphere(Sphere(1.0e5, Vector3(50.0, 40.8, 1.0e5),         Material(Color(), Color(0.75, 0.75, 0.75), REFLECTION_DIFFUSE)));        // Back wall (white)
        scene.addSphere(Sphere(1.0e5, Vector3(50.0, 40.8, -1.0e5 + 250),  Material(Color(), Color(0.00, 0.00, 0.00), REFLECTION_DIFFUSE)));        // Front wall (black)
        scene.addSphere(Sphere(1.0e5, Vector3(50.0, 1.0e5, 81.6),         Material(Color(), Color(0.75, 0.75, 0.75), REFLECTION_DIFFUSE)));        // Floor (white)
        scene.addSphere(Sphere(1.0e5, Vector3(50.0, -1.0e5 + 81.6, 81.6), Material(Color(), Color(0.75, 0.75, 0.75), REFLECTION_DIFFUSE)));        // Ceil (white)
        scene.addSphere(Sphere(16.5,  Vector3(77.0, 16.5, 78.0),          Material(Color(), Color(0.99, 0.99, 0.99), REFLECTION_REFRACTION)));     // Glass ball
        scene.addTrimesh(Trimesh("../../data/bunny.ply", Material(Color(), Color(0.99, 0.75, 0.75), REFLECTION_DIFFUSE)));                         // Bunny
        camera = Camera(width, height, 
                        Vector3(50.0, 40.8, 220.0),
                        Vector3(0.0, 0.0, -1.0),
                        Vector3(0.0, 1.0, 0.0),
                        30.0,
                        30.0,
                        140.0,
                        5.0,
                        28.0);
    }
}

#endif  // _SPICA_CORNELL_BOX_H_
