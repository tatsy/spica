#define SPICA_API_EXPORT
#include "engine.h"

#include <cstdarg>
#include <map>

#include "spectrum.h"
#include "rect.h"
#include "primitive.h"
#include "path.h"

#include "../image/film.h"
#include "../filter/spica_filter.h"
#include "../camera/spica_camera.h"
#include "../integrator/spica_integrator.h"
#include "../integrator/render_parameters.h"
#include "../material/spica_material.h"
#include "../texture/constant.h"
#include "../light/spica_light.h"

#include "../random/random.h"
#include "../random/halton.h"

#include "../accel/bbvh_accel.h"
#include "../scenes/scene.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace spica {

namespace {

Vector3d getVector3d(const std::string& attr) {
    double x, y, z;
    if (sscanf(attr.c_str(), "%lf, %lf, %lf", &x, &y, &z) != 3) {
        return Vector3d{};
    }
    return Vector3d{ x, y, z };
}

Spectrum getSpectrum(const std::string& attr) {
    const char* ptr = &attr[0];
    double nm;
    double value;
    std::vector<double> nms;
    std::vector<double> values;
    while (ptr) {
        sscanf(ptr, "%lf:%lf", &nm, &value);
        nms.push_back(nm);
        values.push_back(value);
        ptr = strstr(ptr, ",");
        if (ptr != NULL) {
            ptr++;
        } else {
            break;
        }
    }
    return Spectrum(nms, values);
}

}  // anonymous namespace

Engine::Engine()
    : option_{} {
}

Engine::~Engine() {
}

void Engine::init(const Option& option) {
    this->option_ = option;
}

void Engine::start(const std::string& filename) const {
    using namespace boost::property_tree;

    // Open XML
    ptree xml;
    read_xml(filename, xml);
    if (xml.empty()) {
        printErr("XML file not found: %s\n", filename.c_str());
        return;
    }

    if (xml.get_child("scene").empty()) {
        printErr("The XML does not have the tag \"scene\"!!\n");
        return;
    }

    printOut("Version: %s\n", 
        xml.get<std::string>("scene.<xmlattr>.version").c_str());

    // Parse sensor
    if (xml.get_child("scene.sensor").empty()) {
        printErr("XML file does not have \"sensor\" block!!\n");
        return;
    }

    std::shared_ptr<Camera> camera = nullptr;
    std::shared_ptr<Sampler> sampler = nullptr;
    parseCamera(xml, &camera, &sampler);
    
    // Parse integrator
    Assertion(sampler != nullptr, "Sampler not found!!");
    Assertion(camera  != nullptr, "Camera not found!!");
    std::unique_ptr<Integrator> integrator;
    if (xml.get_child("scene.integrator").empty()) {
        printErr("Integrator not found!!");
        return;
    }

    const std::string intgrName =
        xml.get<std::string>("scene.integrator.<xmlattr>.type");
    if (intgrName == "path") {
        integrator.reset(new PathIntegrator(camera, sampler));
    } else if (intgrName == "sppm") {
        integrator.reset(new SPPMIntegrator(camera, sampler));
    } else {
        printErr("Unknown integrator name: %s\n", intgrName);
        return;
    }

    // Parse materials
    std::map<std::string, std::shared_ptr<Material>> materials;
    for (const auto& child : xml.get_child("scene")) {
        if (child.first != "bsdf") continue;

        std::string type = child.second.get<std::string>("<xmlattr>.type");
        std::string id   = child.second.get<std::string>("<xmlattr>.id");
        std::shared_ptr<Material> mtrl = nullptr;
        if (type == "diffuse") {
            Spectrum ref = getSpectrum(child.second.get<std::string>("spectrum.<xmlattr>.value"));
            mtrl = std::make_shared<LambertianMaterial>(
                   std::make_shared<ConstantTexture<Spectrum>>(ref));
        }
        materials[id] = mtrl;
    }

    // Parse shapes
    std::string dir = path::getDirectory(filename);
    std::vector<std::shared_ptr<Light>> lights;
    std::vector<std::shared_ptr<Primitive>> primitives;
    for (const auto& child : xml.get_child("scene")) {
        if (child.first != "shape") continue;

        // Transform
        Transform toWorld;
        parseTransform(child.second, &toWorld);

        // Shape
        std::string type = child.second.get<std::string>("<xmlattr>.type");
        std::vector<std::shared_ptr<Shape>> shapes;
        if (type == "obj") {
            if (child.second.get<std::string>("string.<xmlattr>.name") == "filename") {
                const std::string relpath =
                    child.second.get<std::string>("string.<xmlattr>.value");
                std::string objfile = dir + relpath;

                OBJMeshIO loader;
                shapes = loader.load(objfile, toWorld);
            }
        }

        // Material
        std::shared_ptr<Material> mtrl = nullptr;
        if (child.second.find("ref") != child.second.not_found()) {
            std::string id = child.second.get<std::string>("ref.<xmlattr>.id");
            mtrl = materials[id];
        }

        // Emitter
        int lightID = -1;
        if (child.second.find("emitter") != child.second.not_found()) {
            lightID = static_cast<int>(lights.size());
            const std::string lightType =
                child.second.get<std::string>("emitter.<xmlattr>.type");

            for (const auto& shape : shapes) {
                if (lightType == "area") {
                    Spectrum Le = getSpectrum(child.second.get<std::string>("emitter.spectrum.<xmlattr>.value"));
                    lights.emplace_back(new AreaLight(shape, toWorld, Le));
                }
            }
        }

        // Set geometric primitives
        for (int i = 0; i < shapes.size(); i++) {
            std::shared_ptr<AreaLight> area = lightID >= 0 ? std::static_pointer_cast<AreaLight>(lights[lightID + i]) : nullptr;
            primitives.emplace_back(new GeometricPrimitive(shapes[i], mtrl, area));
        }
    }

    // Construct BVH and the scene
    auto bbvh = std::make_shared<BBVHAccel>(primitives);
    Scene scene(bbvh, lights);
        
    // Start rendering
    RenderParameters params;
    params.samplePerPixel(option_.nSamples);
    params.saveFilenameFormat(option_.outfile);
    omp_set_num_threads(option_.nThreads);
    integrator->render(scene, params);
}


void Engine::cleanup() {
}

void Engine::printOut(const char* format, ...) const {
    va_list args;
    if (option_.verbose) {
        va_start(args, format);
        fprintf(stdout, format, args);
        va_end(args);
    }
}

void Engine::printErr(const char* format, ...) const {
    va_list args;
    if (option_.verbose) {
        va_start(args, format);
        fprintf(stderr, format, args);
        va_end(args);
    }
}

bool Engine::parseTransform(const boost::property_tree::ptree& xml,
                            Transform* transform) const {
    *transform = Transform();
    for (const auto& props : xml) {
        if (props.first == "translate") {
            double x = props.second.get<double>("<xmlattr>.x");
            double y = props.second.get<double>("<xmlattr>.y");
            double z = props.second.get<double>("<xmlattr>.z");
            *transform *= Transform::translate(Vector3d(x, y, z));        
        }

        if (props.first == "lookAt") {
            Vector3d origin = getVector3d(
                props.second.get<std::string>("<xmlattr>.origin"));
            Vector3d target = getVector3d(
                props.second.get<std::string>("<xmlattr>.target"));

            Vector3d up = getVector3d(
                props.second.get<std::string>("<xmlattr>.up"));
            *transform *= Transform::lookAt(Point3d(origin), Point3d(target), up);
        }
    }
    return true;
}

bool Engine::parseSampler(const boost::property_tree::ptree& xml,
                          std::shared_ptr<Sampler>* sampler) const {
    const std::string type =
        xml.get<std::string>("scene.sensor.sampler.<xmlattr>.type");
    if (type == "ldsampler") {
        const int sampleCount =
            xml.get<int>("scene.sensor.sampler.integer.<xmlattr>.value");
        *sampler = std::make_shared<Halton>(sampleCount, true, (unsigned int)time(0));
    } else {
        printErr("Unknown sampler type \"%s\" is specified !!", type.c_str());
        return false;
    }
    return true;
}

bool Engine::parseCamera(const boost::property_tree::ptree& xml,
                         std::shared_ptr<Camera>* camera,
                         std::shared_ptr<Sampler>* sampler) const {
    const std::string type = 
        xml.get<std::string>("scene.sensor.<xmlattr>.type");

    if (type == "perspective") {
        printOut("Camera: perspective\n");

        // Parse sampler
        if (!parseSampler(xml, sampler)) {
            printErr("Sampler not found!!\n");
            return false;
        }

        Film* film = nullptr;
        if (!parseFilm(xml, &film)) {
            printErr("Failed to parse \"Film\" !!\n");
            return false;
        }

        Transform toWorld;
        double fov, nearClip, farClip, focusDistance, lensr = 1.0;
        for (const auto& props : xml.get_child("scene.sensor")) {
            if (props.first == "<xmlattr>") continue;

            // Parse transform
            if (props.first == "transform") {
                if (props.second.get<std::string>("<xmlattr>.name") == "toWorld") {
                    parseTransform(props.second, &toWorld);
                }
            }

            // Parse float properties
            if (props.first == "float") {
                std::string name =
                    props.second.get<std::string>("<xmlattr>.name");
                if (name == "fov") { 
                    fov = props.second.get<double>("<xmlattr>.value") / 180.0 * PI;
                    if (xml.get<std::string>("scene.sensor.string.<xmlattr>.name") == "fovAxis") {
                        const std::string val =
                            xml.get<std::string>("scene.sensor.string.<xmlattr>.value");
                        if (val == "smaller") {
                            fov *= 0.95;
                        } else if (val == "larger") {
                            fov *= 1.05;
                        }
                    }
                } else if (name == "nearClip") {
                    nearClip = props.second.get<double>("<xmlattr>.value");
                } else if (name == "farClip") {
                    farClip = props.second.get<double>("<xmlattr>.value");
                } else if (name == "focusDistance") {
                    focusDistance = props.second.get<double>("<xmlattr>.value");
                }
            }
        }           

        RectF screen = RectF(-1.0, -1.0, 2.0, 2.0);
        *camera = std::make_shared<PerspectiveCamera>(
            toWorld, screen, lensr, focusDistance, fov, film);
    } else if (type == "orthographic") {
        printf("Orthographic\n");
    } else {
        FatalError("Unknown sensor type \"%s\" is specified!!\n", type.c_str());
    }
}

bool Engine::parseFilm(const boost::property_tree::ptree& xml,
                       Film** film) const {

    int width = 0;
    int height = 0;
    std::unique_ptr<Filter> filter = nullptr;
    
    // Parse width and height
    for (const auto& props : xml.get_child("scene.sensor.film")) {
        if (props.first != "integer") continue;
        const std::string name =
            props.second.get<std::string>("<xmlattr>.name");
        if (name == "width") {
            width = props.second.get<int>("<xmlattr>.value");
        } else if (name == "height") {
            height = props.second.get<int>("<xmlattr>.value");
        }
    }

    // Parse rfilter
    const std::string filtType =
        xml.get<std::string>(
            "scene.sensor.film.rfilter.<xmlattr>.type");
    if (filtType == "tent") {
        filter = std::make_unique<TentFilter>(Vector2d(0.5, 0.5));
    } else {
        printErr("Unknown filter type \"%s\" is specified!!", filtType);
        return false;
    }

    *film = new Film(Point2i(width, height), std::move(filter),
                     option_.outfile);

    return true;
}

}  // namespace spica
