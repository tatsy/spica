#define SPICA_API_EXPORT
#include "scene_loader.h"

#include "yaml_parser.h"
#include "scene.h"
#include "../bsdf/bsdf.h"
#include "../bsdf/brdf.h"
#include "../camera/camera.h"

namespace spica {

    SceneLoader::SceneLoader() {
    }

    SceneLoader::~SceneLoader() {
    }

    void SceneLoader::load(const std::string& filename,
                           Scene* scene, Camera* camera) const {
        YamlElement elem;

        // Load yaml
        YamlParser parser(filename);
        YamlElement root = parser.documentElement().childByKey("scene");

        // Get image size
        elem = root.childByKey("screen");
        const int width = elem.valueByKey("width").asInteger();
        const int height = elem.valueByKey("height").asInteger();

        // Get camera parameters
        elem = root.childByKey("camera");
        std::vector<double> cent = elem.valueByKey("center").asDoubleList();
        std::vector<double> look = elem.valueByKey("lookat").asDoubleList();
        std::vector<double> up   = elem.valueByKey("up").asDoubleList();
        (*camera) = Camera::asDoF(width, height,
                                  Point(cent[0], cent[1], cent[2]),
                                  Vector3D(look[0], look[1], look[2]),
                                  Vector3D(up[0],   up[1],   up[2]),
                                  elem.valueByKey("sensor-size").asDouble(),
                                  elem.valueByKey("sensor-to-lens").asDouble(),
                                  elem.valueByKey("focal-length").asDouble(),
                                  elem.valueByKey("lens-radius").asDouble(),
                                  elem.valueByKey("sensitivity").asDouble());

        // Load meshes
        elem = root.childByKey("meshes").firstChild();
        while (!elem.isNull()) {
            // Read BSDF
            YamlElement bsdfElem = elem.childByKey("bsdf");
            std::string bsdfType = bsdfElem.valueByKey("type").asString();

            std::vector<double> ref = bsdfElem.valueByKey("refl").asDoubleList();
            Spectrum reflectance(ref[0], ref[1], ref[2]);
            std::vector<double> emt = bsdfElem.valueByKey("emit").asDoubleList();
            Spectrum emission(emt[0], emt[1], emt[2]);
            bool isLight = elem.valueByKey("light").asBool();

            BSDF bsdf;
            if (bsdfType == "lambertian") {
                bsdf = LambertianBRDF::factory(reflectance); 
            } else if (bsdfType == "specular") {
                bsdf = SpecularBRDF::factory(reflectance);
            } else if (bsdfType == "refractive") {
                bsdf = RefractiveBSDF::factory(reflectance);
            }

            // Read shape geometry
            // TODO: add shape operation is commented out 
            YamlElement shapeElem = elem.childByKey("shape");
            std::string shape = shapeElem.valueByKey("type").asString();
            if (shape == "quad") {
                YamlElement vertElem = shapeElem.childByKey("vertices").firstChild();
                Vector3D verts[4];
                for (int i = 0; i < 4; i++) {
                    std::vector<double> vert = vertElem.asDoubleList();
                    verts[i] = Vector3D(vert[0], vert[1], vert[2]);
                    vertElem = vertElem.nextSibling();
                }
                
                // scene->addShape(Quad(verts[0], verts[1], verts[2], verts[3]), bsdf, emission, isLight);
            } else if (shape == "sphere") {
                std::vector<double> cent = shapeElem.valueByKey("center").asDoubleList();
                double radius = shapeElem.valueByKey("radius").asDouble();
                //scene->addShape(Sphere(radius, Vector3D(cent[0], cent[1], cent[2])),
                 //                 bsdf, emission, isLight);
            }
            elem = elem.nextSibling();
        }
    }

}  // namespace spica
