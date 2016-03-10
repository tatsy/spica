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

#include "../medium/medium.h"
#include "../medium/homogeneous.h"

#include "../material/spica_material.h"
#include "../texture/constant.h"

namespace spica {
    void cornellBox(Scene* scene,
                    std::vector<Triangle>* tris,
                    std::vector<Spectrum>* Kd) {
        std::vector<std::shared_ptr<Primitive>> primitives;
        std::vector<std::shared_ptr<Light>> lights;

        if (tris) tris->clear();
        if (Kd)   Kd->clear();

        // Light
        {
            Point3d l00(-5.0, 9.9, -5.0);
            Point3d l01(-5.0, 9.9,  5.0);
            Point3d l10( 5.0, 9.9, -5.0);
            Point3d l11( 5.0, 9.9,  5.0);
            Spectrum Le(8.0, 8.0, 8.0);
            auto t1 = std::make_shared<Triangle>(l00, l10, l11);
            auto t2 = std::make_shared<Triangle>(l00, l11, l01);
            auto l1 = std::make_shared<AreaLight>(t1, Transform(), Le);
            auto l2 = std::make_shared<AreaLight>(t2, Transform(), Le);
            Spectrum kd = Spectrum(0.999, 0.999, 0.999);
            auto lightKd   = std::make_shared<ConstantTexture<Spectrum>>(kd);
            auto lightMtrl = std::make_shared<LambertianMaterial>(lightKd); 
            primitives.emplace_back(new GeometricPrimitive(t1, lightMtrl, l1));
            primitives.emplace_back(new GeometricPrimitive(t2, lightMtrl, l2));
            lights.push_back(l1);
            lights.push_back(l2);

            if (tris) {
                tris->push_back(*t1.get());
                tris->push_back(*t2.get());
            }

            if (Kd) {
                Kd->push_back(kd);
                Kd->push_back(kd);
            }
        }

        Point3d v000(-10.0, -10.0, -10.0);
        Point3d v100( 10.0, -10.0, -10.0);
        Point3d v010(-10.0,  10.0, -10.0);
        Point3d v001(-10.0, -10.0,  50.0);
        Point3d v110( 10.0,  10.0, -10.0);
        Point3d v101( 10.0, -10.0,  50.0);
        Point3d v011(-10.0,  10.0,  50.0);
        Point3d v111( 10.0,  10.0,  50.0);
        
        // Ceil
        {
            auto t1 = std::make_shared<Triangle>(v010, v110, v111);
            auto t2 = std::make_shared<Triangle>(v010, v111, v011);
            Spectrum kd(0.75, 0.75, 0.75);
            auto ceilKd   = std::make_shared<ConstantTexture<Spectrum>>(kd);
            auto ceilMtrl = std::make_shared<LambertianMaterial>(ceilKd);
            primitives.emplace_back(new GeometricPrimitive(t1, ceilMtrl, nullptr));
            primitives.emplace_back(new GeometricPrimitive(t2, ceilMtrl, nullptr));

            if (tris) {
                tris->push_back(*t1.get());
                tris->push_back(*t2.get());
            }

            if (Kd) {
                Kd->push_back(kd);
                Kd->push_back(kd);
            }
        }

        // Floor
        {
            auto t1 = std::make_shared<Triangle>(v000, v001, v101);
            auto t2 = std::make_shared<Triangle>(v000, v101, v100);
            Spectrum kd(0.75, 0.75, 0.75);
            auto floorKd   = std::make_shared<ConstantTexture<Spectrum>>(kd);
            auto floorMtrl = std::make_shared<LambertianMaterial>(floorKd);
            primitives.emplace_back(new GeometricPrimitive(t1, floorMtrl, nullptr));
            primitives.emplace_back(new GeometricPrimitive(t2, floorMtrl, nullptr));

            if (tris) {
                tris->push_back(*t1.get());
                tris->push_back(*t2.get());
            }

            if (Kd) {
                Kd->push_back(kd);
                Kd->push_back(kd);
            }
        }

        // Back
        {
            auto t1 = std::make_shared<Triangle>(v000, v100, v110);
            auto t2 = std::make_shared<Triangle>(v000, v110, v010);
            Spectrum kd(0.75, 0.75, 0.75);
            auto backKd   = std::make_shared<ConstantTexture<Spectrum>>(kd);
            auto backMtrl = std::make_shared<LambertianMaterial>(backKd);
            primitives.emplace_back(new GeometricPrimitive(t1, backMtrl, nullptr));
            primitives.emplace_back(new GeometricPrimitive(t2, backMtrl, nullptr));

            if (tris) {
                tris->push_back(*t1.get());
                tris->push_back(*t2.get());
            }

            if (Kd) {
                Kd->push_back(kd);
                Kd->push_back(kd);
            }

        }

        // Left
        {
            auto t1 = std::make_shared<Triangle>(v000, v010, v011);
            auto t2 = std::make_shared<Triangle>(v000, v011, v001);
            Spectrum kd(0.75, 0.25, 0.25);
            auto leftKd   = std::make_shared<ConstantTexture<Spectrum>>(kd);
            auto leftMtrl = std::make_shared<LambertianMaterial>(leftKd);
            primitives.emplace_back(new GeometricPrimitive(t1, leftMtrl, nullptr));
            primitives.emplace_back(new GeometricPrimitive(t2, leftMtrl, nullptr));

            if (tris) {
                tris->push_back(*t1.get());
                tris->push_back(*t2.get());
            }

            if (Kd) {
                Kd->push_back(kd);
                Kd->push_back(kd);
            }
        }

        // Right
        {
            auto t1 = std::make_shared<Triangle>(v100, v101, v111);
            auto t2 = std::make_shared<Triangle>(v100, v111, v110);
            Spectrum kd(0.25, 0.75, 0.25);
            auto rightKd   = std::make_shared<ConstantTexture<Spectrum>>(kd);
            auto rightMtrl = std::make_shared<LambertianMaterial>(rightKd);
            primitives.emplace_back(new GeometricPrimitive(t1, rightMtrl, nullptr));
            primitives.emplace_back(new GeometricPrimitive(t2, rightMtrl, nullptr));

            if (tris) {
                tris->push_back(*t1.get());
                tris->push_back(*t2.get());
            }

            if (Kd) {
                Kd->push_back(kd);
                Kd->push_back(kd);
            }

        }

        // Mirror ball
        {            
            auto sph  = std::make_shared<Sphere>(Point3d(-5.0, -7.0, -5.0), 3.0);
            //auto Kd   = std::make_shared<ConstantTexture<Spectrum>>(Spectrum(0.99, 0.99, 0.99));
            //auto mtrl = std::make_shared<MirrorMaterial>(Kd);
            auto eta = std::make_shared<ConstantTexture<Spectrum>>(Spectrum(0.18309, 0.54410, 1.1715));
            auto k   = std::make_shared<ConstantTexture<Spectrum>>(Spectrum(3.4241, 2.1404, 1.7544));
            auto roughness = std::make_shared<ConstantTexture<double>>(0.1);
            auto mtrl = std::make_shared<MetalMaterial>(eta, k, roughness);
            primitives.emplace_back(new GeometricPrimitive(sph, mtrl, nullptr));
        }

        // Subsurface ball
        /*
        {
            // Regular milk
            const double scale = 5000.0;
            const double eta   = 1.3;
            const double g     = 0.7;
            // Regular milk
            Spectrum sigma_a = Spectrum(0.0015333, 0.0046, 0.019933);
            Spectrum sigma_s = Spectrum(4.5513, 5.8294, 7.136);
            // Skin1
            //Spectrum sigma_a = Spectrum(0.0032, 0.17, 0.48);
            //Spectrum sigma_s = Spectrum(0.74, 0.88, 1.01);
            // Skin2
            //Spectrum sigma_a = Spectrum(0.013, 0.070, 0.145);
            //Spectrum sigma_s = Spectrum(1.09, 1.59, 1.79);
            // Marble
            //const double eta = 1.5;
            //Spectrum sigma_a = Spectrum(0.0021, 0.0041, 0.0071);
            //Spectrum sigma_s = Spectrum(2.19, 2.62, 3.00);

            auto Kr   = std::make_shared<ConstantTexture<Spectrum>>(Spectrum(0.99, 0.99, 0.99));
            auto Kt   = std::make_shared<ConstantTexture<Spectrum>>(Spectrum(0.99, 0.99, 0.99));
            auto sigA = std::make_shared<ConstantTexture<Spectrum>>(sigma_a);
            auto sigS = std::make_shared<ConstantTexture<Spectrum>>(sigma_s);
            auto mtrl = std::make_shared<SubsurfaceMaterial>(scale, Kr, Kt, sigA, sigS, g, eta);

            Transform o2w = Transform::translate(Vector3d(5.0, -12.0, 5.0)) * Transform::scale(50.0, 50.0, 50.0);

            PLYMeshIO meshio;
            auto tris = meshio.load(kDataDirectory + "bunny.ply", o2w);
            for (const auto& t : tris) {
                primitives.emplace_back(new GeometricPrimitive(t, mtrl, nullptr));                
            }
        }
        */

        // Volume material
        {
            // auto sph = std::make_shared<Sphere>(Point3d(0.0, -7.0, 0.0), 3.0);
            double scale = 10.0;
            double g = 0.7;
            // Budweiser
            //Spectrum sigma_a = Spectrum(0.006164, 0.013984, 0.034983) * scale;
            //Spectrum sigma_s = Spectrum(5.0922e-05, 4.301e-05, 0) * scale;
            // Grapefruit juice
            //Spectrum sigma_a = Spectrum(0.0138, 0.018831, 0.056781) * scale;
            //Spectrum sigma_s = Spectrum(0.22826, 0.23998, 0.32748) * scale;
            // Regular milk
            Spectrum sigma_a = Spectrum(0.0015333, 0.0046, 0.019933) * scale;
            Spectrum sigma_s = Spectrum(4.5513, 5.8294, 7.136) * scale;
            // Espresso
            //Spectrum sigma_a = Spectrum(4.72378, 6.5751, 8.8493) * scale;
            //Spectrum sigma_s = Spectrum(0.72378, 0.84557, 1.0247) * scale;
            // Coke
            //Spectrum sigma_a = Spectrum(0.10014, 0.16503, 0.2468) * scale;
            //Spectrum sigma_s = Spectrum(8.9053e-5, 8.372e-5, 0.0) * scale;
            Medium* medium = new HomogeneousMedium(sigma_a, sigma_s, g);
            auto mediumInterface = std::make_shared<MediumInterface>(medium, nullptr);

            auto Kr  = std::make_shared<ConstantTexture<Spectrum>>(Spectrum(0.99, 0.99, 0.99));
            auto rough = std::make_shared<ConstantTexture<double>>(0.0);
            auto ior = std::make_shared<ConstantTexture<double>>(1.5);
            auto mtrl = std::make_shared<GlassMaterial>(Kr, Kr, rough, rough, ior);

            Transform o2w = Transform::translate(Vector3d(5.0, -12.0, -5.0)) * Transform::scale(50.0, 50.0, 50.0);

            PLYMeshIO meshio;
            auto tris = meshio.load(kDataDirectory + "bunny.ply", o2w);
            for (const auto& t : tris) {
                primitives.emplace_back(new GeometricPrimitive(t, mtrl, nullptr, mediumInterface));                
            }

            // primitives.emplace_back(new GeometricPrimitive(sph, mtrl, nullptr, mediumInterface));
        }
        
        // Glass ball
        {
            //auto sph = std::make_shared<Sphere>(Point3d(5.0, -7.0, 5.0), 3.0);
            //auto Kr  = std::make_shared<ConstantTexture<Spectrum>>(Spectrum(0.99, 0.99, 0.99));
            //auto rough = std::make_shared<ConstantTexture<double>>(0.0);
            //auto ior = std::make_shared<ConstantTexture<double>>(1.5);
            //auto mtrl = std::make_shared<GlassMaterial>(Kr, Kr, rough, rough, ior);
            //primitives.emplace_back(new GeometricPrimitive(sph, mtrl, nullptr));
        }

        auto accel = std::make_shared<BBVHAccel>(primitives);
        *scene = Scene(accel, lights);
    }

    /*
    void cornellBoxBunny(Scene* scene, Camera* camera, const int width, const int height) {
        scene->clear(); 

        // Light
        Vector3d l00(-5.0, 9.99, -5.0);
        Vector3d l01(-5.0, 9.99,  5.0);
        Vector3d l10( 5.0, 9.99, -5.0);
        Vector3d l11( 5.0, 9.99,  5.0);
        scene->add(Quad(l00, l10, l11, l01), Material(Spectrum(32.0, 32.0, 32.0), Spectrum(1.0, 1.0, 1.0), REFLECTION_DIFFUSE), true);

        // Walls
        Vector3d v000(-10.0, -10.0, -10.0);
        Vector3d v100( 10.0, -10.0, -10.0);
        Vector3d v010(-10.0,  10.0, -10.0);
        Vector3d v001(-10.0, -10.0,  50.0);
        Vector3d v110( 10.0,  10.0, -10.0);
        Vector3d v101( 10.0, -10.0,  50.0);
        Vector3d v011(-10.0,  10.0,  50.0);
        Vector3d v111( 10.0,  10.0,  50.0);

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
        bunny.translate(Vector3d(-3.0, 0.0, 0.0));
        bunny.putOnPlane(Plane(10.0, Vector3d(0.0, 1.0, 0.0)));
        scene->add(bunny, Material(Spectrum(), Spectrum(0.75, 0.75, 0.25), REFLECTION_DIFFUSE));                         

        scene->add(Sphere(3.0,  Vector3d(5.0, -7.0, 5.0)), Material(Spectrum(), Spectrum(0.99, 0.99, 0.99), REFLECTION_REFRACTION));

        (*camera) = Camera(width, height, 
                           Vector3d(0.0, 0.0, 100.0),
                           Vector3d(0.0, 0.0, -1.0),
                           Vector3d(0.0, 1.0, 0.0),
                           20.0,
                           42.0,
                           58.0,
                           1.0,
                           90.0);
    }

    void cornellBoxOcclusion(Scene* scene, Camera* camera, const int width, const int height) {
        scene->clear(); 

        // Light
        //Vector3d l00(-5.0, 9.99, -5.0);
        //Vector3d l01(-5.0, 9.99,  5.0);
        //Vector3d l10( 5.0, 9.99, -5.0);
        //Vector3d l11( 5.0, 9.99,  5.0);
        //scene->add(Quad(l00, l10, l11, l01), Material(Spectrum(32.0, 32.0, 32.0), Spectrum(1.0, 1.0, 1.0), REFLECTION_DIFFUSE), true);

        // Back light
        Vector3d l00(3.0, -3.0, -9.99);
        Vector3d l01(8.0, -3.0, -9.99);
        Vector3d l10(3.0, -8.0, -9.99);
        Vector3d l11(8.0, -8.0, -9.99);
        scene->add(Quad(l00, l10, l11, l01), Material(Spectrum(128.0, 128.0, 128.0), Spectrum(1.0, 1.0, 1.0), REFLECTION_DIFFUSE), true);

        // Walls
        Vector3d v000(-10.0, -10.0, -10.0);
        Vector3d v100( 10.0, -10.0, -10.0);
        Vector3d v010(-10.0,  10.0, -10.0);
        Vector3d v001(-10.0, -10.0,  50.0);
        Vector3d v110( 10.0,  10.0, -10.0);
        Vector3d v101( 10.0, -10.0,  50.0);
        Vector3d v011(-10.0,  10.0,  50.0);
        Vector3d v111( 10.0,  10.0,  50.0);

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
        Sphere sphere(2.0, Vector3d(-5.0, -8.0, 5.0));
        scene->add(sphere, Material(Spectrum(), Spectrum(0.25, 0.75, 0.25), REFLECTION_DIFFUSE));

        // Objects
        Trimesh dragon(kDataDirectory + "dragon.ply");
        dragon.scale(50.0, 50.0, 50.0);
        dragon.translate(Vector3d(2.0, 0.0, 0.0));
        dragon.putOnPlane(Plane(10.0, Vector3d(0.0, 1.0, 0.0)));
        scene->add(dragon, Material(Spectrum(), Spectrum(0.70, 0.60, 0.40), REFLECTION_REFRACTION));    

        (*camera) = Camera(width, height, 
                           Vector3d(0.0, 0.0, 100.0),
                           Vector3d(0.0, 0.0, -1.0),
                           Vector3d(0.0, 1.0, 0.0),
                           20.0,
                           42.0,
                           58.0,
                           1.0,
                           90.0);
    }

    void cornellBoxDragon(Scene* scene, Camera* camera, const int width, const int height) {
        scene->clear(); 

        // Back light
        scene->add(Sphere(2.0, Vector3d(-5.0, 5.0, -5.0)), Material(Spectrum(64.0, 64.0, 64.0), Spectrum(1.0, 1.0, 1.0), REFLECTION_DIFFUSE), true);

        // Walls
        Vector3d v000(-10.0, -10.0, -10.0);
        Vector3d v100( 10.0, -10.0, -10.0);
        Vector3d v010(-10.0,  10.0, -10.0);
        Vector3d v001(-10.0, -10.0,  50.0);
        Vector3d v110( 10.0,  10.0, -10.0);
        Vector3d v101( 10.0, -10.0,  50.0);
        Vector3d v011(-10.0,  10.0,  50.0);
        Vector3d v111( 10.0,  10.0,  50.0);

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
        dragon.translate(Vector3d(0.0, 0.0, 0.0));
        dragon.putOnPlane(Plane(10.0, Vector3d(0.0, 1.0, 0.0)));
        dragon.buildAccel();

        scene->add(dragon, Material(Spectrum(), Spectrum(0.70, 0.60, 0.40), REFLECTION_SUBSURFACE));                         

        (*camera) = Camera(width, height, 
                           Vector3d(0.0, 0.0, 100.0),
                           Vector3d(0.0, 0.0, -1.0),
                           Vector3d(0.0, 1.0, 0.0),
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

        //Vector3d l00(-5.0, 9.99, -5.0);
        //Vector3d l01(-5.0, 9.99,  5.0);
        //Vector3d l10( 5.0, 9.99, -5.0);
        //Vector3d l11( 5.0, 9.99,  5.0);
        //scene->add(Quad(l00, l10, l11, l01), LambertianBRDF::factory(Spectrum(0.0, 0.0, 0.0)), Spectrum(32.0, 32.0, 32.0), true);

        // Back light
        scene->setAreaLight(Sphere(Point(5.0, 5.0, 5.0), 2.0),
                            Spectrum(64.0, 64.0, 64.0));

        // Walls
        Point3d v000(-10.0, -10.0, -10.0);
        Point3d v100( 10.0, -10.0, -10.0);
        Point3d v010(-10.0,  10.0, -10.0);
        Point3d v001(-10.0, -10.0,  50.0);
        Point3d v110( 10.0,  10.0, -10.0);
        Point3d v101( 10.0, -10.0,  50.0);
        Point3d v011(-10.0,  10.0,  50.0);
        Point3d v111( 10.0,  10.0,  50.0);

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
        // kitten.putOnPlane(Plane(10.0, Vector3d(0.0, 1.0, 0.0)));

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
                                  Vector3d(0.0, 0.0, -1.0),
                                  Vector3d(0.0, 1.0, 0.0),
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
        // kitten.putOnPlane(Plane(10.0, Vector3d(0.0, 1.0, 0.0)));

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
                Point3d p00(ii, -10.0, jj);
                Point3d p01(ii + tileSize, -10.0, jj);
                Point3d p10(ii, -10.0, jj + tileSize);
                Point3d p11(ii + tileSize, -10.0, jj + tileSize);
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

        Point3d eye(0.0, 10.0, 100.0);
        (*camera) = Camera::asDoF(width, height, eye, -Vector3d(eye).normalized(),
                                  Vector3d(0.0, 1.0, 0.0),
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
