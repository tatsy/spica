#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_HOMOGENEOUS_H_
#define _SPICA_HOMOGENEOUS_H_

#include "core/common.h"
#include "core/cobject.h"
#include "core/medium.h"

namespace spica {

class SPICA_EXPORTS HomogeneousMedium : public Medium {
public:
    HomogeneousMedium(const Spectrum& sigmaAbsorb,
                      const Spectrum& sigmaScatter,
                      double scale, double g);

    explicit HomogeneousMedium(RenderParams &params);

    Spectrum Tr(const Ray& ray, Sampler& sampler) const override;
    Spectrum sample(const Ray& ray, Sampler& sampler, MemoryArena& arena,
                    MediumInteraction* mi) const override;
    Spectrum sigmaExt(const Point3d &pos) const override;
                    
private:
    const Spectrum sigmaAbsorb_, sigmaScatter_, sigmaExt_;
    const double g_;
};

SPICA_EXPORT_PLUGIN(HomogeneousMedium, "Homogeneous medium");

}  // namespace spica

#endif  // _SPICA_HOMOGENEOUS_H_
