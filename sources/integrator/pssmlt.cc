#define SPICA_API_EXPORT
#include "pssmlt.h"

#include "mis.h"

#include "../core/renderparams.h"
#include "../core/interaction.h"

#include "../scenes/scene.h"
#include "../camera/spica_camera.h"
#include "../image/film.h"

#include "../random/sampler.h"
#include "../random/random.h"

#include "../bxdf/bsdf.h"
#include "../bxdf/bssrdf.h"

namespace spica {

namespace {

/**
 * The sample object for primary space sampling.
 */
class Sample {
public:
    // Public methods
    Sample()
        : index_{ 0 }
        , value_{ 0.0 } {
    }

    Sample(int index, double value)
        : index_{ index }
        , value_{ value } {
    }

    Sample(const Sample&) = default;
    Sample& operator=(const Sample&) = default;

    inline int index() const { return index_; }
    inline double value() const { return value_; }

private:
    // Private fields
    int index_;
    double value_;
};

/**
 * Path sample.
 */
class PathSample {
public:
    PathSample()
        : x_{ 0 }
        , y_{ 0 }
        , Li_{}
        , weight_{ 0.0 } {
    }

    PathSample(int x, int y, const Spectrum& Li, double weight)
        : x_{ x }
        , y_{ y }
        , Li_{ Li }
        , weight_{ weight } {
    }

    PathSample(const PathSample&) = default;
    PathSample& operator=(const PathSample&) = default;

    inline int x() const { return x_; }
    inline int y() const { return y_; }
    inline Point2i pixel() const { return Point2i(x_, y_); }
    inline const Spectrum& Li() const { return Li_; }
    inline double weight() const { return weight_; }

private:
    int x_;
    int y_;
    Spectrum Li_;
    double weight_;
};

}  // anonymous namespace

/**
 * Management class for PSS sampling.
 */
class PSSMLTIntegrator::PSSSampler : public spica::Sampler {
public:
    PSSSampler(double pLarge)
        : pLarge_{ pLarge } {
    }

    double get1D() override {
    }

    bool startNextSample() override {
        return true;
    }

    std::unique_ptr<Sampler> clone(unsigned int seed = 0) const {
    }

    void accept() {
        // TODO:
    }

    void reject() {
        // TODO:
    }

    // TODO: some inline functions should be added.

private:
    int globalTime_ = 0;
    int largeStep_ = 0;
    int largeStepTime_ = 0;
    int currentCoordIndex_ = 0;
    double pLarge_ = 0.0;
};

PSSMLTIntegrator::PSSMLTIntegrator(const std::shared_ptr<const Camera>& camera)
    : Integrator{ camera } {
}

PSSMLTIntegrator::~PSSMLTIntegrator() {
}

void PSSMLTIntegrator::render(const Scene& scene, const RenderParams& params) {
    const int width  = camera_->film()->resolution().x();
    const int height = camera_->film()->resolution().y();

    // Take parameters.
    const double pLarge  = params.get<double>("PSSMLT_P_LARGE", 0.3);
    const int    nMutate = params.get<int>("MLT_NUM_MUTATE", 64);
    const int    nTrial  = params.get<int>("NUM_SAMPLES");
    const int    initialTrials = params.get<int>("MLT_NUM_INIT_TRIALS", width * height);

    // Pure random generator.
    auto rng = std::make_shared<Random>((unsigned int)time(0));

    auto psSampler = std::make_shared<PSSSampler>((unsigned int)time(0));

    int nAccept = 0;
    for (int t = 0; t < nTrial; t++) {
        // Generate initial paths.
        psSampler->startNextSample();
        PathSample pathSample = calcPathSample();

        // Mutation.
        Image buffer(width, height);
        for (int i = 0; i < nMutate; i++) {
            psSampler->startNextSample();
            PathSample nextSample = calcPathSample();

            double accept = nextSample.Li().luminance() / pathSample.Li().luminance();
            accept = std::min(1.0, accept);

            // TODO: Weight handing.

            Point2d randFilm;
            camera_->film()->addPixel(nextSample.pixel(), randFilm, nextSample.weight() * nextSample.Li());

            if (rng->get1D() < accept) {
                nAccept++;              
                pathSample = nextSample;
                psSampler->accept();
            } else {
                psSampler->reject();            
            }
        }

        camera_->film()->save(t + 1);
    }
}

PathSample PSSMLTIntegrator::calcPathSample() const {
}

Spectrum PSSMLTIntegrator::Li(const Scene& scene,
                              const RenderParams& params,
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
        if (bounces == 0 || specularBounce) {
            if (isIntersect) {
                L += beta * isect.Le(-ray.dir());
            } else {
                for (const auto& light : scene.lights()) {
                    L += beta * light->Le(ray);
                }
            }
        }

        if (!isIntersect || bounces >= params.get<int>("MAX_BOUNCES")) break;

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
