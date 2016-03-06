#define SPICA_API_EXPORT
#include "engine.h"

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

#include "tinyxml2.h"
using namespace tinyxml2;

namespace spica {

namespace {

Vector3d getVector3d(const char* attr) {
    double x, y, z;
    if (sscanf(attr, "%lf, %lf, %lf", &x, &y, &z) != 3) {
        return Vector3d{};
    }
    return Vector3d{ x, y, z };
}

Spectrum getSpectrum(const char* attr) {
    const char* ptr = attr;
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
    auto s = Spectrum(nms, values);
    std::cout << s << std::endl;
    return s;
}

Transform getTransform(XMLElement* elem) {
    if (strcmp(elem->Name(), "transform") != 0) {
        return Transform{};
    }

    Transform ret;

    XMLElement* child = elem->FirstChildElement();
    if (strcmp(child->Name(), "translate") == 0) {
        double x = atof(child->Attribute("x"));
        double y = atof(child->Attribute("y"));
        double z = atof(child->Attribute("z"));
        ret *= Transform::translate(Vector3d(x, y, z));        
    }

    if (strcmp(child->Name(), "lookAt") == 0) {
        Point3d origin = Point3d(getVector3d(child->Attribute("origin")));
        Point3d target = Point3d(getVector3d(child->Attribute("target")));
        Vector3d up    = getVector3d(child->Attribute("up"));
        ret *= Transform::lookAt(origin, target, up);
    }

    return ret;
}

Film* getFilm(XMLElement* elem, const std::string& outfile) {
    XMLElement* child = elem->FirstChildElement();
    int width = 0;
    int height = 0;
    std::unique_ptr<Filter> filter = nullptr;

    while (child) {
        if (strcmp(child->Name(), "integer") == 0) {
            if (strcmp(child->Attribute("name"), "width") == 0) {
                width = child->IntAttribute("value");
            } else if (strcmp(child->Attribute("name"), "height") == 0) {
                height = child->IntAttribute("value");
            }
        } else if (strcmp(child->Name(), "rfilter") == 0) {
            const char* filterType = child->Attribute("type");
            if (strcmp(filterType, "gaussian") == 0) {

            } else if (strcmp(filterType, "tent") == 0) {
                filter = std::make_unique<TentFilter>(Vector2d(0.5, 0.5));
            } else {
                FatalError("Unknown filter type \"%s\" is specified!!", filterType);
            }
        }

        child = child->NextSiblingElement();
    }    
    return new Film(Point2i(width, height), std::move(filter), outfile);
}

}  // anonymous namespace

Engine::Engine()
    : option_{}
    , integrator_{} {
}

Engine::~Engine() {
}

void Engine::init(const Option& option) {
    this->option_ = option;
}

void Engine::start(const std::string& filename) const {
    using namespace tinyxml2;

    XMLDocument doc;
    if (doc.LoadFile(filename.c_str()) != tinyxml2::XMLError::XML_SUCCESS) {
        fprintf(stderr, "XML file is invalid!!\n");
        return;
    }

    XMLElement* elem = doc.FirstChildElement("scene");
    if (!elem) {
        fprintf(stderr, "XML file does not have \"scene\" child!!\n");
        return;
    }

    // Parse camera
    std::shared_ptr<Camera> camera = nullptr;
    std::shared_ptr<Sampler> sampler = nullptr;
    if (elem->FirstChildElement("sensor")) {
        XMLElement* sensorElem = elem->FirstChildElement("sensor");
        std::string sensorType = sensorElem->Attribute("type");

        // Parse sampler
        if (sensorElem->FirstChildElement("sampler")) {
            std::string samplerType = sensorElem->FirstChildElement("sampler")->Attribute("type");
            const int nSamples = sensorElem->FirstChildElement("sampler")->FirstChildElement("integer")->IntAttribute("value");
            if (samplerType == "random") {
                sampler = std::make_shared<Random>(0);
            } else if (samplerType == "ldsampler") {
                sampler = std::make_shared<Halton>(0, true, nSamples);
            }
        }

        if (sensorType == "perspective") {
            Film* film = getFilm(sensorElem->FirstChildElement("film"), option_.outfile);

            XMLElement* child = sensorElem->FirstChildElement("float");
            double fov, near, far, focal, lensr = 1.0;
            while (child) {
                if (strcmp(child->Attribute("name"), "fov") == 0) 
                    fov = child->DoubleAttribute("value") / 180.0 * PI;
                if (strcmp(child->Attribute("name"), "nearClip") == 0)
                    near = child->DoubleAttribute("value");
                if (strcmp(child->Attribute("name"), "farClip") == 0)
                    far = child->DoubleAttribute("value");
                if (strcmp(child->Attribute("name"), "focusDistance") == 0)
                    focal = child->DoubleAttribute("value");

                child = child->NextSiblingElement("float");
            }
            

            Transform toWorld = getTransform(sensorElem->FirstChildElement("transform"));
            RectF screen = RectF(-1.0, -1.0, 2.0, 2.0);
            camera = std::make_shared<PerspectiveCamera>(toWorld, screen, lensr, focal, fov, film);
        } else if (sensorType == "orthographic") {
            FatalError("Not implemented yet!!");
        } else {
            fprintf(stderr, "Unknown sensor type \"%s\" is specified!!", sensorType.c_str());
            return;
        }
    } else {
        fprintf(stderr, "Sensor not found!!");
    }

    // Parse integrator
    Assertion(sampler != nullptr, "Sampler not found!!");
    Assertion(camera  != nullptr, "Camera not found!!");
    std::unique_ptr<Integrator> integrator;
    if (elem->FirstChildElement("integrator")) {
        const std::string integratorName =
            elem->FirstChildElement("integrator")->Attribute("type");
        if (integratorName == "path") {
            integrator.reset(new PathIntegrator(camera, sampler));
        }
    } else {
        fprintf(stderr, "Integrator not found!!");
    }

    // Parse materials
    std::map<std::string, std::shared_ptr<Material>> materials;
    XMLElement* bsdfElem = elem->FirstChildElement("bsdf");
    while (bsdfElem) {
        std::string type = bsdfElem->Attribute("type");
        std::string id   = bsdfElem->Attribute("id");
        std::shared_ptr<Material> mtrl = nullptr;
        if (type == "diffuse") {
            Spectrum ref = getSpectrum(bsdfElem->FirstChildElement("spectrum")->Attribute("value"));
            mtrl = std::make_shared<LambertianMaterial>(
                   std::make_shared<ConstantTexture<Spectrum>>(ref));
        }
        materials.insert(std::make_pair(id, mtrl));
    
        bsdfElem = bsdfElem->NextSiblingElement("bsdf");
    }

    // Parse shapes
    std::string dir = path::getDirectory(filename);
    std::vector<std::shared_ptr<Light>> lights;
    std::vector<std::shared_ptr<Primitive>> primitives;
    XMLElement* shapeElem = elem->FirstChildElement("shape");
    while (shapeElem) {
        // Transform
        Transform toWorld;
        if (shapeElem->FirstChildElement("transform")) {
            toWorld = getTransform(shapeElem->FirstChildElement("transform"));
        }

        std::vector<std::shared_ptr<Shape>> shapes;
        if (strcmp(shapeElem->Attribute("type"), "obj") == 0) {
            if (shapeElem->FirstChildElement("string") &&
                strcmp(shapeElem->FirstChildElement("string")->Attribute("name"), "filename") == 0) {
                std::string objfile = dir + shapeElem->FirstChildElement("string")->Attribute("value");

                OBJMeshIO loader;
                shapes = loader.load(objfile, toWorld);
            }
        }

        // Reflectance
        std::shared_ptr<Material> mtrl = nullptr;
        if (shapeElem->FirstChildElement("ref")) {
            const char* id = shapeElem->FirstChildElement("ref")->Attribute("id");
            mtrl = materials[id];
        }

        // Emitter
        int lightID = -1;
        if (shapeElem->FirstChildElement("emitter")) {
            lightID = static_cast<int>(lights.size());
            const char* lightType = shapeElem->FirstChildElement("emitter")->Attribute("type");
            Spectrum Le = getSpectrum(shapeElem->FirstChildElement("emitter")->FirstChildElement("spectrum")->Attribute("value"));

            for (const auto& shape : shapes) {
                if (strcmp(lightType, "area") == 0) {
                    lights.emplace_back(new AreaLight(shape, toWorld, Le));
                }
            }
        }

        for (int i = 0; i < shapes.size(); i++) {
            std::shared_ptr<AreaLight> area = lightID >= 0 ? std::static_pointer_cast<AreaLight>(lights[lightID + i]) : nullptr;
            primitives.emplace_back(new GeometricPrimitive(shapes[i], mtrl, area));
        }
        shapeElem = shapeElem->NextSiblingElement("shape");
    }

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

}  // namespace spica
