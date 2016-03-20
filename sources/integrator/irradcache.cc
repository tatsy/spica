#define SPICA_API_EXPORT
#include "irradcache.h"

#include "../core/memory.h"
#include "../core/parallel.h"
#include "../core/interaction.h"
#include "../core/sampling.h"

#include "../image/film.h"
#include "../scenes/scene.h"
#include "../camera/camera.h"
#include "../random/sampler.h"

#include "../bxdf/bsdf.h"
#include "../bxdf/bxdf.h"
#include "../bxdf/bssrdf.h"

#include "mis.h"
#include "render_parameters.h"


namespace spica {

struct CacheData {
    Point3d pos;
    Spectrum E;
    Normal3d nrm;
    double Ri;

    CacheData()
        : pos{}
        , E{}
        , nrm{}
        , Ri{} {
    }

    CacheData(const Point3d& p, const Spectrum& e, const Normal3d& n, double t)
        : pos{ p }
        , E{ e }
        , nrm{ n }
        , Ri{ t } {
    }

    double weight(const Point3d& p, const Normal3d& n) const {
        double d = (pos - p).norm();
        double epsilon = d / Ri + std::sqrt(1.0 - vect::dot(nrm, n));
        return epsilon != 0.0 ? 1.0 / epsilon : 0.0;
    }

    double operator[](int i) const {
        return pos[i];
    }
};

struct CacheQuery {
    Point3d pos;
    Normal3d nrm;
    double r2;
    double threshold;
    CacheQuery(const Point3d& p, const Normal3d& n, double r2_, double t)
        : pos{ p }
        , nrm{ n }
        , r2{ r2_ }
        , threshold{ t } {
    }
};

struct CacheNode {
    CacheData data;
    CacheNode* left  = nullptr;
    CacheNode* right = nullptr;
    int axis = 0;
};

class IrradCacheIntegrator::IrradCache {
public:
    IrradCache() {
    }

    ~IrradCache() {
        release();
        root_ = nullptr;
    }

    void release() {
        deleteNode(root_);
        root_ = nullptr;
    }

    void add(const CacheData& data) {
        root_ = addRec(root_, data, 0);
    }

    void search(const CacheQuery& query, std::vector<CacheData>* result) const {
        searchRec(root_, query, result);
    }

private:
    CacheNode* addRec(CacheNode* node, const CacheData& data, int axis) {
        if (node == nullptr) {
            CacheNode* newNode = new CacheNode;
            newNode->data = data;
            newNode->axis = axis;
            return newNode;
        } else {
            if (data[axis] < node->data[axis]) {
                node->left  = addRec(node->left, data, (axis + 1) % 3);
            } else {
                node->right = addRec(node->right, data, (axis + 1) % 3);
            }
            return node;
        }
    }

    void searchRec(CacheNode* node, const CacheQuery& query, std::vector<CacheData>* result) const {
        if (node == nullptr) {
            return;
        }

        const int axis = node->axis;
        double delta = query.pos[axis] - node->data.pos[axis];

        double weight = node->data.weight(query.pos, query.nrm);
        if (weight > query.threshold) {
            result->push_back(node->data);
        }

        if (delta < 0.0) {
            searchRec(node->left, query, result);
            if (delta * delta < query.r2) {
                searchRec(node->right, query, result);
            }
        } else {
            searchRec(node->right, query, result);
            if (delta * delta < query.r2) {
                searchRec(node->left, query, result);
            }
        }
    }

    void deleteNode(CacheNode* node) {
        if (node != nullptr) {
            if (node->left != nullptr) {
                deleteNode(node->left);
            }
            if (node->right != nullptr) {
                deleteNode(node->right);
            }
            delete node;
        }
    }

    CacheNode* root_ = nullptr;
};

IrradCacheIntegrator::IrradCacheIntegrator(
    const std::shared_ptr<const Camera>& camera,
    const std::shared_ptr<Sampler>& sampler)
    : SamplerIntegrator{ camera, sampler }
    , nGathering_{ 32 }
    , cacheMode_{ false }
    , cache_{ std::make_unique<IrradCache>() } {
}

IrradCacheIntegrator::~IrradCacheIntegrator() {
}

void IrradCacheIntegrator::initialize(const Scene& scene,
                                      const RenderParameters& params,
                                      Sampler& sampler) {
}

void IrradCacheIntegrator::loopStarted(const Scene& scene,
                                       const RenderParameters& params,
                                       Sampler& sampler) {
    // Prepare samplers and memory arenas
    const int nThreads = numSystemThreads();
    std::vector<std::shared_ptr<Sampler>> samplers(nThreads);
    for (int i = 0; i < nThreads; i++) {
        samplers[i] = sampler.clone((unsigned int)time(0) + i);
    }
    std::vector<MemoryArena> arenas(nThreads);

    // Prebuild irradiance cache
    cacheMode_ = true;
    parallel_for (0, 5000, [&](int i) {
        const int threadID = getThreadID();
        int px = camera_->film()->resolution().x() * sampler.get1D();
        int py = camera_->film()->resolution().y() * sampler.get1D();
        double pdfPos, pdfDir;
        Ray ray = camera_->spawnRay(Point2i(px, py), samplers[threadID]->get2D(),
                                    samplers[threadID]->get2D(), &pdfPos, &pdfDir);

        Li(scene, params, ray, *samplers[threadID], arenas[threadID]);
    });
    cacheMode_ = false;
}

void IrradCacheIntegrator::loopFinished(const Scene& scene,
                                        const RenderParameters& params,
                                        Sampler& sampler) {
    cache_->release();
}

Spectrum IrradCacheIntegrator::Li(const Scene& scene,
                                  const RenderParameters& params,
                                  const Ray& r,
                                  Sampler& sampler,
                                  MemoryArena& arena,
                                  int depth) const {
    Ray ray(r);
    Spectrum L(0.0);
    Spectrum beta(1.0);
    bool specularBounce = false;
    int bounces;
    for (bounces = 0; ; bounces++) {
        SurfaceInteraction isect;
        bool isIntersect = scene.intersect(ray, &isect);

        // Sample Le which contributes without any loss
        if (depth >= 0 && (bounces == 0 || specularBounce)) {
            if (isIntersect) {
                L += beta * isect.Le(-ray.dir());
            } else {
                for (const auto& light : scene.lights()) {
                    L += beta * light->Le(ray);
                }
            }
        }

        if (!isIntersect || bounces >= params.bounceLimit()) break;

        isect.setScatterFuncs(ray, arena);
        if (!isect.bsdf()) {
            ray = isect.spawnRay(ray.dir());
            bounces--;
            continue;
        }

        if (isect.bsdf()->numComponents(BxDFType::All & (~BxDFType::Specular)) > 0) {
            Spectrum Ld = beta * mis::uniformSampleOneLight(isect, scene, arena, sampler);
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

        beta *= ref * vect::absDot(wi, isect.normal()) / pdf;
        specularBounce = (sampledType & BxDFType::Specular) != BxDFType::None;
        ray = isect.spawnRay(wi);

        if ((sampledType & BxDFType::Diffuse) != BxDFType::None &&
            (sampledType & BxDFType::Reflection) != BxDFType::None && depth >= 0) {

            const double threshold = 50.0;
            const double Rmax = 100.0;
            const double radius = (1.0 / threshold) * Rmax;

            CacheQuery query(isect.pos(), isect.normal(), radius * radius, threshold);
            std::vector<CacheData> results;
            cache_->search(query, &results);

            Spectrum E(0.0);
            if (results.empty() || cacheMode_) {
                double Ri = 0.0;
                for (int k = 0; k < nGathering_; k++) {
                    Vector3d n(isect.normal());
                    Vector3d u, v;
                    vect::coordinateSystem(n, &u, &v);
                    Vector3d dir = sampleCosineHemisphere(sampler.get2D());
                    double pdfDir = cosineHemispherePdf(vect::cosTheta(dir));
                    if (pdfDir == 0.0) continue;

                    dir = u * dir.x() + v * dir.y() + n * dir.z();
                    Ray ray(isect.pos(), dir);
                    SurfaceInteraction isect;
                    if (scene.intersect(ray, &isect)) {
                        Ri += 1.0 / (isect.pos() - ray.org()).norm();
                        E  += Li(scene, params, ray, sampler, arena, -1) *
                              vect::dot(n, dir) / pdfDir;                        
                    }
                }
                E  = E / nGathering_;
                Ri = 1.0 / (Ri / nGathering_);
                cache_->add(CacheData(isect.pos(), E, isect.normal(), Ri));
            } else {
                double sumWgt = 0.0;
                for (const auto& irr : results) {
                    double weight = irr.weight(isect.pos(), isect.normal());
                    E      += weight * irr.E;
                    sumWgt += weight;
                }

                if (sumWgt != 0.0) {
                    E = E / sumWgt;
                }
            }
            L += beta * E / PI;
            break;
        }

        // Account for BSSRDF
        if (isect.bssrdf() && (sampledType & BxDFType::Transmission) != BxDFType::None) {
            SurfaceInteraction pi;
            Spectrum S = isect.bssrdf()->sample(scene, sampler.get1D(),
                sampler.get2D(), arena, &pi, &pdf);

            if (S.isBlack() || pdf == 0.0) break;
            beta *= S / pdf;

            L += beta * mis::uniformSampleOneLight(pi, scene, arena, sampler);

            Spectrum f = pi.bsdf()->sample(pi.wo(), &wi, sampler.get2D(), &pdf,
                                           BxDFType::All, &sampledType);
            if (f.isBlack() || pdf == 0.0) break;
            beta *= f * vect::absDot(wi, pi.normal()) / pdf;

            specularBounce = (sampledType & BxDFType::Specular) != BxDFType::None;
            ray = pi.spawnRay(wi);
        }

        // Russian roulette
        if (bounces > 3) {
            double continueProbability = std::min(0.95, beta.luminance());
            if (sampler.get1D() > continueProbability) break;
            beta /= continueProbability;
        }
    }
    return L;
}

}  // namespace spica
