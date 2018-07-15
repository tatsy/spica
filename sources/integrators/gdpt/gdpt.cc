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
#include "core/visibility_tester.h"

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

struct SurfaceEventRecord {
    SurfaceEventRecord() {}
    SurfaceEventRecord(const Vector3d &wh_, const Vector3d &whLocal_, BxDFType bxdfType_,
                       double eta_, int sampledLightIndex_ = -1,
                       const Point2d &randLight_ = Point2d(),
                       const Point2d &randShade_ = Point2d())
        : wh{wh_}
        , whLocal{whLocal_}
        , bxdfType{bxdfType_}
        , eta{eta_}
        , sampledLightIndex{sampledLightIndex_}
        , randLight{randLight_}
        , randShade{randShade_} {
    }

    Vector3d wh;
    Vector3d whLocal;
    BxDFType bxdfType;
    double eta;
    int sampledLightIndex;
    Point2d randLight;
    Point2d randShade;
};

struct Vertex {
    static Vertex createCamera(const Point3d &pos) {
        Vertex v;
        v.intr = std::make_shared<Interaction>(pos);
        v.type = VertexType::Camera;
        return v;
    }

    static Vertex createSurface(const SurfaceInteraction &intr_,
                                const SurfaceEventRecord &record_) {
        Vertex v;
        v.intr = std::make_shared<SurfaceInteraction>(intr_);
        v.surfaceRecord = record_;
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
        return type == VertexType::Surface && 
               (surfaceRecord.bxdfType & BxDFType::Diffuse) != BxDFType::None;
    }

    bool isGlossy() const {
        return type == VertexType::Surface && 
               (surfaceRecord.bxdfType & BxDFType::Glossy) != BxDFType::None;
    }

    bool isSpecular() const {
        return type == VertexType::Surface && 
               (surfaceRecord.bxdfType & BxDFType::Specular) != BxDFType::None;
    }

    bool isReflection() const {
        return type == VertexType::Surface && 
               (surfaceRecord.bxdfType & BxDFType::Reflection) != BxDFType::None;
    }

    bool isTransmission() const {
        return type == VertexType::Surface && 
               (surfaceRecord.bxdfType & BxDFType::Transmission) != BxDFType::None;
    }

    std::shared_ptr<Interaction> intr;
    VertexType type;
    SurfaceEventRecord surfaceRecord;
};

bool nextDirection(const SurfaceInteraction &isect, const Vertex &prev, const Vertex &current, const Vertex &next,
                   Vector3d *wiOffset, double *pdf, Spectrum *f, double *J, bool *reconnect, bool *specularBounce) {
    const Vector3d woOffset = isect.wo();
    *reconnect = false;
    
    // Compute next direction, reflectance and PDF
    if (current.isSpecular() && isect.bsdf()->hasType(BxDFType::Specular)) {
        // Specular reflection / transmission
        if (current.isReflection()) {
            // Reflection
            *wiOffset = vect::reflect(woOffset, isect.ns());
        } else if (current.isTransmission()) {
            // Transmission
            bool entering = vect::dot(woOffset, isect.ns()) > 0.0;
            double eta = entering ? 1.0 / isect.bsdf()->eta() : isect.bsdf()->eta();
            if (!vect::refract(woOffset, vect::faceforward(isect.ns(), woOffset), eta, wiOffset)) {
                return false;
            }
        }

        *f = isect.bsdf()->f(woOffset, *wiOffset);
        *pdf = isect.bsdf()->pdf(woOffset, *wiOffset);
        *specularBounce = true;
        if (f->isBlack() || (*pdf) == 0.0) {
            return false;
        }
    } else if ((current.isDiffuse() && isect.bsdf()->hasType(BxDFType::Diffuse)) &&
               (next.isDiffuse() || next.isAreaLight())) {
        // Reconnect
        *wiOffset = vect::normalize(next.pos() - isect.pos());
        *f = isect.bsdf()->f(woOffset, *wiOffset);
        *pdf = isect.bsdf()->pdf(woOffset, *wiOffset);
        *reconnect = true;
    } else if ((current.isDiffuse() && isect.bsdf()->hasType(BxDFType::Diffuse)) ||
               (current.isGlossy() && isect.bsdf()->hasType(BxDFType::Glossy))) {
        // Half-vector copy
        const Vector3d whLocalBase = current.surfaceRecord.whLocal;
        const Vector3d whOffset = vect::normalize(whLocalBase.x() * isect.dpdu() +
                                                  whLocalBase.y() * isect.dpdv() +
                                                  whLocalBase.z() * Vector3d(isect.normal()));
        if (current.isReflection()) {
            // Reflection
            *wiOffset = vect::reflect(isect.wo(), whOffset);
            // Side check
            if (vect::dot(*wiOffset, isect.normal()) * vect::dot(woOffset, isect.normal()) < 0.0) {
                return false;
            }
        } else if (current.isTransmission()) {
            // Transmission
            if (!vect::refract(woOffset, whOffset, current.surfaceRecord.eta, wiOffset)) {
                return false;                
            }
            // Side check
            if (vect::dot(*wiOffset, isect.normal()) * vect::dot(woOffset, isect.normal()) > 0.0) {
                return false;
            }
        }
        *f = isect.bsdf()->f(woOffset, *wiOffset);
        *pdf = isect.bsdf()->pdf(woOffset, *wiOffset);
    } else {
        // Different material
        return false;
    }

    // Compute Jacobian
    const Vector3d wiBase = vect::normalize(next.pos() - current.pos());
    const Vector3d woBase = vect::normalize(prev.pos() - current.pos());
    if (*reconnect) {
        // Reconnect
        Normal3d nx = current.intr->normal();
        Normal3d ny = isect.ns();
        double cosThetaX = std::max(0.0, vect::dot(nx, wiBase));
        double cosThetaY = std::max(0.0, vect::dot(ny, *wiOffset));
        double distX = (next.pos() - current.pos()).squaredNorm();
        double distY = (next.pos() - isect.pos()).squaredNorm();
        if (cosThetaX * distY == 0.0) {
            *J = 0.0;
        } else {
            *J = (cosThetaY * distX) / (cosThetaX * distY);
        }
    } else {
        // Half-vector copy
        if (current.isReflection()) {
            // Reflection
            const Vector3d whBase = current.surfaceRecord.wh;
            const Vector3d whOffset = vect::normalize(woOffset + (*wiOffset));
            const double dotX = std::max(0.0, vect::dot(woBase, whBase));
            const double dotY = std::max(0.0, vect::dot(woOffset, whOffset));
            if (dotX == 0.0) {
                *J = 0.0;
            } else {
                *J = dotY / dotX;
            }
        } else if (current.isTransmission()) {
            // Transmission
            double etaX = current.surfaceRecord.eta;
            if (vect::dot(current.normal(), woBase) < 0.0) {
                etaX = 1.0 / etaX;
            }
            double etaY = isect.bsdf()->eta();
            if (vect::dot(isect.normal(), woOffset) < 0.0) {
                etaY = 1.0 / etaY;
            }
            
            const Vector3d whBase = current.surfaceRecord.wh;
            const Vector3d whOffset = vect::normalize(woOffset + etaY * (*wiOffset));
            const double dotX = vect::absDot(wiBase, whBase);
            const double dotY = vect::absDot(*wiOffset, whOffset);
            const double distX = (etaX * wiBase + woBase).squaredNorm();
            const double distY = (etaY * (*wiOffset) + woOffset).squaredNorm();
            if (dotX * distY == 0.0) {
                *J = 0.0;
            } else {
                *J = (dotY * distX) / (dotX * distY);
            }
        } else {
            FatalError("The surface vertex has neither reflect or transmit type!");
        }
    }
    return true;
}

TraceRecord shiftMap(const Scene &scene, RenderParams &params, const Ray &r, Sampler &sampler, MemoryArena &arena,
                     const std::vector<Vertex> &baseVerts) {
    // Special case
    if (baseVerts.size() <= 2 && baseVerts[baseVerts.size() - 1].isLight()) {
        return TraceRecord(PathType::NotInvertible);
    }

    // Initialization
    Spectrum L(0.0);
    Spectrum beta(1.0);
    double totalPDF = 1.0;
    double jacobian = 1.0;
    Ray ray(r);
    bool reconnect = false;
    bool specularBounce = false;

    // Process interactions
    int bounces;
    for (bounces = 1; bounces < baseVerts.size(); bounces++) {
        // Target and neighboring vertices
        const Vertex &current = baseVerts[bounces];
        
        // Find next vertex
        SurfaceInteraction isect;
        bool isIntersect = scene.intersect(ray, &isect);
        if (!isIntersect) {
            return TraceRecord(PathType::NotInvertible);
        }

        // If previous and current vertices are reconnected,
        // confirm whether next base and offset vertices are the same.
        if (reconnect) {
            if ((isect.pos() - current.pos()).norm() > 1.0e-6) {
                // Occlusion occur
                return TraceRecord(PathType::NonSymmetric);
            }
            reconnect = false;    
        }

        // Check light endpoint
        if (specularBounce || bounces == 1) {
            Spectrum Le(0.0);
            if (isIntersect) {
                // Area light
                Le = isect.Le(-ray.dir());
            } else {
                // Not area light (e.g. envmap)
                for (const auto &light : scene.lights()) {
                    Le += light->Le(ray);
                }
            }

            if (!Le.isBlack()) {
                if (current.isLight()) {
                    L += beta * Le;
                    break;
                } 
            }
        }
        
        if (current.isLight()) {
            return TraceRecord(PathType::NotInvertible);
        }

        isect.setScatterFuncs(ray, arena);
        Assertion(isect.bsdf() != nullptr, "No BSDF is set to a material!");

        // Next event estimation
        if (isect.bsdf()->numComponents(BxDFType::All & (~BxDFType::Specular)) > 0) {
            if (current.surfaceRecord.sampledLightIndex >= 0) {
                const int nLights = scene.lights().size();
                const int lightID = current.surfaceRecord.sampledLightIndex;
                Assertion(lightID < nLights, "Light ID is invalid: %d > %d", lightID, nLights);
                const auto& light = scene.lights()[lightID];
                const Point2d &randLight = current.surfaceRecord.randLight;
                const Point2d &randShade = current.surfaceRecord.randShade;
                Spectrum Ld = nLights * beta *
                              estimateDirectLight(isect, randShade, *light, randLight,
                                                  scene, sampler, arena);
                L += Ld;
            }
        }

        if (bounces == baseVerts.size() - 1) break;
        const Vertex &prev = baseVerts[bounces - 1];
        const Vertex &next = baseVerts[bounces + 1];

        // Next direction
        Vector3d wiSub;
        double J = 1.0, pdf = 0.0;
        Spectrum f(0.0);
        specularBounce = false;
        bool foundNext = nextDirection(isect, prev, current, next, &wiSub, &pdf, &f, &J, &reconnect, &specularBounce);
        if (!foundNext || f.isBlack() || pdf == 0.0 || J == 0.0) {
            return TraceRecord(PathType::NotInvertible);
        }

        // Update information
        beta *= f * vect::absDot(wiSub, isect.ns()) / pdf;
        totalPDF *= pdf * J;
        jacobian *= J;
        ray = isect.spawnRay(wiSub);
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

        int sampledLightIndex = -1;
        Point2d randLight, randShade;
        if (isect.bsdf()->numComponents(BxDFType::All & (~BxDFType::Specular)) > 0) {
            const int nLights = static_cast<int>(scene.lights().size());
            if (nLights != 0) {
                sampledLightIndex = std::min((int)(sampler.get1D() * nLights), nLights - 1);
                const auto& light = scene.lights()[sampledLightIndex];
                randLight = sampler.get2D();
                randShade = sampler.get2D();
                Spectrum Ld = nLights * beta *
                              estimateDirectLight(isect, randShade, *light, randLight,
                                                  scene, sampler, arena);
                L += Ld;
            }
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

        // Compute half vector
        Vector3d wh;
        double eta = 1.0;
        if ((sampledType & BxDFType::Transmission) != BxDFType::None) {
            
            eta = isect.bsdf()->eta();
            if (vect::dot(wo, isect.normal()) < 0.0) {
                // in -> out
                eta = 1.0 / eta;
            }
            wh = vect::normalize(wi * eta + wh);
        } else {
            wh = vect::normalize(wi + wo);            
        }
        Vector3d whLocal = Vector3d(wh.dot(isect.dpdu()), wh.dot(isect.dpdv()), wh.dot(Vector3d(isect.normal()))).normalized();


        // Add vertex
        SurfaceEventRecord record{
            wh, whLocal, sampledType, eta, sampledLightIndex, randLight, randShade
        };
        addItem(Vertex::createSurface(isect, record));

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

GDPTIntegrator::GDPTIntegrator(const std::shared_ptr<Sampler> &sampler,
                               const std::string &solver)
    : Integrator{}
    , sampler_{sampler}
    , solver_{solver} {
}

GDPTIntegrator::GDPTIntegrator(RenderParams &params)
    : GDPTIntegrator{std::static_pointer_cast<Sampler>(params.getObject("sampler", true)),
                     params.getString("solver", "L1", true)} {
}

void GDPTIntegrator::render(const std::shared_ptr<const Camera> &camera,
                            const Scene &scene,
                            RenderParams &params) {
    // Initialization
    Integrator::render(camera, scene, params);
    auto initSampler = sampler_->clone((unsigned int)time(0));
    
    const int width = camera->film()->resolution().x();
    const int height = camera->film()->resolution().y();
    GDPTFilm film(camera->film());

    const int numThreads = numSystemThreads();
    auto samplers = std::vector<std::unique_ptr<Sampler>>(numThreads);
    auto arenas   = std::vector<MemoryArena>(numThreads);

    // Trace rays
    const int offsetX[] = { -1, 1, 0, 0 };
    const int offsetY[] = { 0, 0, -1, 1 };
    const int numPixels  = width * height;
    const int numSamples = params.getInt("sampleCount");
    
    Image inversionRatio(width, height);
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
        int nInvertedPath = 0;
        int nSampledPath = 0;
        parallel_for(0, numPixels, [&](int pid) {
            const int threadID = getThreadID();
            const auto &sampler = samplers[threadID];
            sampler->startPixel();

            const int y = pid / width;
            const int x = pid % width;
            const Point2d randFilm = sampler->get2D();
            const Point2d randLens = sampler->get2D();
            const double filterWeight = film.evalFilter(randFilm);
            const Ray ray = camera->spawnRay(Point2i(width - x - 1, y), randFilm, randLens);

            // Base path
            std::vector<Vertex> baseVerts;
            TraceRecord baseRecord = pathTrace(scene, params, ray, *sampler, arenas[threadID], &baseVerts);
            film.addPixel(x, y, baseRecord.f, filterWeight);

            // Offset path
            double invRatio = 0.0;
            for (int k = 0; k < 4; k++) {
                const int nx = x + offsetX[k];
                const int ny = y + offsetY[k];
                if (nx < 0 || ny < 0 || nx >= width || ny >= height) {
                    continue;
                }

                // Shift mapping
                const Ray subRay = camera->spawnRay(Point2i(width - nx - 1, ny), randFilm, randLens);
                TraceRecord subRecord(PathType::NotInvertible);
                if (!baseVerts.empty()) {
                    subRecord = shiftMap(scene, params, subRay, *sampler, arenas[threadID], baseVerts);
                }

                // Compute contribution
                Spectrum G;
                if (subRecord.type == PathType::Invertible) {
                    // Compute gradient with MIS
                    // Eq.(10) of original paper
                    const double misWeight = baseRecord.pdf / (baseRecord.pdf + subRecord.pdf);
                    G = misWeight * (baseRecord.f - subRecord.f);
                    nInvertedPath += 1;
                    invRatio += 0.25;
                } else if (subRecord.type == PathType::NotInvertible) {
                    // Naively compute gradient with path tracing
                    subRecord = pathTrace(scene, params, subRay, *sampler, arenas[threadID], nullptr);
                    G = 0.5 * (baseRecord.f - subRecord.f);                
                } else if (subRecord.type == PathType::NonSymmetric) {
                    // Subpath results in "zero contribution"
                    G = baseRecord.f;
                }
                nSampledPath += 1;

                // Forward or backward difference
                if (k == 0 || k == 2) {
                    film.addGradient(x, y, k,  G, filterWeight);
                } else {
                    film.addGradient(x, y, k, -G, filterWeight);
                }
        
                // Debug
                #ifdef GDPT_TAKE_LOG
                shiftImages[k].pixel(x, y) += subRecord.f;
                misImages[k].pixel(x, y) += Spectrum(subRecord.jacobian);
                #endif
            }
            inversionRatio.pixel(x, y) += Spectrum(invRatio);

            proc++;
            if (proc % 1000 == 0 || proc == numPixels) {
                printf("\r[ %d / %d ] %6.2f %% processed...", i + 1, numSamples, 100.0 * proc / numPixels);
                fflush(stdout);
            }
        });
        printf("\n");
        MsgInfo("%d / %d samples inverted!", nInvertedPath, nSampledPath);

        film.save(i + 1, solver_);

        #ifdef GDPT_TAKE_LOG
        Image temp;
        char filename[256];
        for (int k = 0; k < 4; k++) {
            temp = misImages[k];
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    temp.pixel(x, y) /= (i + 1);   
                }
            }

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
        
        temp = inversionRatio;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                temp.pixel(x, y) /= (i + 1);
            }
        }
        temp.save("inversion.png");
        #endif

        for (int t = 0; t < numThreads; t++) {
            arenas[t].reset();
        }
    }
    printf("Finish!!\n");
}

}  // namespace spica
