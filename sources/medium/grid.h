#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_MEDIUM_GRID_H_
#define _SPICA_MEDIUM_GRID_H_

#include "../math/transform.h"

#include "medium.h"

namespace spica {

class SPICA_EXPORTS GridDensityMedium : public Medium {
public:
    // Public methods
    GridDensityMedium(const Spectrum& sigmaAbsorb,
                      const Spectrum& sigmaScatter, double g,
                      int nx, int ny, int nz, const Transform& mediumToWorld,
                      const std::vector<double>& data);

    double density(const Point3d& p) const;
    double D(const Point3i& p) const;

    Spectrum sample(const Ray& ray, Sampler& sampler, MemoryArena& arena,
                    MediumInteraction* mi) const override;

    Spectrum Tr(const Ray& ray, Sampler& sampler) const override;

private:
    // Private fields
    const Spectrum sigmaAbsorb_, sigmaScatter_;
    const double g_;
    const int nx_, ny_, nz_;
    const Transform worldToMedium_;
    std::unique_ptr<double[]> density_;
    double sigmaExtinct_;
    double invMaxDensity_;
};

}  // namespace spica

#endif  // _SPICA_MEDIUM_GRID_H_
