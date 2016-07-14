#define SPICA_API_EXPORT
#include "bdpt.h"

#include <mutex>

#include "../core/ray.h"
#include "../core/interaction.h"
#include "../core/sampling.h"
#include "../core/memory.h"
#include "../core/parallel.h"
#include "../core/renderparams.h"

#include "../image/film.h"
#include "../scenes/scene.h"

#include "../light/spica_light.h"

#include "../camera/spica_camera.h"

#include "../bxdf/bxdf.h"
#include "../bxdf/phase.h"
#include "../bxdf/bsdf.h"

#include "../medium/medium.h"

#include "../random/sampler.h"

#include "mis.h"

namespace spica {

enum class VertexType : int {
    Camera, Light, Surface, Medium
};

struct EndpointInteraction : Interaction { 
    union {
        const Camera* camera;
        const Light* light;
    };

    EndpointInteraction() 
        : Interaction{}
        , light{ nullptr } {
    }

    EndpointInteraction(const Ray& ray)
        : Interaction { ray.proceeded(1.0) }
        , light{ nullptr } {
        normal_ = Normal3d(-ray.dir());
    }

    EndpointInteraction(const Interaction& it, const Light* light_)
        : Interaction{ it }
        , light{ light_ } {
    }

    EndpointInteraction(const Interaction& it, const Camera* camera_)
        : Interaction{ it }
        , camera{ camera_ } {
    }

    EndpointInteraction(const Camera* camera_, const Ray& ray)
        : Interaction{ ray.org() }
        , camera{ camera_ } {
    }

    // TODO: a bit different.
    EndpointInteraction(const Light* light_, const Ray& r, const Normal3d& nl)
        : Interaction{ r.org() }
        , light{ light_ } {
        normal_ = nl;        
    }
};

template <class Type>
class ScopedAssignment {
public:
    ScopedAssignment(Type* target_ = nullptr, Type value = Type())
        : target{ target_ } {
        if (target) {
            backup = *target;
            *target = value;
        }
    }

    ~ScopedAssignment() {
        if (target) *target = backup;
    }

    ScopedAssignment(const ScopedAssignment& sa) = delete;
    ScopedAssignment& operator=(const ScopedAssignment& sa) = delete;

    ScopedAssignment& operator=(ScopedAssignment&& sa) {
        this->target = sa.target;
        this->backup = sa.backup;
        sa.target = nullptr;
        return *this;
    }

private:
    Type* target, backup;
};

struct Vertex {
    // Public methods
    Vertex() {
        intr = std::make_shared<EndpointInteraction>();
    }

    Vertex(VertexType type_, const EndpointInteraction& ei, const Spectrum& beta_)
        : type{ type_ }
        , beta{ beta_ } {
        intr = std::make_shared<EndpointInteraction>(ei);
    }

    Vertex(const SurfaceInteraction& it, const Spectrum& beta_)
        : type{ VertexType::Surface }
        , beta{ beta_ } {
        intr = std::make_shared<SurfaceInteraction>(it);
    }

    Vertex(const MediumInteraction& it, const Spectrum& beta_)
        : type{ VertexType::Medium } 
        , beta{ beta_ } {
        intr = std::make_shared<MediumInteraction>(it);
    }

    Vertex(const Vertex& v) {
        operator=(v);
    }

    Vertex& operator=(const Vertex& v) {
        this->type = v.type;
        this->beta = v.beta;
        this->intr = v.intr;
        this->pdfFwd = v.pdfFwd;
        this->pdfRev = v.pdfRev;
        this->pdfRev = v.pdfRev;
        this->delta = v.delta;
        return *this;
    }

    inline Point3d pos() const { return intr->pos(); }
    inline Normal3d normal() const { return intr->normal(); }

    Spectrum Le(const Scene& scene, const Vertex& v) const {
        if (!isLight()) return Spectrum(0.0);

        Vector3d w = v.pos() - this->pos();
        if (w.squaredNorm() == 0.0) return Spectrum(0.0);

        w = w.normalized();
        if (isIBL()) {
            Spectrum ret(0.0);
            for (const auto& l : scene.lights()) {
                ret += l->Le(Ray(pos(), w));
            }
            return ret;
        } else {
            const AreaLight* l = si()->primitive()->areaLight();
            Assertion(l != nullptr, "Area light not detected");
            return l->L(*si(), w);
        }
    }

    inline SurfaceInteraction* si() const {
        return (SurfaceInteraction*)intr.get();
    }

    inline MediumInteraction* mi() const {
        return (MediumInteraction*)intr.get();
    }

    inline EndpointInteraction* ei() const {
        return (EndpointInteraction*)intr.get();
    }

    inline bool isOnSurface() const {
        return normal() != Normal3d();
    }

    Spectrum f(const Vertex& next) const {
        Vector3d wi = next.pos() - this->pos();
        if (wi.squaredNorm() == 0.0) return Spectrum(0.0);

        wi = wi.normalized();
        switch (type) {
        case VertexType::Surface:
            return si()->bsdf()->f(si()->wo(), wi);

        case VertexType::Medium:
            return Spectrum(mi()->phase()->p(mi()->wo(), wi));

        default:
            FatalError("Vertex::f() not implemented!!");
            return Spectrum(0.0);
        }
    }

    bool isConnectible() const {
        switch (type) {
        case VertexType::Medium:
            return true;
        
        case VertexType::Light:
            // In the future, followling line should be revised for directional light.
            return true;

        case VertexType::Camera:
            return true;

        case VertexType::Surface:
            return si()->bsdf()->numComponents(BxDFType::Diffuse | BxDFType::Glossy |
                                               BxDFType::Reflection |
                                               BxDFType::Transmission) > 0;
        }

        FatalError("Unhandled vertex type in isConnectible()");
        return false;
    }

    bool isLight() const {
        return type == VertexType::Light ||
               (type == VertexType::Surface && si()->primitive()->areaLight());
    }

    bool isDeltaLight() const {
        return false;
    }

    bool isIBL() const {
        // In the future, followling line should be revised for directional light.
        return type == VertexType::Light && (!ei()->light);
    }

    // Convert PDF to that considers solid angle density.
    double convertDensity(double pdf, const Vertex& next) const {
        if (next.isIBL()) return pdf;

        Vector3d w = next.pos() - this->pos();
        double dist2 = w.squaredNorm();
        if (dist2 == 0.0) return 0.0;

        double invDist2 = 1.0 / dist2;
        if (next.isOnSurface()) {
            pdf *= vect::absDot(next.normal(), w * std::sqrt(invDist2));
        }
        return pdf * invDist2;
    }

    double pdf(const Scene& scene, const Vertex* prev,
               const Vertex& next) const {
        if (type == VertexType::Light) return pdfLight(scene, next);

        Vector3d wn = next.pos() - this->pos();
        if (wn.squaredNorm() == 0.0) return 0.0;

        wn = wn.normalized();
        Vector3d wp;
        if (prev) {
            wp = prev->pos() - this->pos();
            if (wp.squaredNorm() == 0.0) return 0.0;
            wp = wp.normalized();
        } else {
            Assertion(type == VertexType::Camera, "Here, type should be camera");
        }

        double pdf = 0.0, unused;
        if (type == VertexType::Camera) {
            ei()->camera->pdfWe(ei()->spawnRay(wn), &unused, &pdf);
        } else if (type == VertexType::Surface) {
            pdf = si()->bsdf()->pdf(wp, wn);
        } else if (type == VertexType::Medium) {
            pdf = mi()->phase()->p(wp, wn);
        } else {
            FatalError("Vertex::pdf() not implemented");
        }

        return convertDensity(pdf, next);
    }

    double pdfLight(const Scene& scene, const Vertex& v) const {
        Vector3d w = v.pos() - this->pos();
        double invDist2 = 1.0 / w.squaredNorm();
        w *= std::sqrt(invDist2);

        double pdf;
        if (isIBL()) {
            Bounds3d b = scene.worldBound();
            Point3d worldCenter = (b.posMin() + b.posMax()) * 0.5;
            double worldRadius = (b.posMax() - worldCenter).norm();
            pdf = 1.0 / (PI * worldRadius * worldRadius);
        } else {
            Assertion(isLight(), "Here, vertex type should be light");
            const Light* light = type == VertexType::Light ? ei()->light
                                                           : si()->primitive()->areaLight();
            Assertion(light != nullptr, "Light is null");

            double pdfPos, pdfDir;
            light->pdfLe(Ray(pos(), w), normal(), &pdfPos, &pdfDir);
            pdf = pdfDir * invDist2;
        }

        if (v.isOnSurface()) pdf *= vect::absDot(v.normal(), w);
        return pdf;
    }

    double pdfLightOrigin(const Scene& scene, const Vertex& v,
                          const Distribution1D& lightDist) const {
        Vector3d w = v.pos() - this->pos();
        if (w.squaredNorm() == 0.0) return 0.0;

        w = w.normalized();
        if (isIBL()) {
        
        } else {
            double pdfPos, pdfDir, pdfChoise = 0.0;
            Assertion(isLight(), "This object should not be light.");
           
            const Light* light = type == VertexType::Light ? ei()->light
                                                           : si()->primitive()->areaLight();
            Assertion(light != nullptr, "Light is nullptr");

            for (int i = 0; i < scene.lights().size(); i++) {
                if (scene.lights()[i].get() == light) {
                    pdfChoise = lightDist.pdfDiscrete(i);
                    break;
                }
            }
            Assertion(pdfChoise != 0.0, "Current light is not included in the scene");

            light->pdfLe(Ray(pos(), w), normal(), &pdfPos, &pdfDir);
            return pdfPos * pdfChoise;
        }
        return 0.0;
    }

    static inline Vertex createCamera(const Camera* camera, const Ray& ray,
                                      const Spectrum& beta) {
        return Vertex(VertexType::Camera, EndpointInteraction(camera, ray), beta);            
    }

    static inline Vertex createCamera(const Camera* camera, const Interaction& it,
                                      const Spectrum& beta) {
        return Vertex(VertexType::Camera, EndpointInteraction(it, camera), beta);
    }

    static inline Vertex createLight(const EndpointInteraction& ei,
                                     const Spectrum& beta, double pdf) {
        Vertex v(VertexType::Light, ei, beta);
        v.pdfFwd = pdf;
        return v;
    }

    static inline Vertex createLight(const Light& light, const Ray& ray,
                                     const Normal3d& nrmLight, const Spectrum& Le,
                                     double pdf) {
        Vertex v(VertexType::Light, EndpointInteraction(&light, ray, nrmLight), Le);
        v.pdfFwd = pdf;
        return v;
    }

    static inline Vertex createSurface(const SurfaceInteraction& isect,
                                       const Spectrum& beta, double pdf,
                                       const Vertex& prev) {
        Vertex v(isect, beta);
        v.pdfFwd = prev.convertDensity(pdf, v);
        return v;
    }

    static inline Vertex createMedium(const MediumInteraction& mi,
                                      const Spectrum& beta, double pdf,
                                      const Vertex& prev) {
        Vertex v(mi, beta);
        v.pdfFwd = prev.convertDensity(pdf, v);
        return v;
    }

    const Interaction& getInteraction() const {
        switch (type) {
        case VertexType::Medium:
            return *mi();

        case VertexType::Surface:
            return *si();

        default:
            return *ei();
        }
    }

    // Public fields
    VertexType type;
    Spectrum beta;
    std::shared_ptr<Interaction> intr = nullptr;
    double pdfFwd = 0.0;
    double pdfRev = 0.0;
    bool delta = false;
};

int randomWalk(const Scene& scene, Ray ray, Sampler& sampler,
                MemoryArena& arena, Spectrum beta, double pdf, int maxDepth,
                Vertex* path, bool isCamera) {
    if (maxDepth == 0) return 0;

    int bounces = 0;
    double pdfFwd = pdf, pdfRev = 0.0;
    while (true) {
        MediumInteraction mi;

        SurfaceInteraction isect;
        bool isIntersect = scene.intersect(ray, &isect);

        if (ray.medium()) {
            beta *= ray.medium()->sample(ray, sampler, arena, &mi);
        }

        if (beta.isBlack()) break;

        Vertex& vertex = path[bounces];
        Vertex& prev   = path[bounces - 1];
        if (mi.isValid()) {
            // Process medium interaction.
            vertex = Vertex::createMedium(mi, beta, pdfFwd, prev);
            if (++bounces >= maxDepth) break;
        
            Vector3d wi;
            pdfFwd = pdfRev = mi.phase()->sample(-ray.dir(), &wi, sampler.get2D());
            ray = mi.spawnRay(wi);
        } else {
            // Process surface interaction.
            if (!isIntersect) {
                if (isCamera) {
                    vertex = Vertex::createLight(EndpointInteraction(ray), beta, pdfFwd);
                    ++bounces;
                }   
                break;
            }

            // If medium boundary is detected, current intersection is skipped.
            isect.setScatterFuncs(ray, arena);
            if (!isect.bsdf()) {
                ray = isect.spawnRay(ray.dir());
                continue;
            }

            // Store surface interaction.
            vertex = Vertex::createSurface(isect, beta, pdfFwd, prev);
            if (++bounces >= maxDepth) break;

            // Sample next direction and compute reverse probability.
            Vector3d wi, wo = isect.wo();
            BxDFType type;
            Spectrum f = isect.bsdf()->sample(wo, &wi, sampler.get2D(), &pdfFwd,
                                              BxDFType::All, &type);
            if (f.isBlack() || pdfFwd == 0.0) break;

            beta *= f * vect::absDot(wi, isect.normal()) / pdfFwd;
            pdfRev = isect.bsdf()->pdf(wi, wo, BxDFType::All);
            if ((type & BxDFType::Specular) != BxDFType::None) {
                vertex.delta = true;
                pdfRev = pdfFwd = 0.0;
            }
            ray = isect.spawnRay(wi);
        }
        prev.pdfRev = vertex.convertDensity(pdfRev, prev);
    }
    return bounces;
}

int calcCameraSubpath(const Scene& scene, Sampler& sampler,
                       MemoryArena& arena, int maxDepth,
                       const Camera& camera, const Point2i& pixel,
                       const Point2d& randFilm,
                       Vertex* path) {
    if (maxDepth == 0) return 0;

    Point2d randLens = sampler.get2D();

    Ray ray = camera.spawnRay(pixel, randFilm, randLens); 
    Spectrum beta(1.0);

    path[0] = Vertex::createCamera(&camera, ray, beta);
    
    double pdfPos, pdfDir;
    camera.pdfWe(ray, &pdfPos, &pdfDir);
    return randomWalk(scene, ray, sampler, arena, beta, pdfDir, maxDepth - 1, path + 1, true) + 1;
}

int calcLightSubpath(const Scene& scene, Sampler& sampler,
                      MemoryArena& arena, int maxDepth, const Distribution1D& lightDist,
                      Vertex* path) {
    if (maxDepth == 0) return 0;

    // Sample light.
    double lightPdf;
    const int lightID = lightDist.sampleDiscrete(sampler.get1D(), &lightPdf);
    const auto& light = scene.lights()[lightID];

    // Generate a ray, and compute contributing light radiance.
    Ray ray;
    Normal3d nrmLight;
    double pdfPos, pdfDir;
    Spectrum Le = light->sampleLe(sampler.get2D(), sampler.get2D(), &ray,
                                  &nrmLight, &pdfPos, &pdfDir);
    if (pdfPos == 0.0 || pdfDir == 0.0 || Le.isBlack()) return 0;

    // Create light end point vertex.
    path[0] = Vertex::createLight(*light, ray, nrmLight, Le, pdfPos * lightPdf);
    Spectrum beta = Le * vect::absDot(nrmLight, ray.dir()) / (lightPdf * pdfPos * pdfDir);

    // Compute a path by random walk.
    int bounces = randomWalk(scene, ray, sampler, arena, beta, pdfDir,
                             maxDepth - 1, path + 1, false);

    // Correct sampling density for image-based light.
    if (path[0].isIBL()) {
        if (bounces > 0) {
            path[1].pdfFwd = pdfPos;
            if (path[1].isOnSurface()) {
                path[1].pdfFwd *= vect::absDot(ray.dir(), path[1].normal());
            }
        }

        path[0].pdfFwd = 0.0;
    }

    return bounces + 1;
}

double calcMISWeight(const Scene& scene,
                     Vertex* lightPath, Vertex* cameraPath,
                     Vertex& sampled,
                     int lightID, int cameraID, 
                     const Distribution1D& lightDist) {
    // Single bounce connection.
    if (lightID + cameraID == 2) return 1.0;

    // To handle specular reflectio, zero contribution is remapped to 1.0.
    auto remap0 = [](double f) -> double { return f != 0.0 ? f : 1.0; };

    // Take current and previous sample.
    Vertex* vl = lightID > 0 ? &lightPath[lightID - 1] : nullptr;
    Vertex* vc = cameraID > 0 ? &cameraPath[cameraID - 1] : nullptr;
    Vertex* vlMinus = lightID > 1 ? &lightPath[lightID - 2] : nullptr;
    Vertex* vcMinus = cameraID > 1 ? &cameraPath[cameraID - 2] : nullptr;

    // Temporal swaps in this function scope.
    ScopedAssignment<Vertex> a1;
    if (lightID == 1) {
        a1 = { vl, sampled };
    } else if (cameraID == 1) {
        a1 = { vc, sampled };
    }

    ScopedAssignment<bool> a2, a3;
    if (vc) a2 = { &vc->delta, false };
    if (vl) a3 = { &vl->delta, false };

    ScopedAssignment<double> a4;
    if (vc) {
        a4 = { &vc->pdfRev, lightID > 0 
                                      ? vl->pdf(scene, vlMinus, *vc)
                                      : vc->pdfLightOrigin(scene, *vcMinus, lightDist) };
    }

    ScopedAssignment<double> a5;
    if (vcMinus) {
        a5 = { &vcMinus->pdfRev, lightID > 0 ? vc->pdf(scene, vl, *vcMinus)
                                             : vc->pdfLight(scene, *vcMinus) };
    }

    ScopedAssignment<double> a6;
    if (vl) {
        a6 = { &vl->pdfRev, vc->pdf(scene, vcMinus, *vl) };
    }
    ScopedAssignment<double> a7;
    if (vlMinus) {
        a7 = { &vlMinus->pdfRev, vl->pdf(scene, vc, *vlMinus) };
    }

    // Compute sum of path contributions of the same length.
    double sumRi = 0.0;
    double ri = 1.0;
    for (int i = cameraID - 1; i > 0; i--) {
        ri *= remap0(cameraPath[i].pdfRev) / remap0(cameraPath[i].pdfFwd);
        if (cameraPath[i].delta || cameraPath[i - 1].delta) continue;

        sumRi += ri;
    }

    ri = 1.0;
    for (int i = lightID - 1; i >= 0; i--) {
        ri *= remap0(lightPath[i].pdfRev) / remap0(lightPath[i].pdfFwd);
        bool isDeltaLight = i > 0 ? lightPath[i - 1].delta
                                  : lightPath[0].isDeltaLight();
        if (lightPath[i].delta || isDeltaLight) continue;

        sumRi += ri;
    }

    return 1.0 / (1.0 + sumRi);
}

Spectrum G(const Scene& scene, Sampler& sampler, const Vertex& v0,
           const Vertex& v1) {
    Vector3d d = v0.pos() - v1.pos();
    double g = 1.0 / d.squaredNorm();

    d *= std::sqrt(g);
    if (v0.isOnSurface()) g *= vect::absDot(v0.normal(), d);
    if (v1.isOnSurface()) g *= vect::absDot(v1.normal(), d);

    VisibilityTester vis(v0.getInteraction(), v1.getInteraction());
    return g * vis.transmittance(scene, sampler);
}

Spectrum connectBDPT(const Scene& scene,
                     Vertex* lightPath, Vertex* cameraPath,
                     int lightID, int cameraID, const Distribution1D& lightDist,
                     const Camera& camera, Sampler& sampler, Point2d* pRaster,
                     double* misWeight) {
    Spectrum L(0.0);
    if (cameraID > 1 && lightID != 0 &&
        cameraPath[cameraID - 1].type == VertexType::Light) {
        return Spectrum(0.0);
    }

    Vertex sampled;
    if (lightID == 0) {
        const Vertex& vc = cameraPath[cameraID - 1];
        if (vc.isLight()) L = vc.Le(scene, cameraPath[cameraID - 2]) * vc.beta;
    } else if (cameraID == 1) {
        const Vertex& vl = lightPath[lightID - 1];
        if (vl.isConnectible()) {
            VisibilityTester vis;
            Vector3d wi;
            double pdf;
            Spectrum Wi = camera.sampleWi(vl.getInteraction(), sampler.get2D(),
                                          &wi, &pdf, pRaster, &vis);
            if (pdf > 0.0 && !Wi.isBlack()) {
                sampled = Vertex::createCamera(&camera, vis.p2(), Wi / pdf);
                L = vl.beta * vl.f(sampled) * sampled.beta;
                if (vl.isOnSurface()) L *= vect::absDot(wi, vl.normal());
                if (!L.isBlack()) L *= vis.transmittance(scene, sampler);
            }
        }   
    } else if (lightID == 1) {
        const Vertex& vc = cameraPath[cameraID - 1];
        if (vc.isConnectible()) {
            double lightPdf;
            VisibilityTester vis;
            Vector3d wi;
            double pdf;
            int id = lightDist.sampleDiscrete(sampler.get1D(), &lightPdf);
            const auto& l = scene.lights()[id];

            Spectrum lightWeight = l->sampleLi(vc.getInteraction(), sampler.get2D(),
                                               &wi, &pdf, &vis);
            
            if (pdf > 0.0 && !lightWeight.isBlack()) {
                EndpointInteraction ei(vis.p2(), l.get());
                sampled = Vertex::createLight(ei, lightWeight / (pdf * lightPdf), 0.0);
                sampled.pdfFwd = sampled.pdfLightOrigin(scene, vc, lightDist);
                L = vc.beta * vc.f(sampled) * sampled.beta;

                if (vc.isOnSurface()) L *= vect::absDot(wi, vc.normal());
                if (!L.isBlack()) L *= vis.transmittance(scene, sampler);
            }
        }
    } else {
        const Vertex& vc = cameraPath[cameraID - 1];
        const Vertex& vl = lightPath[lightID - 1];
        if (vc.isConnectible() && vl.isConnectible()) {
            L = vc.beta * vc.f(vl) * vl.f(vc) * vl.beta;
            if (!L.isBlack()) L *= G(scene, sampler, vl, vc);
        }
    }

    double misW = L.isBlack() ? 0.0 : calcMISWeight(scene, lightPath, cameraPath,
                                                    sampled, lightID, cameraID,
                                                    lightDist);
    Assertion(!std::isnan(misW), "Invalid MIS weight!!");

    L *= misW;
    if (misWeight) *misWeight = misW;

    return L;    
}

BDPTIntegrator::BDPTIntegrator(const std::shared_ptr<const Camera>& camera,
                               const std::shared_ptr<Sampler>& sampler)
    : Integrator{ camera }
    , sampler_{ sampler } {
}

BDPTIntegrator::~BDPTIntegrator() {
}

void BDPTIntegrator::render(const Scene& scene,
                       const RenderParams& params) {
    // Initialization
    const int width = camera_->film()->resolution().x();
    const int height = camera_->film()->resolution().y();

    const int numThreads = numSystemThreads();
    auto samplers = std::vector<std::unique_ptr<Sampler>>(numThreads);
    auto arenas   = std::vector<MemoryArena>(numThreads);

    Distribution1D lightDist = mis::calcLightPowerDistrib(scene);

    const int numPixels = width * height;
    const int numSamples = params.get<int>("NUM_SAMPLES");
    const int maxBounces = params.get<int>("MAX_BOUNCES");
    for (int i = 0; i < numSamples; i++) {
        // Prepare samplers
        if (i % numThreads == 0) {
            for (int t = 0; t < numThreads; t++) {
                auto seed = static_cast<unsigned int>(time(0) + t);
                samplers[t] = sampler_->clone(seed);
            }
        }

        std::mutex mtx;
        std::atomic<int> proc(0);
        parallel_for(0, numPixels, [&](int pid) {
            const int threadID = getThreadID();
            const int y = pid / width;
            const int x = pid % width;
            const Point2d randFilm = samplers[threadID]->get2D();

            auto cameraPath = std::make_unique<Vertex[]>(maxBounces + 2);
            auto lightPath  = std::make_unique<Vertex[]>(maxBounces + 1);
            const int nCamera = calcCameraSubpath(scene, *samplers[threadID], arenas[threadID],
                              maxBounces + 2, *camera_, Point2i(x, y), randFilm,
                              cameraPath.get());
            const int nLight = calcLightSubpath(scene, *samplers[threadID], arenas[threadID],
                             maxBounces + 1, lightDist, lightPath.get());


            Spectrum L(0.0);
            for (int cid = 1; cid <= nCamera; cid++) {
                for (int lid = 0; lid <= nLight; lid++) {
                    int depth = cid + lid - 2;
                    if ((cid == 1 && lid == 1) || (depth < 0) || (depth > maxBounces)) continue;

                    Point2d pFilm = Point2d(x + randFilm.x(), y + randFilm.y());
                    double misWeight = 0.0;

                    Spectrum Lpath = connectBDPT(scene, lightPath.get(), cameraPath.get(),
                        lid, cid, lightDist, *camera_, *samplers[threadID],
                        &pFilm, &misWeight);
                    if (cid == 1 && !Lpath.isBlack()) {
                        pFilm = Point2d(width - pFilm.x(), pFilm.y());
                        mtx.lock();
                        camera_->film()->addPixel(pFilm, Lpath);
                        mtx.unlock();
                    } else {
                        L += Lpath;
                    }
                }
            }
            camera_->film()->addPixel(Point2i(width - x - 1, y), randFilm, L);

            proc++;
            if (proc % 1000 == 0) {
                printf("%6.2f %% processed...\r", 100.0 * proc / numPixels);
                fflush(stdout);
            }
        });

        camera_->film()->saveMLT(1.0 / (i + 1), i + 1);

        for (int t = 0; t < numThreads; t++) {
            arenas[t].reset();
        }
    }
    std::cout << "Finish!!" << std::endl;    
}

}  // namespace spica
