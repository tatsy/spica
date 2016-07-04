#define SPICA_API_EXPORT
#include "pssmlt.h"

#include <atomic>
#include <mutex>

#include "mis.h"

#include "../core/memory.h"
#include "../core/parallel.h"
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
        : modifyTime_{ 0 }
        , value_{ 0.0 } {
    }

    Sample(int modifyTime, double value)
        : modifyTime_{ modifyTime }
        , value_{ value } {
    }

    Sample(const Sample&) = default;
    Sample& operator=(const Sample&) = default;
    
    void mutate(const Point2d& rand) {
        static const double s1 = 1.0 / 1024.0;
        static const double s2 = 1.0 / 64.0;

        const double dv = s2 * std::exp(-std::log(s2 / s1) * rand[0]);
        if (rand[1] < 0.5) {
            value_ += dv;
        } else {
            value_ -= dv;
        }

        value_ -= std::floor(value_);
        modifyTime_++;
    }

    inline int modifyTime() const { return modifyTime_; }
    inline double value() const { return value_; }

private:
    // Private fields
    int modifyTime_;
    double value_;
};

}  // anonymous namespace

/**
 * Path sample.
 */
class PSSMLTIntegrator::PathSample {
public:
    PathSample()
        : pixel_{ 0.0, 0.0 }
        , Li_{} {
    }

    PathSample(const Point2d& pixel, const Spectrum& Li)
        : pixel_{ pixel }
        , Li_{ Li } {
    }

    PathSample(const PathSample&) = default;
    PathSample& operator=(const PathSample&) = default;

    inline Point2d pixel() const { return pixel_; }
    inline Point2i pixel2i() const { return Point2i((int)pixel_.x(), (int)pixel_.y()); }
    inline Point2d pixelDec() const { return Point2d(pixel_.x() - (int)pixel_.x(), pixel_.y() - (int)pixel_.y()); }
    inline const Spectrum& Li() const { return Li_; }
 
private:
    Point2d pixel_;
    Spectrum Li_;
};

/**
 * Management class for PSS sampling.
 */
class PSSSampler : public Sampler {
public:
    PSSSampler(const std::shared_ptr<Sampler>& rand, double pLarge)
        : rand_{ rand }
        , pLarge_{ pLarge } {
        static const int size = 128;
        currentSamples_.resize(size);
        previousSamples_.resize(size);
        for (int i = 0; i < size; i++) {
            currentSamples_[i] = Sample(0, rand_->get1D());
            previousSamples_[i] = Sample(0, rand_->get1D());
        }
    }

    double get1D() override {
        if (currentCoordIndex_ >= currentSamples_.size()) {
            const int size = currentSamples_.size();
            currentSamples_.resize(size * 2);
            previousSamples_.resize(size * 2);
            for (int i = size; i < size * 2; i++) {
                currentSamples_[i] = Sample(0, rand_->get1D());
                previousSamples_[i] = Sample(0, rand_->get1D());
            }
        }
        
        if (currentSamples_[currentCoordIndex_].modifyTime() < globalTime_) {
            if (largeStep_ != 0) {
                // Large step
                previousSamples_[currentCoordIndex_] = currentSamples_[currentCoordIndex_];
                currentSamples_[currentCoordIndex_] = Sample(globalTime_, rand_->get1D());
            } else {
                // Small step
                if (currentSamples_[currentCoordIndex_].modifyTime() < largeStepTime_) {
                    previousSamples_[currentCoordIndex_] = currentSamples_[currentCoordIndex_];
                    currentSamples_[currentCoordIndex_] = Sample(largeStepTime_, rand_->get1D());
                }
                
                while (currentSamples_[currentCoordIndex_].modifyTime() < globalTime_ - 1) {
                    currentSamples_[currentCoordIndex_].mutate(rand_->get2D());
                }
                previousSamples_[currentCoordIndex_] = currentSamples_[currentCoordIndex_];
                currentSamples_[currentCoordIndex_].mutate(rand_->get2D());
            }
        }
        
        return currentSamples_[currentCoordIndex_++].value();
    }

    bool startNextSample() override {
        currentCoordIndex_ = 0;
        if (rand_->get1D() < pLarge_) {
            largeStep_ = 1;
        } else {
            largeStep_ = 0;
        }
        return true;
    }

    std::unique_ptr<Sampler> clone(unsigned int seed = 0) const override {
        return std::make_unique<PSSSampler>(rand_->clone(seed), pLarge_);
    }

    void accept() {
        if (largeStep_ != 0) {
            largeStepTime_ = globalTime_;
        }
        currentCoordIndex_ = 0;
        globalTime_++;
    }

    void reject() {
        for (int p = 0; p < currentCoordIndex_; p++) {
            currentSamples_[p] = previousSamples_[p];
        }
        currentCoordIndex_ = 0;
    }

    inline int globalTime() const { return globalTime_; }
    inline int largeStep() const { return largeStep_;}
    inline int largeStepTime() const { return largeStepTime_; }
    inline double pLarge() const { return pLarge_; }

private:
    int globalTime_ = 0;
    int largeStep_ = 0;
    int largeStepTime_ = 0;
    int currentCoordIndex_ = 0;
    
    std::shared_ptr<Sampler> rand_;
    double pLarge_;
    
    std::vector<Sample> currentSamples_;
    std::vector<Sample> previousSamples_;
};

PSSMLTIntegrator::PSSMLTIntegrator(const std::shared_ptr<const Camera>& camera)
    : Integrator{ camera } {
}

PSSMLTIntegrator::~PSSMLTIntegrator() {
}

void PSSMLTIntegrator::render(const Scene& scene, const RenderParams& params) {
    // Take parameters.
    const double pLarge  = params.get<double>("PSSMLT_P_LARGE", 0.5);
    const int    nMutate = params.get<int>("MLT_NUM_MUTATE", 100000);
    const int    nTrial  = params.get<int>("NUM_SAMPLES");
    const int    initialTrials = params.get<int>("MLT_NUM_INIT_TRIALS", 2000);
    
    // Pure random generator.
    auto rand = std::make_shared<Random>(0);

    // Start rendering.
    const double scrnArea = camera_->film()->resolution().x() * camera_->film()->resolution().y();

    const int nThreads = numSystemThreads();
    const int nLoop = (nTrial + nThreads - 1) / nThreads;
    
    int progress = 0;
    for (int loop = 0; loop < nLoop; loop++) {
        std::mutex mtx;
        const int nTasks = std::min(nThreads, nTrial - nThreads * loop);
        std::atomic<long long> nAccept(0ll);
        std::atomic<long long> nTotal(0ll);
        parallel_for (0, nThreads, [&](int t) {
            MemoryArena arena;
        
            // Generate initial paths (burn-in step).
            PathSample currentSample;
            std::shared_ptr<PSSSampler> psSampler = nullptr;
            double sumI = 0.0;
            while (sumI == 0.0) {
                psSampler = std::make_shared<PSSSampler>(rand->clone((unsigned int)time(0)), pLarge);
                for (int i = 0; i < initialTrials; i++) {
                    psSampler->startNextSample();
                    currentSample = generateSample(scene, params, *psSampler, arena);
                    sumI += currentSample.Li().luminance();
                }
            }

            // Mutation.
            const int M = nMutate;
            const double b = sumI / initialTrials;
            for (int i = 0; i < nMutate; i++) {
                psSampler->startNextSample();
                PathSample nextSample = generateSample(scene, params, *psSampler, arena);

                double acceptRatio = currentSample.Li().isBlack() ? 1.0
                                                                  : nextSample.Li().luminance() / currentSample.Li().luminance();
                acceptRatio = std::min(1.0, acceptRatio);

                // Update image.
                mtx.lock();
                {
                    double currentWeight = (1.0 - acceptRatio) /
                                           ((currentSample.Li().luminance() / b + psSampler->pLarge()) * M);
                    double nextWeight    = (acceptRatio + psSampler->largeStep()) /
                                           ((nextSample.Li().luminance() / b + psSampler->pLarge()) * M);

                    camera_->film()->addPixel(currentSample.pixel2i(), currentSample.pixelDec(),
                                              currentWeight * currentSample.Li());
                    camera_->film()->addPixel(nextSample.pixel2i(), nextSample.pixelDec(),
                                              nextWeight * nextSample.Li());
                }
                mtx.unlock();

                // Update sample.
                if (rand->get1D() < acceptRatio) {
                    nAccept++;              
                    currentSample = nextSample;
                    psSampler->accept();
                } else {
                    psSampler->reject();            
                }
                nTotal++;

                if (nTotal % 1000 == 0) {
                    const double ratio = 100.0 * (nTotal + 1) / (nThreads * nMutate);
                    printf("%6.2f %% processed...\r", ratio);
                    fflush(stdout);
                }

                // Reset memory arena.
                arena.reset();
            }
        });

        // Report accept / reject ratio.
        const double ratio = 100.0 * nAccept / nTotal;
        printf("[ %5.2f %% ] ", ratio);
        std::cout << nAccept << " / " << nTotal << std::endl;
        fflush(stdout);

        // Save image.
        progress += nTasks;
        camera_->film()->saveMLT(scrnArea / progress, progress);
    }
}

PSSMLTIntegrator::PathSample PSSMLTIntegrator::generateSample(const Scene& scene,
                                                              const RenderParams& params,
                                                              Sampler& sampler,
                                                              MemoryArena& arena) const {
    const int width  = camera_->film()->resolution().x();
    const int height = camera_->film()->resolution().y();

    const double fx = std::min(sampler.get1D() * width, (double)(width - EPS));
    const double fy = std::min(sampler.get1D() * height, (double)(height - EPS));
    const int x = std::min((int)fx, width - 1);
    const int y = std::min((int)fy, height - 1);
    const Point2d randLens = sampler.get2D();
    const Point2d randFilm(fx - x, fy - y);
    const Ray ray = camera_->spawnRay(Point2i(x, y), randFilm, randLens);

    Spectrum L = Li(scene, params, ray, sampler, arena);
    return PathSample(Point2d(fx, fy), L);
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
    
    const int maxBounces = params.get<int>("MAX_BOUNCES");
    for (int bounces = 0; bounces < maxBounces; bounces++) {
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

        if (!isIntersect || bounces >= maxBounces) break;

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
