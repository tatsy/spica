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

struct Vertex {
    static Vertex createCamera(const Point3d &pos) {
        Vertex v;
        v.intr = std::make_shared<Interaction>(pos);
        v.type = VertexType::Camera;
        return v;
    }

    static Vertex createSurface(const SurfaceInteraction &intr_,
                                const Vector3d &wh_,
                                BxDFType bxdfType_,
                                double eta_ = 1.0) {
        Vertex v;
        v.intr     = std::make_shared<SurfaceInteraction>(intr_);
        v.wh       = wh_;
        v.bxdfType = bxdfType_;
        v.eta      = eta_;
        v.type     = VertexType::Surface;
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
               (bxdfType & BxDFType::Diffuse) != BxDFType::None;
    }

    bool isGlossy() const {
        return type == VertexType::Surface && 
               (bxdfType & BxDFType::Glossy) != BxDFType::None;
    }

    bool isSpecular() const {
        return type == VertexType::Surface && 
               (bxdfType & BxDFType::Specular) != BxDFType::None;
    }

    bool isReflection() const {
        return type == VertexType::Surface && 
               (bxdfType & BxDFType::Reflection) != BxDFType::None;    
    }

    bool isTransmission() const {
        return type == VertexType::Surface && 
               (bxdfType & BxDFType::Transmission) != BxDFType::None;    
    }

    std::shared_ptr<Interaction> intr;
    Vector3d wh;
    BxDFType bxdfType;
    double eta;
    VertexType type;
};

bool nextDirection(const SurfaceInteraction &isect, const Vertex &prev, const Vertex &current, const Vertex &next,
                   Vector3d *wiOffset, double *pdf, Spectrum *f, double *J, bool *reconnect) {

    const Vector3d woOffset = isect.wo();
    *reconnect = false;

    // Compute next direction, reflectance and PDF
    if (current.isSpecular() && isect.bsdf()->hasType(BxDFType::Specular)) {
        // Specular transmission
        *f = isect.bsdf()->sample(woOffset, wiOffset, Point2d(), pdf, current.bxdfType);            
        if (f->isBlack()) {
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
        const Vector3d whBase = current.wh;
        if (current.isReflection()) {
            // Reflection
            *wiOffset = vect::reflect(isect.wo(), whBase);
            // Side check
            if (vect::dot(*wiOffset, isect.normal()) * vect::dot(woOffset, isect.normal()) < 0.0) {
                return false;
            }
        } else if (current.isTransmission()) {
            // Transmission
            if (!vect::refract(woOffset, whBase, current.eta, wiOffset)) {
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
        Normal3d ny = isect.normal();
        double cosThetaX = std::max(0.0, vect::dot(nx, wiBase));
        double cosThetaY = std::max(0.0, vect::dot(ny, *wiOffset));
        double distX = (next.pos() - current.pos()).squaredNorm();
        double distY = (next.pos() - isect.pos()).squaredNorm();
        *J = (cosThetaY * distX) / (cosThetaX * distY + EPS);    
    } else {
        // Half-vector copy
        if (current.isReflection()) {
            // Reflection
            const Vector3d whBase = current.wh;
            const Vector3d whOffset = vect::normalize(woOffset + (*wiOffset));
            const double dotX = std::max(0.0, vect::dot(woBase, whBase));
            const double dotY = std::max(0.0, vect::dot(woOffset, whOffset));
            *J = dotY / (dotX + EPS);
        } else if (current.isTransmission()) {
            // Transmission
            double etaX = current.eta;
            if (vect::dot(current.normal(), woBase) < 0.0) {
                etaX = 1.0 / etaX;
            }
            double etaY = isect.bsdf()->eta();
            if (vect::dot(isect.normal(), woOffset) < 0.0) {
                etaY = 1.0 / etaY;
            }
            
            const Vector3d whBase = current.wh;
            const Vector3d whOffset = vect::normalize(woOffset + etaY * (*wiOffset));
            const double dotX = vect::absDot(wiBase, whBase);
            const double dotY = vect::absDot(*wiOffset, whOffset);
            const double distX = (etaX * wiBase + woBase).squaredNorm();
            const double distY = (etaY * (*wiOffset) + woOffset).squaredNorm();
            *J = (dotY * distX) / (dotX * distY + EPS);
        } else {
            FatalError("The surface vertex has neither reflect or transmit type!");
        }
    }
    return true;
}

TraceRecord shiftMap(const Scene &scene, RenderParams &params, const Ray &r, Sampler &sampler, MemoryArena &arena,
                     const std::vector<Vertex> &baseVerts) {
    // Special case
    // The ray directory hits a light, or goes outside the scene.
    if (baseVerts.size() <= 2) {
        return TraceRecord(PathType::NotInvertible);
    }

    // Initialization
    Spectrum L(0.0);
    Spectrum beta(1.0);
    double totalPDF = 1.0;
    double jacobian = 1.0;
    Ray ray(r);
    bool reconnect = false;

    // Process interactions
    int bounces;
    for (bounces = 1; bounces < baseVerts.size(); bounces++) {
        // Target and neighboring vertices
        const Vertex &current = baseVerts[bounces];

        // Find next vertex
        SurfaceInteraction isect;
        if (!scene.intersect(ray, &isect)) {
            return TraceRecord(PathType::NotInvertible);
        }

        // If previous and current vertices are reconnected,
        // confirm whether next base and offset vertices are the same.
        if (reconnect) {
            double dist1 = (isect.pos() - ray.org()).norm();
            double dist2 = (current.pos() - baseVerts[bounces - 1].pos()).norm();
            if (std::abs(dist1 - dist2) > 1.0e-6) {
                // Occlusion occur
                return TraceRecord(PathType::NonSymmetric);
            }
            reconnect = false;    
        }

        // Check light endpoint
        if (current.isLight()) {
            Spectrum Le = current.Le(scene, ray);
            if (Le.isBlack()) {
                return TraceRecord(PathType::NonSymmetric);
            }
            L += beta * Le;
            break;
        }

        isect.setScatterFuncs(ray, arena);
        Assertion(isect.bsdf() != nullptr, "No BSDF is set to a material!");

        // Next event estimation
        if (isect.bsdf()->numComponents(BxDFType::All & (~BxDFType::Specular)) > 0) {
            Spectrum Ld = beta * uniformSampleOneLight(isect, scene, arena, sampler);
            L += Ld;            
        }

        if (bounces == baseVerts.size() - 1) break;
        const Vertex &prev = baseVerts[bounces - 1];
        const Vertex &next = baseVerts[bounces + 1];

        // Next direction
        Vector3d wiSub;
        double J = 1.0, pdf = 0.0;
        Spectrum f(0.0);
        bool foundNext = nextDirection(isect, prev, current, next, &wiSub, &pdf, &f, &J, &reconnect);
        if (!foundNext || f.isBlack() || pdf == 0.0) {
            return TraceRecord(PathType::NotInvertible);
        }

        // Update information
        beta *= f * vect::absDot(wiSub, isect.ns()) * J / pdf;
        totalPDF *= pdf;
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

        // Add vertex
        addItem(Vertex::createSurface(isect, wh, sampledType, eta));

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
                    subRecord = shiftMap(scene, params, subRay, *samplers[threadID], arenas[threadID], baseVerts);
                }

                // Compute contribution
                Spectrum G;
                if (subRecord.type == PathType::Invertible) {
                    // Compute gradient with MIS
                    // Eq.(10) of original paper
                    const double misWeight = baseRecord.pdf / (baseRecord.pdf + subRecord.pdf * subRecord.jacobian);
                    G = misWeight * (baseRecord.f - subRecord.f);
                } else if (subRecord.type == PathType::NotInvertible) {
                    // Naively compute gradient with path tracing
                    subRecord = pathTrace(scene, params, subRay, *samplers[threadID], arenas[threadID], nullptr);
                    G = 0.5 * (baseRecord.f - subRecord.f);                
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
