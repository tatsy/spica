#ifndef _SPICA_CORNELL_BOX_H_
#define _SPICA_CORNELL_BOX_H_

#include "../renderer/material.h"
#include "../renderer/scene.h"
#include "../renderer/camera.h"

namespace spica {
    inline void cornellBox(Scene& scene, Camera& camera, const int width, const int height) {
        scene.clear();

        Vector3 v000(-10.0, -10.0, -10.0);
        Vector3 v100( 10.0, -10.0, -10.0);
        Vector3 v010(-10.0,  10.0, -10.0);
        Vector3 v001(-10.0, -10.0,  10.0);
        Vector3 v110( 10.0,  10.0, -10.0);
        Vector3 v101( 10.0, -10.0,  10.0);
        Vector3 v011(-10.0,  10.0,  10.0);
        Vector3 v111( 10.0,  10.0,  10.0);

        Quad ceilWall(v010, v110, v111, v011);
        Quad floorWall(v000, v001, v101, v100);
        Quad backWall(v000, v100, v110, v010);
        Quad leftWall(v000, v010, v011, v001);
        Quad rightWall(v100, v101, v111, v110);

        scene.add(ceilWall, Material(Color(), Color(0.75, 0.75, 0.75), REFLECTION_DIFFUSE));
        scene.add(floorWall, Material(Color(), Color(0.75, 0.75, 0.75), REFLECTION_DIFFUSE));
        scene.add(backWall, Material(Color(), Color(0.75, 0.75, 0.75), REFLECTION_DIFFUSE));
        scene.add(leftWall, Material(Color(), Color(0.75, 0.25, 0.25), REFLECTION_DIFFUSE));
        scene.add(rightWall, Material(Color(), Color(0.25, 0.25, 0.75), REFLECTION_DIFFUSE));

        /*
        scene.add(Sphere(20.0,  Vector3(50.0, 20.0, 50.0),          Material(Color(), Color(0.25, 0.75, 0.25), REFLECTION_DIFFUSE)));        // Green ball
        scene.add(Sphere(16.5,  Vector3(19.0, 16.5, 25.0),          Material(Color(), Color(0.99, 0.99, 0.99), REFLECTION_SPECULAR)));       // Mirror ball
        scene.add(Sphere(16.5,  Vector3(77.0, 16.5, 78.0),          Material(Color(), Color(0.99, 0.99, 0.99), REFLECTION_REFRACTION)));     // Glass ball
        */

        camera = Camera(width, height,
                        Vector3(0.0, 0.0, 20.0),
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

        Vector3 v000(-10.0, -10.0, -10.0);
        Vector3 v100( 10.0, -10.0, -10.0);
        Vector3 v010(-10.0,  10.0, -10.0);
        Vector3 v001(-10.0, -10.0,  10.0);
        Vector3 v110( 10.0,  10.0, -10.0);
        Vector3 v101( 10.0, -10.0,  10.0);
        Vector3 v011(-10.0,  10.0,  10.0);
        Vector3 v111( 10.0,  10.0,  10.0);

        Quad ceilWall(v010, v110, v111, v011);
        Quad floorWall(v000, v001, v101, v100);
        Quad backWall(v000, v100, v110, v010);
        Quad leftWall(v000, v010, v011, v001);
        Quad rightWall(v100, v101, v111, v110);

        scene.add(ceilWall, Material(Color(), Color(0.75, 0.75, 0.75), REFLECTION_DIFFUSE));
        scene.add(floorWall, Material(Color(), Color(0.75, 0.75, 0.75), REFLECTION_DIFFUSE));
        scene.add(backWall, Material(Color(), Color(0.75, 0.75, 0.75), REFLECTION_DIFFUSE));
        scene.add(leftWall, Material(Color(), Color(0.75, 0.25, 0.25), REFLECTION_DIFFUSE));
        scene.add(rightWall, Material(Color(), Color(0.25, 0.25, 0.75), REFLECTION_DIFFUSE));

        Trimesh bunny(DATA_DIR + "bunny.ply");
        bunny.buildKdTreeAccel();
        scene.add(bunny, Material(Color(), Color(0.99, 0.75, 0.75), REFLECTION_DIFFUSE));                         

        scene.add(Sphere(3.0,  Vector3(3.0, -7.0, 3.0)), Material(Color(), Color(0.25, 0.75, 0.25), REFLECTION_DIFFUSE));

        camera = Camera(width, height, 
                        Vector3(0.0, 0.0, 20.0),
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
