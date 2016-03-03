#define SPICA_API_EXPORT
#include "engine.h"

#include <map>

#include "spectrum.h"
#include "rect.h"
#include "primitive.h"

#include "../image/film.h"
#include "../filter/spica_filter.h"
#include "../camera/spica_camera.h"
#include "../integrator/spica_integrator.h"
#include "../integrator/render_parameters.h"
#include "../material/spica_material.h"
#include "../texture/constant.h"

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

std::unique_ptr<Film> getFilm(XMLElement* elem, const std::string& outfile) {
    const int width = elem->IntAttribute("width");
    const int height = elem->IntAttribute("height");
    const std::string type = elem->Attribute("type");
    
    std::unique_ptr<Filter> filter = nullptr;
    if (type == "tent") {
        filter = std::make_unique<TentFilter>(Vector2d(0.5, 0.5));
    } else {
        fprintf(stderr, "Error!!\n");
    }

    return std::move(std::make_unique<Film>(Point2i(width, height), filter, outfile));
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

    // Parse sampler
    std::shared_ptr<Sampler> sampler = nullptr;
    if (elem->FirstChildElement("sampler")) {
        std::string samplerType = elem->FirstChildElement("sampler")->Attribute("type");
        const int nSamples = elem->FirstChildElement("sampler")->FirstChildElement("integer")->IntAttribute("value");
        if (samplerType == "random") {
            sampler = std::make_shared<Random>(0);
        } else if (samplerType == "halton") {
            sampler = std::make_shared<Halton>(0, true, nSamples);
        }
    }

    // Parse camera
    std::shared_ptr<Camera> camera = nullptr;
    std::unique_ptr<Film>   film   = nullptr;
    if (elem->FirstChildElement("sensor")) {
        XMLElement* sensorElem = elem->FirstChildElement("sensor");
        std::string sensorType = sensorElem->Attribute("type");

        if (sensorType == "perspective") {
            film = getFilm(sensorElem->FirstChildElement("film"), option_.outfile);

            double fov = sensorElem->DoubleAttribute("fov");
            double near = sensorElem->DoubleAttribute("nearClip");
            double far  = sensorElem->DoubleAttribute("farClip");
            double focal = sensorElem->DoubleAttribute("focalDistance");
            double lensr = sensorElem->DoubleAttribute("lensRadius");
            Transform toWorld = getTransform(sensorElem->FirstChildElement("transform"));
            RectF screen = RectF(0.0, 0.0, film->resolution().x(), film->resolution().y());

            camera = std::make_shared<PerspectiveCamera>(toWorld, screen, lensr, focal, fov, film.get());
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
            Spectrum ref; // = getSpectrum(elem->FirstChildElement("spectrum"));
            mtrl = std::make_shared<LambertianMaterial>(
                std::make_shared<ConstantTexture<Spectrum>>(ref));
        }
        materials.insert(std::make_pair(id, mtrl));
    
        bsdfElem = bsdfElem->NextSiblingElement("bsdf");
    }

    // Parse shapes
    std::vector<std::shared_ptr<Light>> lights;
    std::vector<std::shared_ptr<Primitive>> primitives;
    XMLElement* shapeElem = elem->FirstChildElement("shape");
    while (bsdfElem) {

        bsdfElem = bsdfElem->NextSiblingElement("shape");
    }

    auto bbvh = std::make_shared<BBVHAccel>(primitives);
    Scene scene(bbvh, lights);
        
    // Start rendering
    RenderParameters params;
    // integrator->render(scene, params);
}

void Engine::cleanup() {
}

}  // namespace spica
