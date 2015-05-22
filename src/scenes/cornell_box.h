#ifndef _SPICA_CORNELL_BOX_H_
#define _SPICA_CORNELL_BOX_H_

#include "../renderer/material.h"
#include "../renderer/scene.h"
#include "../renderer/camera.h"

namespace spica {
    inline void cornellBox(Scene& scene, Camera& camera, const int width, const int height) {
        scene.clear();

        // Light
        Vector3 l00(-5.0, 9.99, -5.0);
        Vector3 l01(-5.0, 9.99,  5.0);
        Vector3 l10( 5.0, 9.99, -5.0);
        Vector3 l11( 5.0, 9.99,  5.0);
        scene.add(Quad(l00, l10, l11, l01), Material(Color(32.0, 32.0, 32.0), Color(1.0, 1.0, 1.0), REFLECTION_DIFFUSE), true);

        Vector3 v000(-10.0, -10.0, -10.0);
        Vector3 v100( 10.0, -10.0, -10.0);
        Vector3 v010(-10.0,  10.0, -10.0);
        Vector3 v001(-10.0, -10.0,  50.0);
        Vector3 v110( 10.0,  10.0, -10.0);
        Vector3 v101( 10.0, -10.0,  50.0);
        Vector3 v011(-10.0,  10.0,  50.0);
        Vector3 v111( 10.0,  10.0,  50.0);

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

        scene.add(Sphere(3.0,  Vector3( 0.0, -7.0,  0.0)), Material(Color(), Color(0.25, 0.75, 0.25), REFLECTION_DIFFUSE));
        scene.add(Sphere(3.0,  Vector3(-5.0, -7.0, -5.0)), Material(Color(), Color(0.99, 0.99, 0.99), REFLECTION_SPECULAR));       // Mirror ball
        scene.add(Sphere(3.0,  Vector3( 5.0, -7.0,  5.0)), Material(Color(), Color(0.99, 0.99, 0.99), REFLECTION_REFRACTION));     // Glass ball

        camera = Camera(width, height, 
                        Vector3(0.0, 0.0, 100.0),
                        Vector3(0.0, 0.0, -1.0),
                        Vector3(0.0, 1.0, 0.0),
                        20.0,
                        42.0,
                        58.0,
                        1.0,
                        90.0);
    }

    inline void cornellBoxBunny(Scene& scene, Camera& camera, const int width, const int height) {
        scene.clear(); 

        // Light
        Vector3 l00(-5.0, 9.99, -5.0);
        Vector3 l01(-5.0, 9.99,  5.0);
        Vector3 l10( 5.0, 9.99, -5.0);
        Vector3 l11( 5.0, 9.99,  5.0);
        scene.add(Quad(l00, l10, l11, l01), Material(Color(32.0, 32.0, 32.0), Color(1.0, 1.0, 1.0), REFLECTION_DIFFUSE), true);

        // Walls
        Vector3 v000(-10.0, -10.0, -10.0);
        Vector3 v100( 10.0, -10.0, -10.0);
        Vector3 v010(-10.0,  10.0, -10.0);
        Vector3 v001(-10.0, -10.0,  50.0);
        Vector3 v110( 10.0,  10.0, -10.0);
        Vector3 v101( 10.0, -10.0,  50.0);
        Vector3 v011(-10.0,  10.0,  50.0);
        Vector3 v111( 10.0,  10.0,  50.0);

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

        // Objects
        Trimesh bunny(DATA_DIR + "bunny.ply");
        bunny.translate(Vector3(-3.0, 0.0, 0.0));
        bunny.putOnPlane(Plane(10.0, Vector3(0.0, 1.0, 0.0)));
        scene.add(bunny, Material(Color(), Color(0.75, 0.75, 0.25), REFLECTION_DIFFUSE));                         

        scene.add(Sphere(3.0,  Vector3(5.0, -7.0, 5.0)), Material(Color(), Color(0.99, 0.99, 0.99), REFLECTION_REFRACTION));

        camera = Camera(width, height, 
                        Vector3(0.0, 0.0, 100.0),
                        Vector3(0.0, 0.0, -1.0),
                        Vector3(0.0, 1.0, 0.0),
                        20.0,
                        42.0,
                        58.0,
                        1.0,
                        90.0);
    }

    inline void cornellBoxDragon(Scene& scene, Camera& camera, const int width, const int height) {
        scene.clear(); 

        // Light
        Vector3 l00(-5.0, 9.99, -5.0);
        Vector3 l01(-5.0, 9.99,  5.0);
        Vector3 l10( 5.0, 9.99, -5.0);
        Vector3 l11( 5.0, 9.99,  5.0);
        scene.add(Quad(l00, l10, l11, l01), Material(Color(32.0, 32.0, 32.0), Color(1.0, 1.0, 1.0), REFLECTION_DIFFUSE), true);

        // Back light
        /*
        Vector3 l00(-3.0, -3.0, -9.99);
        Vector3 l01(-3.0,  3.0, -9.99);
        Vector3 l10( 3.0, -3.0, -9.99);
        Vector3 l11( 3.0,  3.0, -9.99);
        scene.add(Quad(l00, l10, l11, l01), Material(Color(128.0, 128.0, 128.0), Color(1.0, 1.0, 1.0), REFLECTION_DIFFUSE), true);
        */

        // Walls
        Vector3 v000(-10.0, -10.0, -10.0);
        Vector3 v100( 10.0, -10.0, -10.0);
        Vector3 v010(-10.0,  10.0, -10.0);
        Vector3 v001(-10.0, -10.0,  50.0);
        Vector3 v110( 10.0,  10.0, -10.0);
        Vector3 v101( 10.0, -10.0,  50.0);
        Vector3 v011(-10.0,  10.0,  50.0);
        Vector3 v111( 10.0,  10.0,  50.0);

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

        // Objects
        Trimesh dragon(DATA_DIR + "dragon.ply");
        dragon.scale(70.0, 70.0, 70.0);
        dragon.translate(Vector3(0.0, 0.0, 0.0));
        dragon.putOnPlane(Plane(10.0, Vector3(0.0, 1.0, 0.0)));
        scene.add(dragon, Material(Color(), Color(0.99, 0.99, 0.99), REFLECTION_SUBSURFACE));                         

        camera = Camera(width, height, 
                        Vector3(0.0, 0.0, 100.0),
                        Vector3(0.0, 0.0, -1.0),
                        Vector3(0.0, 1.0, 0.0),
                        20.0,
                        42.0,
                        58.0,
                        1.0,
                        90.0);
    }
}

#endif  // _SPICA_CORNELL_BOX_H_
