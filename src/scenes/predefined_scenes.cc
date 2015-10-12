#define SPICA_PREDEFINED_SCENES_EXPORT
#include "predefined_scenes.h"

#include "../core/common.h"
#include "../core/image.h"
#include "../scenes/scene.h"
#include "../camera/camera.h"
#include "../bsdf/bsdf.h"
#include "../bsdf/brdf.h"
#include "../bsdf/bssrdf.h"
#include "../shape/shape.h"

namespace spica {
    void cornellBox(Scene* scene, Camera* camera, const int width, const int height) {
        scene->clear();

        // Light
        Vector3D l00(-5.0, 9.99, -5.0);
        Vector3D l01(-5.0, 9.99,  5.0);
        Vector3D l10( 5.0, 9.99, -5.0);
        Vector3D l11( 5.0, 9.99,  5.0);
        scene->setAreaLight(Quad(l00, l10, l11, l01), 
                            Color(32.0, 32.0, 32.0));

        Vector3D v000(-10.0, -10.0, -10.0);
        Vector3D v100( 10.0, -10.0, -10.0);
        Vector3D v010(-10.0,  10.0, -10.0);
        Vector3D v001(-10.0, -10.0,  50.0);
        Vector3D v110( 10.0,  10.0, -10.0);
        Vector3D v101( 10.0, -10.0,  50.0);
        Vector3D v011(-10.0,  10.0,  50.0);
        Vector3D v111( 10.0,  10.0,  50.0);

        Quad ceilWall(v010, v110, v111, v011);
        Quad floorWall(v000, v001, v101, v100);
        Quad backWall(v000, v100, v110, v010);
        Quad leftWall(v000, v010, v011, v001);
        Quad rightWall(v100, v101, v111, v110);

        scene->addShape(floorWall, LambertianBRDF::factory(Color(0.75, 0.75, 0.75)));
        scene->addShape(ceilWall,  LambertianBRDF::factory(Color(0.75, 0.75, 0.75)));
        scene->addShape(backWall,  LambertianBRDF::factory(Color(0.75, 0.75, 0.75)));
        scene->addShape(leftWall,  LambertianBRDF::factory(Color(0.75, 0.25, 0.25)));
        scene->addShape(rightWall, LambertianBRDF::factory(Color(0.25, 0.25, 0.75)));

        scene->addShape(Sphere(Vector3D( 0.0, -7.0,  0.0), 3.0),
                        LambertianBRDF::factory(Color(0.25, 0.75, 0.25)));
        scene->addShape(Sphere(Vector3D(-5.0, -7.0, -5.0), 3.0),
                        SpecularBRDF::factory(Color(0.99, 0.99, 0.99)));
        scene->addShape(Sphere(Vector3D( 5.0, -7.0,  5.0), 3.0),
                        RefractiveBSDF::factory(Color(0.99, 0.99, 0.99)));

        Trimesh texcube(kDataDirectory + "tex_cube.ply");
        texcube.fitToBBox(BBox(-2.0, -2.0, -2.0, 2.0, 2.0, 2.0));
        texcube.putOnPlane(Plane(10.0, Vector3D(0.0, 1.0, 0.0)));
        texcube.translate(Vector3D(-5.0, 0.0, 5.0));
        scene->addShape(texcube, LambertianBRDF::factory(Color(0.75, 0.75, 0.75)));

        scene->setAccelType(AccelType::QBVH);
        scene->finalize();

        (*camera) = Camera::asDoF(width, height, 
                                  Vector3D(0.0, 0.0, 100.0),
                                  Vector3D(0.0, 0.0, -1.0),
                                  Vector3D(0.0, 1.0, 0.0),
                                  20.0,
                                  42.0,
                                  58.0,
                                  1.0,
                                  90.0);
    }

    /*
    void cornellBoxBunny(Scene* scene, Camera* camera, const int width, const int height) {
        scene->clear(); 

        // Light
        Vector3D l00(-5.0, 9.99, -5.0);
        Vector3D l01(-5.0, 9.99,  5.0);
        Vector3D l10( 5.0, 9.99, -5.0);
        Vector3D l11( 5.0, 9.99,  5.0);
        scene->add(Quad(l00, l10, l11, l01), Material(Color(32.0, 32.0, 32.0), Color(1.0, 1.0, 1.0), REFLECTION_DIFFUSE), true);

        // Walls
        Vector3D v000(-10.0, -10.0, -10.0);
        Vector3D v100( 10.0, -10.0, -10.0);
        Vector3D v010(-10.0,  10.0, -10.0);
        Vector3D v001(-10.0, -10.0,  50.0);
        Vector3D v110( 10.0,  10.0, -10.0);
        Vector3D v101( 10.0, -10.0,  50.0);
        Vector3D v011(-10.0,  10.0,  50.0);
        Vector3D v111( 10.0,  10.0,  50.0);

        Quad ceilWall(v010, v110, v111, v011);
        Quad floorWall(v000, v001, v101, v100);
        Quad backWall(v000, v100, v110, v010);
        Quad leftWall(v000, v010, v011, v001);
        Quad rightWall(v100, v101, v111, v110);

        const BRDF brdf = PhongBRDF::factory(Color(0.75, 0.75, 0.75), 32.0);
        scene->add(floorWall, Material(Color(), Color(0.75, 0.75, 0.75), REFLECTION_BRDF, brdf));
        scene->add(ceilWall, Material(Color(), Color(0.75, 0.75, 0.75), REFLECTION_DIFFUSE));
        scene->add(backWall, Material(Color(), Color(0.75, 0.75, 0.75), REFLECTION_DIFFUSE));
        scene->add(leftWall, Material(Color(), Color(0.75, 0.25, 0.25), REFLECTION_DIFFUSE));
        scene->add(rightWall, Material(Color(), Color(0.25, 0.25, 0.75), REFLECTION_DIFFUSE));

        // Objects
        Trimesh bunny(kDataDirectory + "bunny.ply");
        bunny.translate(Vector3D(-3.0, 0.0, 0.0));
        bunny.putOnPlane(Plane(10.0, Vector3D(0.0, 1.0, 0.0)));
        scene->add(bunny, Material(Color(), Color(0.75, 0.75, 0.25), REFLECTION_DIFFUSE));                         

        scene->add(Sphere(3.0,  Vector3D(5.0, -7.0, 5.0)), Material(Color(), Color(0.99, 0.99, 0.99), REFLECTION_REFRACTION));

        (*camera) = Camera(width, height, 
                           Vector3D(0.0, 0.0, 100.0),
                           Vector3D(0.0, 0.0, -1.0),
                           Vector3D(0.0, 1.0, 0.0),
                           20.0,
                           42.0,
                           58.0,
                           1.0,
                           90.0);
    }

    void cornellBoxOcclusion(Scene* scene, Camera* camera, const int width, const int height) {
        scene->clear(); 

        // Light
        //Vector3D l00(-5.0, 9.99, -5.0);
        //Vector3D l01(-5.0, 9.99,  5.0);
        //Vector3D l10( 5.0, 9.99, -5.0);
        //Vector3D l11( 5.0, 9.99,  5.0);
        //scene->add(Quad(l00, l10, l11, l01), Material(Color(32.0, 32.0, 32.0), Color(1.0, 1.0, 1.0), REFLECTION_DIFFUSE), true);

        // Back light
        Vector3D l00(3.0, -3.0, -9.99);
        Vector3D l01(8.0, -3.0, -9.99);
        Vector3D l10(3.0, -8.0, -9.99);
        Vector3D l11(8.0, -8.0, -9.99);
        scene->add(Quad(l00, l10, l11, l01), Material(Color(128.0, 128.0, 128.0), Color(1.0, 1.0, 1.0), REFLECTION_DIFFUSE), true);

        // Walls
        Vector3D v000(-10.0, -10.0, -10.0);
        Vector3D v100( 10.0, -10.0, -10.0);
        Vector3D v010(-10.0,  10.0, -10.0);
        Vector3D v001(-10.0, -10.0,  50.0);
        Vector3D v110( 10.0,  10.0, -10.0);
        Vector3D v101( 10.0, -10.0,  50.0);
        Vector3D v011(-10.0,  10.0,  50.0);
        Vector3D v111( 10.0,  10.0,  50.0);

        Quad ceilWall(v010, v110, v111, v011);
        Quad floorWall(v000, v001, v101, v100);
        Quad backWall(v000, v100, v110, v010);
        Quad leftWall(v000, v010, v011, v001);
        Quad rightWall(v100, v101, v111, v110);

        scene->add(ceilWall, Material(Color(), Color(0.75, 0.75, 0.75), REFLECTION_DIFFUSE));
        scene->add(floorWall, Material(Color(), Color(0.75, 0.75, 0.75), REFLECTION_DIFFUSE));
        scene->add(backWall, Material(Color(), Color(0.75, 0.75, 0.75), REFLECTION_DIFFUSE));
        scene->add(leftWall, Material(Color(), Color(0.75, 0.25, 0.25), REFLECTION_DIFFUSE));
        scene->add(rightWall, Material(Color(), Color(0.25, 0.25, 0.75), REFLECTION_DIFFUSE));

        // Sphere
        Sphere sphere(2.0, Vector3D(-5.0, -8.0, 5.0));
        scene->add(sphere, Material(Color(), Color(0.25, 0.75, 0.25), REFLECTION_DIFFUSE));

        // Objects
        Trimesh dragon(kDataDirectory + "dragon.ply");
        dragon.scale(50.0, 50.0, 50.0);
        dragon.translate(Vector3D(2.0, 0.0, 0.0));
        dragon.putOnPlane(Plane(10.0, Vector3D(0.0, 1.0, 0.0)));
        scene->add(dragon, Material(Color(), Color(0.70, 0.60, 0.40), REFLECTION_REFRACTION));    

        (*camera) = Camera(width, height, 
                           Vector3D(0.0, 0.0, 100.0),
                           Vector3D(0.0, 0.0, -1.0),
                           Vector3D(0.0, 1.0, 0.0),
                           20.0,
                           42.0,
                           58.0,
                           1.0,
                           90.0);
    }

    void cornellBoxDragon(Scene* scene, Camera* camera, const int width, const int height) {
        scene->clear(); 

        // Back light
        scene->add(Sphere(2.0, Vector3D(-5.0, 5.0, -5.0)), Material(Color(64.0, 64.0, 64.0), Color(1.0, 1.0, 1.0), REFLECTION_DIFFUSE), true);

        // Walls
        Vector3D v000(-10.0, -10.0, -10.0);
        Vector3D v100( 10.0, -10.0, -10.0);
        Vector3D v010(-10.0,  10.0, -10.0);
        Vector3D v001(-10.0, -10.0,  50.0);
        Vector3D v110( 10.0,  10.0, -10.0);
        Vector3D v101( 10.0, -10.0,  50.0);
        Vector3D v011(-10.0,  10.0,  50.0);
        Vector3D v111( 10.0,  10.0,  50.0);

        Quad ceilWall(v010, v110, v111, v011);
        Quad floorWall(v000, v001, v101, v100);
        Quad backWall(v000, v100, v110, v010);
        Quad leftWall(v000, v010, v011, v001);
        Quad rightWall(v100, v101, v111, v110);

        scene->add(ceilWall, Material(Color(), Color(0.75, 0.75, 0.75), REFLECTION_DIFFUSE));
        scene->add(floorWall, Material(Color(), Color(0.75, 0.75, 0.75), REFLECTION_DIFFUSE));
        scene->add(backWall, Material(Color(), Color(0.75, 0.75, 0.75), REFLECTION_DIFFUSE));
        scene->add(leftWall, Material(Color(), Color(0.75, 0.25, 0.25), REFLECTION_DIFFUSE));
        scene->add(rightWall, Material(Color(), Color(0.25, 0.25, 0.75), REFLECTION_DIFFUSE));

        // Objects
        Trimesh dragon(kDataDirectory + "dragon.ply");
        dragon.scale(70.0, 70.0, 70.0);
        dragon.translate(Vector3D(0.0, 0.0, 0.0));
        dragon.putOnPlane(Plane(10.0, Vector3D(0.0, 1.0, 0.0)));
        dragon.buildAccel();

        scene->add(dragon, Material(Color(), Color(0.70, 0.60, 0.40), REFLECTION_SUBSURFACE));                         

        (*camera) = Camera(width, height, 
                           Vector3D(0.0, 0.0, 100.0),
                           Vector3D(0.0, 0.0, -1.0),
                           Vector3D(0.0, 1.0, 0.0),
                           20.0,
                           42.0,
                           58.0,
                           1.0,
                           90.0);
    }
    */

    void kittenBox(Scene* scene, Camera* camera, const int width, const int height) {
        scene->clear(); 

        //Vector3D l00(-5.0, 9.99, -5.0);
        //Vector3D l01(-5.0, 9.99,  5.0);
        //Vector3D l10( 5.0, 9.99, -5.0);
        //Vector3D l11( 5.0, 9.99,  5.0);
        //scene->add(Quad(l00, l10, l11, l01), LambertianBRDF::factory(Color(0.0, 0.0, 0.0)), Color(32.0, 32.0, 32.0), true);

        // Back light
        scene->setAreaLight(Sphere(Vector3D(-5.0, 5.0, -5.0), 3.0),
                            Color(32.0, 32.0, 32.0));

        // Walls
        Vector3D v000(-10.0, -10.0, -10.0);
        Vector3D v100( 10.0, -10.0, -10.0);
        Vector3D v010(-10.0,  10.0, -10.0);
        Vector3D v001(-10.0, -10.0,  50.0);
        Vector3D v110( 10.0,  10.0, -10.0);
        Vector3D v101( 10.0, -10.0,  50.0);
        Vector3D v011(-10.0,  10.0,  50.0);
        Vector3D v111( 10.0,  10.0,  50.0);

        Quad ceilWall(v010, v110, v111, v011);
        Quad floorWall(v000, v001, v101, v100);
        Quad backWall(v000, v100, v110, v010);
        Quad leftWall(v000, v010, v011, v001);
        Quad rightWall(v100, v101, v111, v110);

        scene->addShape(ceilWall,  LambertianBRDF::factory(Color(0.75, 0.75, 0.75)));
        scene->addShape(floorWall, LambertianBRDF::factory(Color(0.75, 0.75, 0.75)));
        scene->addShape(backWall,  LambertianBRDF::factory(Color(0.75, 0.75, 0.75)));
        scene->addShape(leftWall,  LambertianBRDF::factory(Color(0.75, 0.25, 0.25)));
        scene->addShape(rightWall, LambertianBRDF::factory(Color(0.25, 0.25, 0.75)));

        // Objects
        Trimesh kitten(kDataDirectory + "kitten.ply");
        kitten.scale(0.15, 0.15, 0.15);
        kitten.putOnPlane(Plane(10.0, Vector3D(0.0, 1.0, 0.0)));

        const Color sigmap_s = Color(2.19, 2.62, 3.00) * 0.1;
        const Color sigma_a  = Color(0.0021, 0.0041, 0.0071) * 10.0;
        const BSSRDF bssrdf = DipoleBSSRDF::factory(sigma_a, sigmap_s, 1.5);

        BSDF kittenBsdf = RefractiveBSDF::factory(Color(0.999, 0.999, 0.999));
        kittenBsdf.setBssrdf(bssrdf);
        scene->addShape(kitten, kittenBsdf);

        scene->setAccelType(AccelType::QBVH);
        scene->finalize();

        (*camera) = Camera::asDoF(width, height, 
                                  Vector3D(0.0, 0.0, 100.0),
                                  Vector3D(0.0, 0.0, -1.0),
                                  Vector3D(0.0, 1.0, 0.0),
                                  20.0,
                                  42.0,
                                  58.0,
                                  1.0,
                                  90.0);
    }

    void kittenEnvmap(Scene* scene, Camera* camera, const int width, const int height) {
        scene->clear(); 

        // Objects
        Trimesh kitten(kDataDirectory + "kitten.ply");
        kitten.scale(0.15, 0.15, 0.15);
        kitten.putOnPlane(Plane(10.0, Vector3D(0.0, 1.0, 0.0)));

        const Color sigmap_s = Color(2.19, 2.62, 3.00) * 0.1;
        const Color sigma_a  = Color(0.0021, 0.0041, 0.0071) * 10.0;
        const BSSRDF bssrdf = DipoleBSSRDF::factory(sigma_a, sigmap_s, 1.5);

        BSDF kittenBsdf = RefractiveBSDF::factory(Color(0.999, 0.999, 0.999));
        kittenBsdf.setBssrdf(bssrdf);
        scene->addShape(kitten, kittenBsdf);
        
        Disk disk(Vector3D(0.0, -10.0, 0.0), Vector3D(0.0, 1.0, 0.0), 20.0);
        scene->addShape(disk, LambertianBRDF::factory(Color(0.95, 0.95, 0.95)));

        (*camera) = Camera::asDoF(width, height, 
                                  Vector3D(0.0, 5.0, 100.0),
                                  Vector3D(0.0, -5.0, -100.0).normalized(),
                                  Vector3D(0.0, 1.0, 0.0),
                                  20.0,
                                  42.0,
                                  58.0,
                                  1.0,
                                  90.0);

        // Envmap
        Image envmap = Image::fromFile(kDataDirectory + "cave_room.hdr");
        scene->setEnvmap(envmap, *camera);

        scene->finalize();
    }

}  // namespace spica
