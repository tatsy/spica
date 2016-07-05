#define SPICA_API_EXPORT
#include "engine.h"

#include <cstdio>
#include <cstdarg>
#include <map>

#include "spectrum.h"
#include "bounds2d.h"
#include "primitive.h"
#include "path.h"

#include "../image/film.h"
#include "../filter/spica_filter.h"
#include "../camera/spica_camera.h"
#include "../integrator/spica_integrator.h"
#include "../core/renderparams.h"
#include "../material/spica_material.h"
#include "../texture/constant.h"
#include "../light/spica_light.h"

#include "../random/random.h"
#include "../random/halton.h"

#include "../medium/spica_medium.h"

#include "../accelerator/spica_accelerator.h"
#include "../scenes/scene.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace spica {

Vector3d getVector3d(const std::string& attr) {
    double x, y, z;
    if (sscanf(attr.c_str(), "%lf, %lf, %lf", &x, &y, &z) == 3) {
        return Vector3d{ x, y, z };
    } else if (sscanf(attr.c_str(), "%lf %lf %lf", &x, &y, &z) == 3) {
        return Vector3d{ x, y, z };
    }
    return Vector3d{ };
}

Spectrum getSpectrum(const std::string& attr) {
    const char* ptr = &attr[0];
    double r, g, b;
    if (sscanf(ptr, "%lf, %lf, %lf", &r, &g, &b) == 3 ||
        sscanf(ptr, "%lf %lf %lf", &r, &g, &b) == 3) {
        return RGBSpectrum(r, g, b);
    }

    double nm;
    double value;
    std::vector<double> nms;
    std::vector<double> values;
    while (ptr) {
        if (sscanf(ptr, "%lf:%lf", &nm, &value) != 2) break;
        nms.push_back(nm);
        values.push_back(value);
        ptr = strstr(ptr, ",");
        if (ptr != NULL) {
            ptr++;
        } else {
            break;
        }
    }

    if (!nms.empty() && !values.empty()) {
        return Spectrum(nms, values);
    }

    if (sscanf(ptr, "%lf", &r) == 1) {
        return Spectrum(r);       
    }

    return Spectrum();
}

template <>
bool Engine::parse(const boost::property_tree::ptree& xml,
                   Transform* transform) const {
    *transform = Transform();
    for (const auto& props : xml) {
        if (props.first == "translate") {
            double x = props.second.get<double>("<xmlattr>.x");
            double y = props.second.get<double>("<xmlattr>.y");
            double z = props.second.get<double>("<xmlattr>.z");
            *transform = Transform::translate(Vector3d(x, y, z)) * (*transform);        
        }

        if (props.first == "scale") {
            double x = props.second.get<double>("<xmlattr>.x");
            double y = props.second.get<double>("<xmlattr>.y");
            double z = props.second.get<double>("<xmlattr>.z");
            *transform = Transform::scale(x, y, z) * (*transform);
        }

        if (props.first == "rotate") {
            Vector3d axis = getVector3d(props.second.get<std::string>("<xmlattr>.axis"));
            double angle = props.second.get<double>("<xmlattr>.angle");
            *transform = Transform::rotate(angle, axis) * (*transform);
        }

        if (props.first == "lookAt") {
            Vector3d origin = getVector3d(
                props.second.get<std::string>("<xmlattr>.origin"));
            Vector3d target = getVector3d(
                props.second.get<std::string>("<xmlattr>.target"));
            Vector3d up = getVector3d(
                props.second.get<std::string>("<xmlattr>.up"));
            *transform = Transform::lookAt(Point3d(origin), Point3d(target), up) * (*transform);
        }
    }
    return true;
}


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

    // Open XML file.
    ptree xml;
    try {
        read_xml(filename, xml);
    } catch (const xml_parser_error& err) {
        std::cerr << "[ERROR] " << err.message() << std::endl;
        return;
    }

    if (xml.empty()) {
        fprintf(stderr, "XML file not found: %s\n", filename.c_str());
        return;
    }

    // Find root tag "scene".
    if (xml.get_child("scene").empty()) {
        fprintf(stderr, "The XML does not have the tag \"scene\"!!\n");
        return;
    }
    
    // Version string.
    const std::string version = xml.get<std::string>("scene.<xmlattr>.version");
    fprintf(stdout, "Version: %s\n", version.c_str());

    // Parse sensor.
    auto sensorNode = xml.get_child("scene.sensor");
    if (sensorNode.empty()) {
        fprintf(stderr, "\"sensor\" block not found!!\n");
        return;
    }

    std::shared_ptr<Camera> camera  = nullptr;
    if (!parse(sensorNode, &camera)) {
        fprintf(stderr, "Failed to parse \"sensor\" block!!\n");
    }

    // Parse sampler.
    auto samplerNode = xml.get_child("scene.sensor.sampler");
    if (samplerNode.empty()) {
        fprintf(stderr, "\"sampler\" block not found!!\n");
        return;
    }

    std::shared_ptr<Sampler> sampler = nullptr;
    if (!parse(samplerNode, &sampler)) {
        fprintf(stderr, "Failed to parse \"sampler\" block!!\n");
        return;
    }
    
    // Parse integrator
    auto integNode = xml.get_child("scene.integrator");
    if (integNode.empty()) {
        fprintf(stderr, "\"integrator\" block not found!!\n");
        return;
    }

    std::unique_ptr<Integrator> integrator = nullptr;
    RenderParams params;
    if (!parse_integrator(integNode, &integrator, &params, camera, sampler)) {
        fprintf(stderr, "Failed to parse \"integrator\" block!!\n");
        return;
    }

    // Parse bsdf, subsurface, and medium.
    std::map<std::string, std::shared_ptr<Material>> materials;
    for (const auto& child : xml.get_child("scene")) {
        if (child.first != "bsdf" && child.first != "subsurface" &&
            child.first != "medium") continue;

        std::string id   = child.second.get<std::string>("<xmlattr>.id", "");
        if (id == "") continue;

        std::shared_ptr<Material> mtrl = nullptr;
        if (!parse(child.second, &mtrl)) {
            fprintf(stderr, "Failed to parse \"bsdf\" block!!\n");
            return;
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
        if (child.second.find("transform") != child.second.not_found()) {
            parse(child.second.get_child("transform"), &toWorld);
        }

        // Shape
        std::vector<std::shared_ptr<Shape>> shapes;
        parse_shape(child.second, &shapes, toWorld, dir);

        // Material
        std::shared_ptr<Material>        mtrl   = nullptr;
        std::shared_ptr<MediumInterface> medium = nullptr;
        std::shared_ptr<Texture<Spectrum>> sigma_a = nullptr;
        std::shared_ptr<Texture<Spectrum>> sigma_s = nullptr;
        double scaleSSS = 1.0;
        double gSSS     = 0.0;
        double etaSSS   = 1.0;

        // Parse references
        for (const auto& k : child.second) {
            if (k.first != "ref") continue;

            std::string id = k.second.get<std::string>("<xmlattr>.id", "");
            if (id == "") continue;

            std::string tagname;
            boost::property_tree::ptree result;
            if (!find_field(xml.get_child("scene"), "id", id, &result, &tagname)) {
                fprintf(stderr, "Invalid reference found: %s", id.c_str());
                continue;
            }

            if (tagname == "bsdf") {
                if (!parse(result, &mtrl)) {
                    fprintf(stderr, "Failed to parse material!!\n");
                    continue;
                }
            } else if (tagname == "subsurface") {
                if (!parse_subsurface(result, &sigma_a, &sigma_s, &etaSSS, &scaleSSS, &gSSS)) {
                    fprintf(stderr, "Failed to parse subsurface!!\n");
                    continue;
                }            
            } else if (tagname == "medium") {
                if (!parse(result, &medium)) {
                    fprintf(stderr, "Failed to parse medium!!\n");
                    continue;
                }
            }        
        }

        // Parse subsurface
        if (child.second.find("subsurface") != child.second.not_found()) {
            if (!parse_subsurface(child.second.get_child("subsurface"),
                &sigma_a, &sigma_s, &etaSSS, &scaleSSS, &gSSS)) {
                fprintf(stderr, "Failed to parse subsurface!!\n");
                continue;
            }
        }

        // Parse material
        if (child.second.find("bsdf") != child.second.not_found()) {
            if (sigma_a && sigma_s) {
                if (!parse_subsurface_mtrl(child.second.get_child("bsdf"), &mtrl,
                    sigma_a, sigma_s, etaSSS, scaleSSS, gSSS)) {
                    fprintf(stderr, "Failed to parse material!!\n");
                    continue;
                }
            } else {
                if (!parse(child.second.get_child("bsdf"), &mtrl)) {
                    fprintf(stderr, "Failed to parse material!!\n");
                    continue;
                }
            }
        }

        // Parse medium
        if (child.second.find("medium") != child.second.not_found()) {
            if (!parse(child.second.get_child("medium"), &medium)) {
                fprintf(stderr, "Failed to parse medium!!\n");
                continue;
            }
        }

        // Emitter
        int lightID = -1;
        if (child.second.find("emitter") != child.second.not_found()) {
            lightID = static_cast<int>(lights.size());
            const std::string lightType =
                child.second.get<std::string>("emitter.<xmlattr>.type");

            for (const auto& shape : shapes) {
                if (lightType == "area") {
                    Spectrum Le; 
                    if (child.second.get_child("emitter").find("rgb") != child.second.get_child("emitter").not_found()) {
                        Le = getSpectrum(child.second.get<std::string>("emitter.rgb.<xmlattr>.value"));
                    } else {
                        Le = getSpectrum(child.second.get<std::string>("emitter.spectrum.<xmlattr>.value"));
                    }
                    lights.emplace_back(new AreaLight(shape, toWorld, Le));
                }
            }
        }
        
        // Set geometric primitives
        for (int i = 0; i < shapes.size(); i++) {
            std::shared_ptr<AreaLight> area = lightID >= 0 ? std::static_pointer_cast<AreaLight>(lights[lightID + i]) : nullptr;
            primitives.emplace_back(new GeometricPrimitive(shapes[i], mtrl, area, medium));
        }
    }

    // Construct BVH and the scene
    auto bbvh = std::make_shared<BBVHAccel>(primitives);

    // Parse non-area emitters
    for (const auto& child : xml.get_child("scene")) {
        if (child.first != "emitter") continue;

        std::string type = child.second.get<std::string>("<xmlattr>.type", "");
        std::shared_ptr<Light> light = nullptr;
        if (type == "environment") {
            Bounds3d bbox = bbvh->worldBound();
            Point3d center = 0.5 * (bbox.posMin() + bbox.posMax());
            double  radius = (center - bbox.posMin()).norm();
            Sphere  sphere(center, radius);
            if (!parse_envmap(child.second, &light, sphere, dir)) {
                std::cerr << "Failed to parse Envmap!!" << std::endl;
                continue;
            }

            if (light) {
                lights.push_back(light);
            }
        }
    }

    // Finalize scene.
    Scene scene(bbvh, lights);
        
    // Start rendering
    params.set("NUM_SAMPLES", option_.nSamples);
    params.set("OUTPUT_FILE", option_.outfile);
    params.set("NUM_THREADS", option_.nThreads);
    integrator->render(scene, params);
}

void Engine::cleanup() {
}

bool Engine::find_field(const boost::property_tree::ptree& xml,
                        const std::string& field, const std::string& value,
                        boost::property_tree::ptree* result,
                        std::string* tag) const {
    for (const auto& node : xml) {
        if (node.first != "bsdf" && node.first != "subsurface" &&
            node.first != "medium") {
            continue;
        }

        std::string target = "<xmlattr>." + field;
        std::string v = node.second.get<std::string>(target, "");
        if (v == value) {
            *tag    = node.first;
            *result = node.second;
            return true;
        }
    }    
    return false;
}

template <class T>
bool Engine::parse(const boost::property_tree::ptree& xml, T* value) const {
    return false;
}

bool Engine::parse_film(const boost::property_tree::ptree& xml, Film** film) const {
    int width = 0;
    int height = 0;
    std::unique_ptr<Filter> filter = nullptr;
    
    // Parse width and height
    for (const auto& props : xml.get_child("")) {
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
    const std::string filtType = xml.get<std::string>("rfilter.<xmlattr>.type");
    if (filtType == "box") {
        filter = std::make_unique<BoxFilter>(Vector2d(0.5, 0.5));
    } else if (filtType == "tent") {
        filter = std::make_unique<TentFilter>(Vector2d(0.5, 0.5));
    } else if (filtType == "gaussian") {
        filter = std::make_unique<GaussianFilter>(Vector2d(0.5, 0.5), 2.0);
    } else {
        fprintf(stderr, "Unknown filter type \"%s\" is specified!!", filtType.c_str());
        return false;
    }

    *film = new Film(Point2i(width, height), std::move(filter),
                     option_.outfile);

    return true;
}

template <>
bool Engine::parse(const boost::property_tree::ptree& xml,
                   std::shared_ptr<Camera>* camera) const {
    const std::string type = xml.get<std::string>("<xmlattr>.type");
    if (type == "perspective") {
        fprintf(stdout, "Camera: perspective\n");

        Film* film = nullptr;
        auto filmNode = xml.get_child("film");
        if (filmNode.empty()) {
            fprintf(stderr, "\film\" block not found!!\n");
            return false;
        }

        if (!parse_film(filmNode, &film)) {
            fprintf(stderr, "Failed to parse \"Film\" !!\n");
            return false;
        }

        Transform toWorld;
        double fov, nearClip, farClip, focusDistance = 1.0, lensr = 0.0;
        for (const auto& props : xml.get_child("")) {
            if (props.first == "<xmlattr>") continue;

            // Parse transform
            if (props.first == "transform") {
                if (props.second.get<std::string>("<xmlattr>.name") == "toWorld") {
                    parse(props.second, &toWorld);
                }
            }

            // Parse float properties
            if (props.first == "float") {
                std::string name =
                    props.second.get<std::string>("<xmlattr>.name");
                if (name == "fov") { 
                    fov = props.second.get<double>("<xmlattr>.value") / 180.0 * PI;
                    if (xml.get<std::string>("string.<xmlattr>.name") == "fovAxis") {
                        const std::string val =
                            xml.get<std::string>("string.<xmlattr>.value");
                        if (val == "smaller") {
                            //fov *= 0.95;
                        } else if (val == "larger") {
                            //fov *= 1.05;
                        }
                    }
                } else if (name == "nearClip") {
                    nearClip = props.second.get<double>("<xmlattr>.value");
                } else if (name == "farClip") {
                    farClip = props.second.get<double>("<xmlattr>.value");
                } else if (name == "focusDistance") {
                    focusDistance = props.second.get<double>("<xmlattr>.value");
                } else if (name == "apertureRadius") {
                    lensr = props.second.get<double>("<xmlattr>.value");
                }
            }
        }           

        Bounds2d screen(-1.0, -1.0, 1.0, 1.0);
        *camera = std::make_shared<PerspectiveCamera>(
            toWorld, screen, lensr, focusDistance, fov, film);
    } else if (type == "orthographic") {
        printf("Orthographic\n");
    } else {
        FatalError("Unknown sensor type \"%s\" is specified!!\n", type.c_str());
    }
    return true;
}

bool Engine::parse_integrator(const boost::property_tree::ptree& xml,
                              std::unique_ptr<Integrator>* integrator,
                              RenderParams* params,
                              const std::shared_ptr<const Camera>& camera,
                              const std::shared_ptr<Sampler>& sampler) const {
    // Parse integrator.
    const std::string name =
        xml.get<std::string>("<xmlattr>.type");
    if (name == "path") {
        *integrator = std::make_unique<PathIntegrator>(camera, sampler);
    } else if (name == "volpath") {
        *integrator = std::make_unique<VolPathIntegrator>(camera, sampler);
    } else if (name == "direct") {
        *integrator = std::make_unique<DirectLightingIntegrator>(camera, sampler);
    } else if (name == "sppm") {
        *integrator = std::make_unique<SPPMIntegrator>(camera, sampler);
    } else if (name == "ppm") {
        *integrator = std::make_unique<PPMProbIntegrator>(camera, sampler);
    } else if (name == "irrcache") {
        *integrator = std::make_unique<IrradCacheIntegrator>(camera, sampler);
    } else if (name == "pssmlt") {
        *integrator = std::make_unique<PSSMLTIntegrator>(camera);
    } else {
        fprintf(stderr, "Unsupported integrator type: %s\n", name.c_str());
        return false;
    }
    printf("Integrator: %s\n", name.c_str());

    // Parse options.
    for (const auto& child : xml) {
        if (child.first == "integer") {
            std::string name = child.second.get<std::string>("<xmlattr>.name", "");
            int n = child.second.get<int>("<xmlattr>.value");
            if (name == "maxDepth") {
                params->set("MAX_BOUNCES", n);                
            } else if (name == "globalPhotons") {
                params->set("CAST_PHOTONS", n);
            } else if (name == "lookupSize") {
                params->set("GATHER_PHOTONS", n);
            }
        } else if (child.first == "float") {
            std::string name = child.second.get<std::string>("<xmlattr>.name", "");
            double f = child.second.get<double>("<xmlattr>.value");
            if (name == "globalLookupRadius") {
                params->set("GATHER_RADIUS", f);
            }
        }
    }
    
    return true;
}

bool Engine::parse_envmap(const boost::property_tree::ptree& xml,
                          std::shared_ptr<Light>* light,
                          const Sphere& worldSphere,
                          const std::string& directory) const {
    const std::string type = xml.get<std::string>("<xmlattr>.type");
    if (type == "environment") {
        Image envmap;
        Transform toWorld;
        double gamma = 1.0;
        double scale = 1.0;
        int numSamples = 1;

        for (const auto& child : xml) {
            if (child.first == "string") {
                std::string name = child.second.get<std::string>("<xmlattr>.name", "");
                if (name == "filename") {
                    std::string filename = child.second.get<std::string>("<xmlattr>.value", "");
                    envmap = Image::fromFile(directory + filename);
                }
            } else if (child.first == "float") {
                std::string name = child.second.get<std::string>("<xmlattr>.name", "");
                if (name == "gamma") {
                    gamma = child.second.get<double>("<xmlattr>.value", 1.0);                    
                } else if (name == "scale") {
                    scale = child.second.get<double>("<xmlattr>.value", 1.0);
                }
            } else if (child.first == "transform") {
                std::string name = child.second.get<std::string>("<xmlattr>.name", "");
                if (name == "toWorld") {
                    if (!parse(child.second, &toWorld)) {
                        std::cerr << "Failed to parse transform!!" << std::endl;
                        return false;
                    }
                }
            }
        }

        *light = std::make_shared<Envmap>(worldSphere, envmap, toWorld, Spectrum(scale), numSamples);
    } else {
        std::cerr << "This node is not for environment light!!" << std::endl;
        return false;
    }

    return true;
}

template <>
bool Engine::parse(const boost::property_tree::ptree& xml,
                   std::shared_ptr<Sampler>* sampler) const {

    const std::string type = xml.get<std::string>("<xmlattr>.type");
    if (type == "independent") {
        *sampler = std::make_shared<Random>((unsigned int)time(0));
    } else if (type == "ldsampler") {
        const int sampleCount = xml.get<int>("integer.<xmlattr>.value");
        *sampler = std::make_shared<Halton>(sampleCount, true, (unsigned int)time(0));
    } else {
        fprintf(stderr, "Unknown sampler type \"%s\" is specified !!", type.c_str());
        return false;
    }
    return true;
}

template <>
bool Engine::parse(const boost::property_tree::ptree& xml,
                   std::shared_ptr<Material>* material) const {
    const std::string type = xml.get<std::string>("<xmlattr>.type");
    if (type == "diffuse") {
        // Diffuse
        Spectrum ref(0.0);
        for (const auto& k : xml) {
            if (k.first == "rgb" || k.first == "spectrum") {
                ref = getSpectrum(k.second.get<std::string>("<xmlattr>.value"));
            }
        }
        auto Kd = std::make_shared<ConstantTexture<Spectrum>>(ref);
        *material = std::make_shared<LambertianMaterial>(Kd);
    } else if (type == "dielectric") {
        Spectrum re  = Spectrum(0.999);
        Spectrum tr  = Spectrum(0.999);
        double alpha = 0.0;
        double eta   = 1.0;
        for (const auto& k : xml) {
            if (k.first == "rgb" || k.first == "spectrum") {
                std::string refname = k.second.get<std::string>("<xmlattr>.name", "");
                if (refname == "specularReflectance") {
                    re = getSpectrum(k.second.get<std::string>("<xmlattr>.value", ""));
                } else if (refname == "specularTransmittance") {
                    tr = getSpectrum(k.second.get<std::string>("<xmlattr>.value", ""));
                } else {
                    fprintf(stderr, "Unknown spectrum name: %s\n", refname.c_str());
                    return false;
                }
            } else if (k.first == "float") {
                std::string propname = k.second.get<std::string>("<xmlattr>.name", "");
                if (propname == "alpha") {
                    alpha = k.second.get<double>("<xmlattr>.value", 0.0);
                } else if (propname == "intIOR") {
                    eta = k.second.get<double>("<xmlattr>.value", 1.0);   
                }
            }
        }

        auto Kr = std::make_shared<ConstantTexture<Spectrum>>(re);
        auto Kt = std::make_shared<ConstantTexture<Spectrum>>(tr);
        auto rough = std::make_shared<ConstantTexture<double>>(alpha);
        auto index = std::make_shared<ConstantTexture<double>>(eta);
        *material = std::make_shared<GlassMaterial>(Kr, Kt, rough, rough, index);
    } else if (type == "roughplastic") {
        // Rough plastic
        Spectrum diffuse(0.9999);
        Spectrum specular(0.9999);                
        double ior = 1.0;
        double alpha = 1.0;
        for (const auto& k : xml.get_child("")) {
            if (k.first == "rgb" || k.first == "spectrum") {
                std::string refName = k.second.get<std::string>("<xmlattr>.name");
                if (refName == "diffuseReflectance") {
                    diffuse = getSpectrum(k.second.get<std::string>("<xmlattr>.value"));
                } else if (refName == "specularReflectance") {
                    specular = getSpectrum(k.second.get<std::string>("<xmlattr>.value"));                    
                }
            } else if (k.first == "float") {
                std::string propName = k.second.get<std::string>("<xmlattr>.name");
                if (propName == "alpha") {
                    alpha = k.second.get<double>("<xmlattr>.value");
                } else if (propName == "intIOR") {
                    ior = k.second.get<double>("<xmlattr>.value");
                }
            }
        }

        auto Kd = std::make_shared<ConstantTexture<Spectrum>>(diffuse);
        auto Ks = std::make_shared<ConstantTexture<Spectrum>>(specular);
        auto rough = std::make_shared<ConstantTexture<double>>(alpha);
        *material = std::make_shared<PlasticMaterial>(Kd, Ks, rough);
    } else if (type == "roughconductor") {
        // Metal
        Spectrum eta(1.0);
        Spectrum specular(0.999);
        double alpha = 0.0;
        for (const auto& k : xml.get_child("")) {
            if (k.first == "rgb" || k.first == "spectrum") {
                std::string refName = k.second.get<std::string>("<xmlattr>.name");
                if (refName == "specularReflectance") {
                    specular = getSpectrum(k.second.get<std::string>("<xmlattr>.value"));
                } else if (refName == "eta") {
                    eta = getSpectrum(k.second.get<std::string>("<xmlattr>.value"));
                }
            }

            if (k.first == "float" && k.second.get<std::string>("<xmlattr>.name") == "alpha") {
                alpha = k.second.get<double>("<xmlattr>.value");
            }
        }

        auto etaT  = std::make_shared<ConstantTexture<Spectrum>>(eta);
        auto Ks    = std::make_shared<ConstantTexture<Spectrum>>(specular);
        auto rough = std::make_shared<ConstantTexture<double>>(alpha);
        *material = std::make_shared<MetalMaterial>(etaT, Ks, rough);
    }

    return true;
}

template <>
bool Engine::parse(const boost::property_tree::ptree& xml,
                  std::shared_ptr<MediumInterface>* medium) const {
    Medium* med = nullptr;

    const std::string type = xml.get<std::string>("<xmlattr>.type", "");
    if (type == "homogeneous") {
        Spectrum sigma_a(0.0);
        Spectrum sigma_s(0.0);
        double scale = 1.0;
        double g = 0.0;

        for (const auto& k : xml) {
            if (k.first == "rgb" || k.first == "spectrum") {
                std::string name = k.second.get<std::string>("<xmlattr>.name", "");
                if (name == "sigmaA") {
                    sigma_a = getSpectrum(k.second.get<std::string>("<xmlattr>.value", ""));
                } else if (name == "sigmaS") {
                    sigma_s = getSpectrum(k.second.get<std::string>("<xmlattr>.value", ""));                
                } else {
                    fprintf(stderr, "Unknown spectrum name: %s\n", name.c_str());
                    return false;
                }
            } else if (k.first == "float") {
                std::string name = k.second.get<std::string>("<xmlattr>.name", "");
                if (name == "scale") {
                    scale = k.second.get<double>("<xmlattr>.value", 1.0);
                } else {
                    fprintf(stderr, "Unknown float name: %s\n", name.c_str());
                    return false;
                }
            } else if (k.first == "phase") {
                std::string type = k.second.get<std::string>("<xmlattr>.type", "");
                if (type == "hg") {
                    for (const auto& l : k.second) {
                        if (l.first == "float") {
                            std::string name = l.second.get<std::string>("<xmlattr>.name", "");
                            if (name == "g") {
                                g = l.second.get<double>("<xmlattr>.value", 0.0);
                            } else {
                                fprintf(stderr, "Unknown float type: %s\n", name.c_str());
                                return false;
                            }
                        }
                    }
                } else {
                    fprintf(stderr, "Unsupported phase function type: %s\n", type.c_str());
                    return false;
                }
            }
        }

        med = new HomogeneousMedium(sigma_a * scale, sigma_s * scale, g);
    } else {
        fprintf(stderr, "Unsupported medium type: %s\n", type.c_str());
        return false;
    }

    if (!med) {
        fprintf(stderr, "Unexpected error while parsing medium!!\n");
        return false;
    }

    *medium = std::make_shared<MediumInterface>(med, nullptr);
    return true;
}

bool Engine::parse_subsurface_mtrl(const boost::property_tree::ptree& xml,
                                   std::shared_ptr<Material>* mtrl,
                                   const std::shared_ptr<Texture<Spectrum>>& sigma_a,
                                   const std::shared_ptr<Texture<Spectrum>>& sigma_s,
                                   double eta, double scale, double g) const {
    const std::string type = xml.get<std::string>("<xmlattr>.type");
    if (type == "roughplastic") {
        // Rough plastic
        Spectrum diffuse  = Spectrum(0.9999);
        double ior = 1.0;
        double alpha = 0.0;
        for (const auto& k : xml) {
            if (k.first == "rgb" || k.first == "spectrum") {
                std::string refName = k.second.get<std::string>("<xmlattr>.name");
                if (refName == "diffuseReflectance") {
                    diffuse = getSpectrum(k.second.get<std::string>("<xmlattr>.value"));
                }
            } else if (k.first == "float") {
                std::string propName = k.second.get<std::string>("<xmlattr>.name");
                if (propName == "alpha") {
                    alpha = k.second.get<double>("<xmlattr>.value");
                } else if (propName == "intIOR") {
                    ior   = k.second.get<double>("<xmlattr>.value");
                }
            }
        }

        auto Kr = std::make_shared<ConstantTexture<Spectrum>>(diffuse);
        auto Kt = std::make_shared<ConstantTexture<Spectrum>>(Spectrum(0.999));
        auto rough = std::make_shared<ConstantTexture<double>>(alpha);
        *mtrl = std::make_shared<SubsurfaceMaterial>(scale, Kr, Kt, sigma_a, sigma_s, g, eta, rough);
    } else {
        fprintf(stderr, "Material \"%s\" does not support subsurface scattering!!\n", type.c_str());
        return false;
    }

    return true;
}

bool Engine::parse_shape(const boost::property_tree::ptree& xml,
                         std::vector<std::shared_ptr<Shape>>* shapes,
                         const Transform& toWorld,
                         const std::string& directory) const {
    const std::string type = xml.get<std::string>("<xmlattr>.type");
    if (type == "obj") {
        if (xml.get<std::string>("string.<xmlattr>.name") == "filename") {
            const std::string relpath =
                xml.get<std::string>("string.<xmlattr>.value");
            std::string objfile = directory + relpath;
            fprintf(stdout, "Loading \"%s\"\n", objfile.c_str());

            MeshIO loader;
            auto groups = loader.load(objfile, toWorld);
            for (const auto& g : groups) {
                shapes->insert(shapes->end(), g.shapes().begin(), g.shapes().end());
            }
        }
    } else if (type == "sphere") {
        Point3d center;
        if (xml.get<std::string>("point.<xmlattr>.name") == "center") {
            double x = xml.get<double>("point.<xmlattr>.x");
            double y = xml.get<double>("point.<xmlattr>.y");
            double z = xml.get<double>("point.<xmlattr>.z");
            center = Point3d(x, y, z);
        }
        double r = xml.get<double>("float.<xmlattr>.value");
        shapes->push_back(std::make_shared<Sphere>(center, r));
    } else {
        fprintf(stderr, "Uknown shape type: %s\n", type.c_str());
        return false;
    }

    return true;
}

bool Engine::parse_subsurface(const boost::property_tree::ptree& xml,
                              std::shared_ptr<Texture<Spectrum>>* sigma_a,
                              std::shared_ptr<Texture<Spectrum>>* sigma_s,
                              double* eta, double* scale, double* g) const {
    // Set default values.
    *scale = 1.0;
    *g     = 0.0;
    for (const auto& child : xml) {
        if (child.first == "string") {
            if (child.second.get<std::string>("<xmlattr>.name", "") == "material") {
                std::string mtrlKind = child.second.get<std::string>("<xmlattr>.value", "");
                if (mtrlKind == "skimmilk") {
                    *sigma_a = std::make_shared<ConstantTexture<Spectrum>>(Spectrum(0.0014, 0.0025, 0.0142));
                    *sigma_s = std::make_shared<ConstantTexture<Spectrum>>(Spectrum(0.70, 1.22, 1.90));
                    *eta     = 1.3;
                }
            }
        }

        if (child.first == "float") {
            std::string fKind = child.second.get<std::string>("<xmlattr>.name", "");
            if (fKind == "scale") {
                *scale = child.second.get<double>("<xmlattr>.value", 1.0);
            } else if (fKind == "intIOR") {
                *eta   = child.second.get<double>("<xmlattr>.value", 1.0);
            }
        }
    }
    return true;
}

}  // namespace spica
