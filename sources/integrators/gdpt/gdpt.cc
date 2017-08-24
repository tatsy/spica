#define SPICA_API_EXPORT
#include "gdpt.h"

#include "core/memory.h"
#include "core/parallel.h"
#include "core/camera.h"
#include "core/film.h"
#include "core/sampler.h"
#include "core/scene.h"
#include "core/bsdf.h"
#include "core/mis.h"
#include "core/interaction.h"

#include "gdptfilm.h"

namespace spica {

namespace {

enum class PathType : int {
    BasePath,
    Invertible,
    NotInvertible,
    NonSymmetric
};

enum class VertexType : int {
    Camera,
    Surface, 
    Medium,
    Light
};

struct TraceRecord {
    TraceRecord() {
    }

    explicit TraceRecord(PathType type_)
        : type{type_} {
    }

    double jacobian = 1.0;
    double pdf = 1.0;
    Spectrum f = Spectrum{0.0, 0.0, 0.0};
    PathType type = PathType::BasePath;
};

struct Vertex {
    static Vertex createCamera(const Point3d &pos) {
        Vertex v;
        v.intr = std::make_shared<Interaction>(pos);
        v.type = VertexType::Camera;
        return v;
    }

    static Vertex createSurface(const SurfaceInteraction &intr_, const Vector3d &wi) {
        Vertex v;
        v.intr = std::make_shared<SurfaceInteraction>(intr_);
        v.wh   = vect::normalize(intr_.wo() + wi);
        v.type = VertexType::Surface;
        return v;
    }

    static Vertex createLight(const SurfaceInteraction &intr_) {
        Vertex v;
        v.intr = std::make_shared<SurfaceInteraction>(intr_);
        v.type = VertexType::Light;
        return v;
    }

    static Vertex createLight(const Ray &ray) {
        Vertex v;
        v.intr = std::make_shared<Interaction>(ray.org(), Normal3d(), -ray.dir());
        v.type = VertexType::Light;
        return v;
    }

    Point3d pos() const {
        Assertion(intr != nullptr, "Interaction has no position. Maybe this is light endpoint of non area light.");
        return intr->pos();
    }

    Normal3d normal() const {
        Assertion(intr != nullptr, "Interaction has no position. Maybe this is light endpoint of non area light.");
        return intr->normal();
    }

    Spectrum Le(const Scene &scene, const Ray &ray) const {
        Assertion(isLight(), "Le is required with non-light vertex!");
        if (intr->isSurfaceInteraction()) {
            auto isect = std::static_pointer_cast<SurfaceInteraction>(intr);
            return isect->Le(-ray.dir());
        } else {
            Spectrum ret(0.0);
            for (const auto &light : scene.lights()) {
                ret += light->Le(ray);
            }
            return ret;
        }
    }

    bool isLight() const {
        return type == VertexType::Light;
    }

    bool isAreaLight() const {
        return type == VertexType::Light && intr->isSurfaceInteraction();
    }

    bool isIBL() const {
        return type == VertexType::Light && !intr->isSurfaceInteraction();
    }

    bool isDiffuse() const {
        if (type == VertexType::Surface) {
            auto isect = std::static_pointer_cast<SurfaceInteraction>(intr);
            return isect->bsdf()->hasType(BxDFType::Diffuse);
        }
        return false;
    }

    bool isGlossy() const {
        if (type == VertexType::Surface) {
            auto isect = std::static_pointer_cast<SurfaceInteraction>(intr);
            return isect->bsdf()->hasType(BxDFType::Glossy);
        }
        return false;
    }

    bool isSpecular() const {
        if (type == VertexType::Surface) {
            auto isect = std::static_pointer_cast<SurfaceInteraction>(intr);
            return isect->bsdf()->hasType(BxDFType::Specular);
        }
        return false;
    }

    std::shared_ptr<Interaction> intr;
    Vector3d wh;
    VertexType type;
};

TraceRecord shift(const Scene &scene, RenderParams &params, const Ray &r, Sampler &sampler, MemoryArena &arena,
                  const std::vector<Vertex> &baseVerts) {
    // Initialization
    Spectrum L(0.0);
    Spectrum beta(1.0);
    double totalPDF = 1.0;
    double jacobian = 1.0;
    Ray ray(r);

    // Process interactions
    bool specularReflection = false;
    int bounces;
    for (bounces = 1; bounces < baseVerts.size(); bounces++) {
        // Target and neighboring vertices
        const Vertex &current = baseVerts[bounces];

        // Check light endpoint
        if ((bounces == 1 || specularReflection) && current.isLight()) {
            Spectrum Le = current.Le(scene, ray);
            if (Le.isBlack()) {
                return TraceRecord(PathType::NotInvertible);
            }
            L += beta * Le;
            break;
        }

        // Intersection test
        SurfaceInteraction isect;
        if (!scene.intersect(ray, &isect)) {
            // If occlusion occurs, then return non symmetric
            return TraceRecord(PathType::NonSymmetric);
        }

        isect.setScatterFuncs(ray, arena);
        Assertion(isect.bsdf() != nullptr, "No BSDF is set to a material!");

        // Next event estimation
        if (isect.bsdf()->numComponents(BxDFType::All & (~BxDFType::Specular)) > 0) {
            Spectrum Ld = beta * uniformSampleOneLight(isect, scene, arena, sampler);
            L += Ld;            
        }

        // Next vertex
        if (bounces == baseVerts.size() - 1) break;
        const Vertex &prev = baseVerts[bounces - 1];
        const Vertex &next = baseVerts[bounces + 1];

        Vector3d woBase = vect::normalize(prev.pos() - current.pos());
        Vector3d wiBase = next.isIBL() ? -next.intr->wo() : vect::normalize(next.pos() - current.pos());
        Vector3d woSub = -ray.dir();

        if (current.isDiffuse()) {
            // Diffuse material
            if (!isect.bsdf()->hasType(BxDFType::Diffuse)) {
                return TraceRecord(PathType::NotInvertible);
            }

            // TODO: reflection only
            Vector3d wiSub;
            double J = 1.0;
            if (next.isDiffuse() || next.isAreaLight()) {
                // Reconnect
                wiSub = vect::normalize(next.pos() - isect.pos());

                // Eq.(13) of the original paper
                Normal3d nx = current.intr->normal();
                Normal3d ny = isect.normal();
                double cosThetaX = std::max(0.0, vect::dot(nx, wiBase));
                double cosThetaY = std::max(0.0, vect::dot(ny, wiSub));
                double distX = (next.pos() - current.pos()).squaredNorm();
                double distY = (next.pos() - isect.pos()).squaredNorm();
                J = (cosThetaY * distX) / (cosThetaX * distY + EPS);
            } else {
                // Copy half-vector
                if (!next.isLight()) {
                    Vector3d wh = current.wh;
                    wiSub = vect::reflect(-ray.dir(), wh);
                } else {
                    // IBL
                    wiSub = -next.intr->wo();
                }
                
                // Eq.(11) of the original paper
                const Vector3d whSub = vect::normalize(woSub + wiSub);
                const double dotX = std::max(0.0, vect::dot(woBase, current.wh));
                const double dotY = std::max(0.0, vect::dot(woSub, whSub));
                J = dotY / (dotX + EPS);
            }

            double pdf = isect.bsdf()->pdf(woSub, wiSub, BxDFType::Diffuse);
            if (pdf == 0.0) {
                return TraceRecord(PathType::NotInvertible);                
            }

            Spectrum f = isect.bsdf()->f(woSub, wiSub, BxDFType::Diffuse);
                
            jacobian *= J;
            beta *= f * vect::absDot(wiSub, isect.ns()) / pdf;
            totalPDF *= pdf;
            ray = Ray(current.pos(), wiSub);
            specularReflection = false;
        } else if (current.isGlossy()) {
            // Glossy material
            if (!isect.bsdf()->hasType(BxDFType::Glossy)) {
                return TraceRecord(PathType::NotInvertible);
            }

        } else if (current.isSpecular()) {
            // Specular material
            if (!isect.bsdf()->hasType(BxDFType::Specular)) {
                return TraceRecord(PathType::NotInvertible);
            }

            // Reflection or refraction
            // TODO: reflection only
            double pdf;
            Vector3d wiSub;
            Spectrum f = isect.bsdf()->sample(woSub, &wiSub, sampler.get2D(), &pdf);

            const Vector3d whSub = vect::normalize(woSub + wiSub);
            const double dotX = std::max(0.0, vect::dot(woBase, current.wh));
            const double dotY = std::max(0.0, vect::dot(woSub, whSub));
            jacobian *= dotY / (dotX + EPS);
            beta *= f * vect::absDot(wiSub, isect.ns()) / pdf;
            totalPDF *= pdf;
            ray = isect.spawnRay(wiSub);
            specularReflection = true;
        } else {
            FatalError("Non surface interaction found in the middle of the path!");
        }

        if (beta.isBlack() || totalPDF == 0.0) {
            break;
        }
    }

    // Create record
    TraceRecord record;
    record.f = L;
    record.pdf = totalPDF;
    record.jacobian = jacobian;
    record.type = PathType::Invertible;
    return record;    
}

TraceRecord pathTrace(const Scene &scene, RenderParams &params, const Ray &r,
                      Sampler &sampler, MemoryArena &arena, std::vector<Vertex> *vertices) {
    const auto addItem = [&](const Vertex &v) {
        if (vertices) {
            vertices->push_back(v);
        }
    };

    Ray ray(r);
    Spectrum L(0.0);
    Spectrum beta(1.0);
    double totalPDF = 1.0;
    bool specularBounce = false;
    if (vertices) {
        vertices->clear();
    }

    // Add camera vertex
    addItem(Vertex::createCamera(ray.org()));

    // Path tracing
    TraceRecord record;
    int bounces;
    for (bounces = 0; ; bounces++) {
        SurfaceInteraction isect;
        bool isIntersect = scene.intersect(ray, &isect);

        // Intersect with light, then stop tracing
        if (specularBounce || bounces == 0) {
            Spectrum Le(0.0);
            if (isIntersect) {
                // Area light
                Le = isect.Le(-ray.dir());
                if (!Le.isBlack()) {
                    addItem(Vertex::createLight(isect));
                }
            } else {
                // Not area light (e.g. envmap)
                for (const auto &light : scene.lights()) {
                    Le += light->Le(ray);
                }

                if (!Le.isBlack()) {
                    addItem(Vertex::createLight(ray));
                }
            }

            if (!Le.isBlack()) {
                L += beta * Le;
                record.f = L;
                record.pdf = totalPDF;
                record.jacobian = 1.0;
                break;
            }
        }

        if (!isIntersect || bounces >= params.getInt("maxDepth")) {
            break;
        }

        isect.setScatterFuncs(ray, arena);
        Assertion(isect.bsdf() != nullptr, "GDPT does not support volume data!");

        if (isect.bsdf()->numComponents(BxDFType::All & (~BxDFType::Specular)) > 0) {
            Spectrum Ld = beta * uniformSampleOneLight(isect, scene, arena, sampler);
            L += Ld;            
        }

        // Process BxDF
        Vector3d wo = -ray.dir();
        Vector3d wi;
        double pdf;
        BxDFType sampledType;
        Spectrum ref = isect.bsdf()->sample(wo, &wi, sampler.get2D(), &pdf,
                                            BxDFType::All, &sampledType);

        if (ref.isBlack() || pdf == 0.0) break;

        beta *= ref * vect::absDot(wi, isect.ns()) / pdf;
        totalPDF *= pdf;
        specularBounce = (sampledType & BxDFType::Specular) != BxDFType::None;
        ray = isect.spawnRay(wi);

        // Add vertex
        addItem(Vertex::createSurface(isect, wi));

        // Account for BSSRDF
        if (isect.bssrdf() && (sampledType & BxDFType::Transmission) != BxDFType::None) {
            FatalError("BSSRDF is not supported in GDPT!");
        }
    }

    record.f = L;
    record.pdf = totalPDF;
    record.jacobian = 1.0;
    return record;
}

}  // Anonymous namespace

GDPTIntegrator::GDPTIntegrator(const std::shared_ptr<Sampler> &sampler)
    : Integrator{}
    , sampler_{sampler} {
}

GDPTIntegrator::GDPTIntegrator(RenderParams &params)
    : GDPTIntegrator{std::static_pointer_cast<Sampler>(params.getObject("sampler", true))} {
}

void GDPTIntegrator::render(const std::shared_ptr<const Camera> &camera,
                            const Scene &scene,
                            RenderParams &params) {
    // Initialization
    Integrator::render(camera, scene, params);
    auto initSampler = sampler_->clone((unsigned int)time(0));
    
    const int width = camera_->film()->resolution().x();
    const int height = camera_->film()->resolution().y();
    GDPTFilm film(camera_->film());

    const int numThreads = numSystemThreads();
    auto samplers = std::vector<std::unique_ptr<Sampler>>(numThreads);
    auto arenas   = std::vector<MemoryArena>(numThreads);

    // Trace rays
    const int offsetX[] = { -1, 1, 0, 0 };
    const int offsetY[] = { 0, 0, -1, 1 };
    const int numPixels  = width * height;
    const int numSamples = params.getInt("sampleCount");
    
    auto shiftImages = std::make_unique<Image[]>(4);
    auto misImages = std::make_unique<Image[]>(4);
    for (int k = 0; k < 4; k++) {
        shiftImages[k] = Image(width, height);
        misImages[k] = Image(width, height);
    }

    for (int i = 0; i < numSamples; i++) {
        // Prepare samplers
        if (i % numThreads == 0) {
            for (int t = 0; t < numThreads; t++) {
                auto seed = static_cast<unsigned int>(time(0) + t);
                samplers[t] = sampler_->clone(seed);
            }
        }

        std::atomic<int> proc(0);
        parallel_for(0, numPixels, [&](int pid) {
            const int threadID = getThreadID();
            const int y = pid / width;
            const int x = pid % width;
            const Point2d randFilm = samplers[threadID]->get2D();
            const Point2d randLens = samplers[threadID]->get2D();
            const double filterWeight = film.evalFilter(randFilm);
            const Ray ray = camera_->spawnRay(Point2i(width - x - 1, y), randFilm, randLens);

            // Base path
            std::vector<Vertex> baseVerts;
            TraceRecord baseRecord = pathTrace(scene, params, ray, *samplers[threadID], arenas[threadID], &baseVerts);
            film.addPixel(x, y, baseRecord.f, filterWeight);

            // Offset path
            for (int k = 0; k < 4; k++) {
                const int nx = x + offsetX[k];
                const int ny = y + offsetY[k];
                if (nx < 0 || ny < 0 || nx >= width || ny >= height) {
                    continue;
                }

                // Shift mapping
                const Ray subRay = camera_->spawnRay(Point2i(width - nx - 1, ny), randFilm, randLens);
                TraceRecord subRecord(PathType::NotInvertible);
                if (!baseVerts.empty()) {
                    subRecord = shift(scene, params, subRay, *samplers[threadID], arenas[threadID], baseVerts);
                }

                // Compute contribution
                Spectrum G;
                if (subRecord.type == PathType::Invertible) {
                    // Compute gradient with MIS
                    // Eq.(10) of original paper
                    double misWeight = baseRecord.pdf / (baseRecord.pdf + subRecord.pdf * subRecord.jacobian);
                    G = misWeight * (baseRecord.f - subRecord.f * subRecord.jacobian);
                } else if (subRecord.type == PathType::NotInvertible) {
                    // Naively compute gradient with path tracing
                    subRecord = pathTrace(scene, params, subRay, *samplers[threadID], arenas[threadID], nullptr);
                    G = baseRecord.f - subRecord.f;                
                } else if (subRecord.type == PathType::NonSymmetric) {
                    // Subpath results in "zero contribution"
                    G = baseRecord.f;
                }

                // Forward or backward difference
                if (k == 0 || k == 2) {
                    film.addGradient(x, y, k,  G, filterWeight);
                } else {
                    film.addGradient(x, y, k, -G, filterWeight);
                }
        
                // Debug
                shiftImages[k].pixel(x, y) += subRecord.f;
                misImages[k].pixel(x, y) += Spectrum(subRecord.jacobian);
            }

            proc++;
            if (proc % 1000 == 0 || proc == numPixels) {
                printf("\r[ %d / %d ] %6.2f %% processed...", i + 1, numSamples, 100.0 * proc / numPixels);
                fflush(stdout);
            }
        });
        printf("\n");

        film.save(i + 1);

        for (int k = 0; k < 4; k++) {
            Image temp = misImages[k];
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    temp.pixel(x, y) /= (i + 1);   
                }
            }

            char filename[256];
            sprintf(filename, "jacobian_%03d.png", k);
            temp.save(filename);

            temp = shiftImages[k];
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    temp.pixel(x, y) /= (i + 1);   
                }
            }

            sprintf(filename, "shift_%03d.png", k);
            temp.save(filename);
        }

        for (int t = 0; t < numThreads; t++) {
            arenas[t].reset();
        }
    }
    printf("Finish!!\n");
}

}  // namespace spica