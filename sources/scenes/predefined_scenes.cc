#define SPICA_API_EXPORT
#include "predefined_scenes.h"

#include "../core/common.h"
#include "../image/image.h"
#include "../scenes/scene.h"
#include "../camera/camera.h"
#include "../light/spica_light.h"
#include "../shape/spica_shape.h"

#include "../accel/bbvh_accel.h"
#include "../accel/qbvh_accel.h"

#include "../material/spica_material.h"
#include "../texture/constant.h"

namespace spica {
    void cornellBox(Scene* scene, Camera* camera, const int width, const int height) {
        std::vector<std::shared_ptr<Primitive>> primitives;
        std::vector<std::shared_ptr<Light>> lights;

        // Light
        {
            Point l00(-5.0, 9.99, -5.0);
            Point l01(-5.0, 9.99,  5.0);
            Point l10( 5.0, 9.99, -5.0);
            Point l11( 5.0, 9.99,  5.0);
            Spectrum Le(8.0, 8.0, 8.0);
            auto t1 = std::make_shared<Triangle>(l00, l10, l11);
            auto t2 = std::make_shared<Triangle>(l00, l11, l01);
            auto l1 = std::make_shared<AreaLight>(t1, Transform(), Le);
            auto l2 = std::make_shared<AreaLight>(t2, Transform(), Le);
            auto lightKd   = std::make_shared<ConstantTexture<Spectrum>>(Spectrum(0.99, 0.99, 0.99));
            auto lightMtrl = std::make_shared<LambertianMaterial>(lightKd); 
            primitives.emplace_back(new GeometricPrimitive(t1, lightMtrl, l1));
            primitives.emplace_back(new GeometricPrimitive(t2, lightMtrl, l2));
            lights.push_back(l1);
            lights.push_back(l2);
        }

        Point v000(-10.0, -10.0, -10.0);
        Point v100( 10.0, -10.0, -10.0);
        Point v010(-10.0,  10.0, -10.0);
        Point v001(-10.0, -10.0,  50.0);
        Point v110( 10.0,  10.0, -10.0);
        Point v101( 10.0, -10.0,  50.0);
        Point v011(-10.0,  10.0,  50.0);
        Point v111( 10.0,  10.0,  50.0);
        
        // Ceil
        {
            auto t1 = std::make_shared<Triangle>(v010, v110, v111);
            auto t2 = std::make_shared<Triangle>(v010, v111, v011);
            auto ceilKd   = std::make_shared<ConstantTexture<Spectrum>>(Spectrum(0.75, 0.75, 0.75));
            auto ceilMtrl = std::make_shared<LambertianMaterial>(ceilKd);
            primitives.emplace_back(new GeometricPrimitive(t1, ceilMtrl, nullptr));
            primitives.emplace_back(new GeometricPrimitive(t2, ceilMtrl, nullptr));
        }

        // Floor
        {
            auto t1 = std::make_shared<Triangle>(v000, v001, v101);
            auto t2 = std::make_shared<Triangle>(v000, v101, v100);
            auto floorKd   = std::make_shared<ConstantTexture<Spectrum>>(Spectrum(0.75, 0.75, 0.75));
            auto floorMtrl = std::make_shared<LambertianMaterial>(floorKd);
            primitives.emplace_back(new GeometricPrimitive(t1, floorMtrl, nullptr));
            primitives.emplace_back(new GeometricPrimitive(t2, floorMtrl, nullptr));
        }

        // Back
        {
            auto t1 = std::make_shared<Triangle>(v000, v100, v110);
            auto t2 = std::make_shared<Triangle>(v000, v110, v010);
            auto backKd   = std::make_shared<ConstantTexture<Spectrum>>(Spectrum(0.75, 0.75, 0.75));
            auto backMtrl = std::make_shared<LambertianMaterial>(backKd);
            primitives.emplace_back(new GeometricPrimitive(t1, backMtrl, nullptr));
            primitives.emplace_back(new GeometricPrimitive(t2, backMtrl, nullptr));
        }

        // Left
        {
            auto t1 = std::make_shared<Triangle>(v000, v010, v011);
            auto t2 = std::make_shared<Triangle>(v000, v011, v001);
            auto leftKd   = std::make_shared<ConstantTexture<Spectrum>>(Spectrum(0.75, 0.25, 0.25));
            auto leftMtrl = std::make_shared<LambertianMaterial>(leftKd);
            primitives.emplace_back(new GeometricPrimitive(t1, leftMtrl, nullptr));
            primitives.emplace_back(new GeometricPrimitive(t2, leftMtrl, nullptr));
        }

        // Right
        {
            auto t1 = std::make_shared<Triangle>(v100, v101, v111);
            auto t2 = std::make_shared<Triangle>(v100, v111, v110);
            auto rightKd   = std::make_shared<ConstantTexture<Spectrum>>(Spectrum(0.25, 0.75, 0.25));
            auto rightMtrl = std::make_shared<LambertianMaterial>(rightKd);
            primitives.emplace_back(new GeometricPrimitive(t1, rightMtrl, nullptr));
            primitives.emplace_back(new GeometricPrimitive(t2, rightMtrl, nullptr));
        }

        // Mirror ball
        {            
            auto sph  = std::make_shared<Sphere>(Point(-5.0, -7.0, -5.0), 3.0);
            //auto Kd   = std::make_shared<ConstantTexture<Spectrum>>(Spectrum(0.99, 0.99, 0.99));
            //auto mtrl = std::make_shared<MirrorMaterial>(Kd);
            auto eta = std::make_shared<ConstantTexture<Spectrum>>(Spectrum(0.18309, 0.54410, 1.1715));
            auto k   = std::make_shared<ConstantTexture<Spectrum>>(Spectrum(3.4241, 2.1404, 1.7544));
            auto roughness = std::make_shared<ConstantTexture<double>>(0.1);
            auto mtrl = std::make_shared<MetalMaterial>(eta, k, roughness);
            primitives.emplace_back(new GeometricPrimitive(sph, mtrl, nullptr));
        }

        // Subsurface ball
        {
            auto sph  = std::make_shared<Sphere>(Point(0.0, -7.0, 0.0), 3.0);
            auto Kr   = std::make_shared<ConstantTexture<Spectrum>>(Spectrum(0.81, 0.81, 0.69));
            auto Kt   = std::make_shared<ConstantTexture<Spectrum>>(Spectrum(0.99, 0.99, 0.99));
            double g = 0.5;
            double eta = 1.3;
            auto sigS = std::make_shared<ConstantTexture<Spectrum>>(Spectrum(0.70, 0.88, 1.01) / (1.0 - g));
            auto sigA = std::make_shared<ConstantTexture<Spectrum>>(Spectrum(0.0014, 0.0025, 0.0142));
            auto mtrl = std::make_shared<SubsurfaceMaterial>(1.0, Kr, Kt, sigA, sigS, g, eta);
            primitives.emplace_back(new GeometricPrimitive(sph, mtrl, nullptr));
        }
        
        // Glass ball
        {
            auto sph = std::make_shared<Sphere>(Point(5.0, -7.0, 5.0), 3.0);
            auto Kr  = std::make_shared<ConstantTexture<Spectrum>>(Spectrum(0.99, 0.99, 0.99));
            auto rough = std::make_shared<ConstantTexture<double>>(0.0);
            auto ior = std::make_shared<ConstantTexture<double>>(1.5);
            auto mtrl = std::make_shared<GlassMaterial>(Kr, Kr, rough, rough, ior);
            primitives.emplace_back(new GeometricPrimitive(sph, mtrl, nullptr));
        }

        auto accel = std::make_shared<BBVHAccel>(primitives);
        *scene = Scene(accel, lights);
    }

    /*
    void cornellBoxBunny(Scene* scene, Camera* camera, const int width, const int height) {
        scene->clear(); 

        // Light
        Vector3D l00(-5.0, 9.99, -5.0);
        Vector3D l01(-5.0, 9.99,  5.0);
        Vector3D l10( 5.0, 9.99, -5.0);
        Vector3D l11( 5.0, 9.99,  5.0);
        scene->add(Quad(l00, l10, l11, l01), Material(Spectrum(32.0, 32.0, 32.0), Spectrum(1.0, 1.0, 1.0), REFLECTION_DIFFUSE), true);

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

        const BRDF brdf = PhongBRDF::factory(Spectrum(0.75, 0.75, 0.75), 32.0);
        scene->add(floorWall, Material(Spectrum(), Spectrum(0.75, 0.75, 0.75), REFLECTION_BRDF, brdf));
        scene->add(ceilWall, Material(Spectrum(), Spectrum(0.75, 0.75, 0.75), REFLECTION_DIFFUSE));
        scene->add(backWall, Material(Spectrum(), Spectrum(0.75, 0.75, 0.75), REFLECTION_DIFFUSE));
        scene->add(leftWall, Material(Spectrum(), Spectrum(0.75, 0.25, 0.25), REFLECTION_DIFFUSE));
        scene->add(rightWall, Material(Spectrum(), Spectrum(0.25, 0.25, 0.75), REFLECTION_DIFFUSE));

        // Objects
        Trimesh bunny(kDataDirectory + "bunny.ply");
        bunny.translate(Vector3D(-3.0, 0.0, 0.0));
        bunny.putOnPlane(Plane(10.0, Vector3D(0.0, 1.0, 0.0)));
        scene->add(bunny, Material(Spectrum(), Spectrum(0.75, 0.75, 0.25), REFLECTION_DIFFUSE));                         

        scene->add(Sphere(3.0,  Vector3D(5.0, -7.0, 5.0)), Material(Spectrum(), Spectrum(0.99, 0.99, 0.99), REFLECTION_REFRACTION));

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
        //scene->add(Quad(l00, l10, l11, l01), Material(Spectrum(32.0, 32.0, 32.0), Spectrum(1.0, 1.0, 1.0), REFLECTION_DIFFUSE), true);

        // Back light
        Vector3D l00(3.0, -3.0, -9.99);
        Vector3D l01(8.0, -3.0, -9.99);
        Vector3D l10(3.0, -8.0, -9.99);
        Vector3D l11(8.0, -8.0, -9.99);
        scene->add(Quad(l00, l10, l11, l01), Material(Spectrum(128.0, 128.0, 128.0), Spectrum(1.0, 1.0, 1.0), REFLECTION_DIFFUSE), true);

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

        scene->add(ceilWall, Material(Spectrum(), Spectrum(0.75, 0.75, 0.75), REFLECTION_DIFFUSE));
        scene->add(floorWall, Material(Spectrum(), Spectrum(0.75, 0.75, 0.75), REFLECTION_DIFFUSE));
        scene->add(backWall, Material(Spectrum(), Spectrum(0.75, 0.75, 0.75), REFLECTION_DIFFUSE));
        scene->add(leftWall, Material(Spectrum(), Spectrum(0.75, 0.25, 0.25), REFLECTION_DIFFUSE));
        scene->add(rightWall, Material(Spectrum(), Spectrum(0.25, 0.25, 0.75), REFLECTION_DIFFUSE));

        // Sphere
        Sphere sphere(2.0, Vector3D(-5.0, -8.0, 5.0));
        scene->add(sphere, Material(Spectrum(), Spectrum(0.25, 0.75, 0.25), REFLECTION_DIFFUSE));

        // Objects
        Trimesh dragon(kDataDirectory + "dragon.ply");
        dragon.scale(50.0, 50.0, 50.0);
        dragon.translate(Vector3D(2.0, 0.0, 0.0));
        dragon.putOnPlane(Plane(10.0, Vector3D(0.0, 1.0, 0.0)));
        scene->add(dragon, Material(Spectrum(), Spectrum(0.70, 0.60, 0.40), REFLECTION_REFRACTION));    

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
        scene->add(Sphere(2.0, Vector3D(-5.0, 5.0, -5.0)), Material(Spectrum(64.0, 64.0, 64.0), Spectrum(1.0, 1.0, 1.0), REFLECTION_DIFFUSE), true);

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

        scene->add(ceilWall, Material(Spectrum(), Spectrum(0.75, 0.75, 0.75), REFLECTION_DIFFUSE));
        scene->add(floorWall, Material(Spectrum(), Spectrum(0.75, 0.75, 0.75), REFLECTION_DIFFUSE));
        scene->add(backWall, Material(Spectrum(), Spectrum(0.75, 0.75, 0.75), REFLECTION_DIFFUSE));
        scene->add(leftWall, Material(Spectrum(), Spectrum(0.75, 0.25, 0.25), REFLECTION_DIFFUSE));
        scene->add(rightWall, Material(Spectrum(), Spectrum(0.25, 0.25, 0.75), REFLECTION_DIFFUSE));

        // Objects
        Trimesh dragon(kDataDirectory + "dragon.ply");
        dragon.scale(70.0, 70.0, 70.0);
        dragon.translate(Vector3D(0.0, 0.0, 0.0));
        dragon.putOnPlane(Plane(10.0, Vector3D(0.0, 1.0, 0.0)));
        dragon.buildAccel();

        scene->add(dragon, Material(Spectrum(), Spectrum(0.70, 0.60, 0.40), REFLECTION_SUBSURFACE));                         

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

/*
    void kittenBox(Scene* scene, Camera* camera, const int width, const int height) {
        scene->clear(); 

        //Vector3D l00(-5.0, 9.99, -5.0);
        //Vector3D l01(-5.0, 9.99,  5.0);
        //Vector3D l10( 5.0, 9.99, -5.0);
        //Vector3D l11( 5.0, 9.99,  5.0);
        //scene->add(Quad(l00, l10, l11, l01), LambertianBRDF::factory(Spectrum(0.0, 0.0, 0.0)), Spectrum(32.0, 32.0, 32.0), true);

        // Back light
        scene->setAreaLight(Sphere(Point(5.0, 5.0, 5.0), 2.0),
                            Spectrum(64.0, 64.0, 64.0));

        // Walls
        Point v000(-10.0, -10.0, -10.0);
        Point v100( 10.0, -10.0, -10.0);
        Point v010(-10.0,  10.0, -10.0);
        Point v001(-10.0, -10.0,  50.0);
        Point v110( 10.0,  10.0, -10.0);
        Point v101( 10.0, -10.0,  50.0);
        Point v011(-10.0,  10.0,  50.0);
        Point v111( 10.0,  10.0,  50.0);

        Quad ceilWall(v010, v110, v111, v011);
        Quad floorWall(v000, v001, v101, v100);
        Quad backWall(v000, v100, v110, v010);
        Quad leftWall(v000, v010, v011, v001);
        Quad rightWall(v100, v101, v111, v110);

        scene->addShape(ceilWall,  LambertianBRDF::factory(Spectrum(0.75, 0.75, 0.75)));
        scene->addShape(floorWall, LambertianBRDF::factory(Spectrum(0.75, 0.75, 0.75)));
        scene->addShape(backWall,  LambertianBRDF::factory(Spectrum(0.75, 0.75, 0.75)));
        scene->addShape(leftWall,  LambertianBRDF::factory(Spectrum(0.75, 0.25, 0.25)));
        scene->addShape(rightWall, LambertianBRDF::factory(Spectrum(0.25, 0.25, 0.75)));

        // Objects
        Trimesh kitten(kDataDirectory + "kitten.ply");
        kitten.scale(0.1, 0.1, 0.1);
        // kitten.putOnPlane(Plane(10.0, Vector3D(0.0, 1.0, 0.0)));

        // BSDF kittenBsdf = LambertianBRDF::factory(Spectrum(0.81, 0.81, 0.69));
        BSDF kittenBsdf = SpecularBRDF::factory(Spectrum(0.999, 0.999, 0.999));
        const Spectrum sigmap_s = Spectrum(0.70, 1.22, 1.90);
        const Spectrum sigma_a  = Spectrum(0.0014, 0.0025, 0.0142);
        const BSSRDF bssrdf = DipoleBSSRDF::factory(sigma_a, sigmap_s, 1.3);
        kittenBsdf.setBssrdf(bssrdf);
        scene->addShape(kitten, kittenBsdf);
        // scene->addShape(kitten, LambertianBRDF::factory(Spectrum(0.75, 0.25, 0.25)));

        scene->setAccelType(AccelType::QBVH);
        scene->finalize();

        (*camera) = Camera::asDoF(width, height, 
                                  Point(0.0, 0.0, 100.0),
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
        // kitten.putOnPlane(Plane(10.0, Vector3D(0.0, 1.0, 0.0)));

        const Spectrum sigmap_s = Spectrum(2.19, 2.62, 3.00);
        const Spectrum sigma_a  = Spectrum(0.0021, 0.0041, 0.0071);
        const BSSRDF bssrdf = DipoleBSSRDF::factory(sigma_a, sigmap_s, 1.5);

        // Set floor
        const int tiles = 8;
        const double tileSize = 8.0;
        for (int i = 0; i <= tiles; i++) {
            for (int j = 0; j <= tiles; j++) {
                double ii = (i - tiles / 2) * tileSize;
                double jj = (j - tiles / 2) * tileSize;
                Point p00(ii, -10.0, jj);
                Point p01(ii + tileSize, -10.0, jj);
                Point p10(ii, -10.0, jj + tileSize);
                Point p11(ii + tileSize, -10.0, jj + tileSize);
                Spectrum color = (i + j) % 2 == 0 ? Spectrum(0.9, 0.9, 0.9) 
                                                  : Spectrum(0.2, 0.2, 0.2);
                BSDF bsdf = (i + j) % 2 == 0 ? LambertianBRDF::factory(color)
                                             : LambertianBRDF::factory(color); 
                scene->addShape(Triangle(p00, p11, p01), bsdf);
                scene->addShape(Triangle(p00, p10, p11), bsdf);
            }
        }


        // BSDF kittenBsdf = PhongBRDF::factory(Spectrum(0.999, 0.999, 0.999));
        BSDF kittenBsdf = LambertianBRDF::factory(Spectrum(0.75, 0.75, 0.25));
        kittenBsdf.setBssrdf(bssrdf);
        scene->addShape(kitten, kittenBsdf);

        Point eye(0.0, 10.0, 100.0);
        (*camera) = Camera::asDoF(width, height, eye, -Vector3D(eye).normalized(),
                                  Vector3D(0.0, 1.0, 0.0),
                                  20.0,
                                  42.0,
                                  58.0,
                                  1.0e-1,
                                  9000.0);

        // Envmap
        Image envmap = Image::fromFile(kDataDirectory + "gold_room.hdr");
        scene->setEnvmap(envmap);

        scene->finalize();
    }
    */

}  // namespace spica
