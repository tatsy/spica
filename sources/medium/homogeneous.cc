#define SPICA_API_EXPORT
#include "homogeneous.h"

#include "../core/memory.h"
#include "../core/ray.h"
#include "../core/interaction.h"
#include "../scenes/scene.h"
#include "../random/sampler.h"
#include "../bxdf/phase.h"

namespace spica {

HomogeneousMedium::HomogeneousMedium(const Spectrum& sigmaAbsorb,
                                     const Spectrum& sigmaScatter, double g)
    : sigmaAbsorb_{ sigmaAbsorb }
    , sigmaScatter_{ sigmaScatter }
    , sigmaExt_{ sigmaAbsorb_ + sigmaScatter_ }
    , g_{ g } {
}

Spectrum HomogeneousMedium::Tr(const Ray& ray, Sampler& smapler) const {
    return Spectrum::exp(-sigmaExt_ *
           std::min(ray.maxDist() * ray.dir().norm(), INFTY));
}

Spectrum HomogeneousMedium::sample(const Ray& ray, Sampler& sampler,
                                   MemoryArena& arena,
                                   MediumInteraction* mi) const {
    int channel = std::min((int)(sampler.get1D() * Spectrum::channels),
                           Spectrum::channels - 1);
    double dist = -std::log(1.0 - sampler.get1D()) / sigmaExt_[channel];
    double t    = std::min(dist * ray.dir().norm(), ray.maxDist());

    bool sampleMedium = t < ray.maxDist();

    if (sampleMedium) {
        Point3d p = ray.org() + t * ray.dir();        
        PhaseFunction* phase = arena.allocate<HenyeyGreenstein>(g_);
        *mi = MediumInteraction(p, -ray.dir(), this, phase);    
    }

    Spectrum tr = Spectrum::exp(-sigmaExt_ * std::min(t, INFTY) * ray.dir().norm());

    Spectrum density = sampleMedium ? (sigmaExt_ * tr) : tr;
    double pdf = 0.0;
    for (int i = 0; i < Spectrum::channels; i++) {
        pdf += density[i];
    }

    pdf *= 1.0 / Spectrum::channels;
    return sampleMedium ? (tr * sigmaScatter_ / pdf) : (tr / pdf);
}

}  // namespace spica
