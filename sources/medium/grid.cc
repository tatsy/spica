#define SPICA_API_EXPORT
#include "grid.h"

#include "../core/memory.h"
#include "../core/interaction.h"
#include "../bxdf/phase.h"
#include "../random/sampler.h"

namespace spica {

GridDensityMedium::GridDensityMedium(const Spectrum& sigmaAbsorb,
                                     const Spectrum& sigmaScatter,
                                     const double g, int nx, int ny, int nz,
                                     const Transform& mediumToWorld,
                                     const std::vector<double>& data)
    : sigmaAbsorb_{ sigmaAbsorb }
    , sigmaScatter_{ sigmaScatter }
    , g_{ g }
    , nx_{ nx }
    , ny_{ ny }
    , nz_{ nz }
    , worldToMedium_{ mediumToWorld.inverted() }
    , density_{ new double[nx * ny * nz] } {
    memcpy(density_.get(), &data[0], sizeof(double) * nx * ny * nz);
    sigmaExtinct_ = (sigmaAbsorb_ + sigmaScatter_)[0];
    if (Spectrum(sigmaExtinct_) != sigmaAbsorb_ + sigmaScatter_) {
        FatalError("GridDensityMedium only accepts spectrally uniform attenuation!!");
    }

    const int totalSize = nx * ny * nz;
    double maxDensity = 0.0;
    for (int i = 0; i < totalSize; i++) {
        maxDensity = std::max(maxDensity, density_[i]);
    }

    invMaxDensity_ = 1.0 / maxDensity;
}

double GridDensityMedium::density(const Point3d& p) const {
    Point3d pSamples(p.x() * nx_, p.y() * ny_, p.z() * nz_);
    Point3i pi(std::floor(pSamples.x()), std::floor(pSamples.y()), std::floor(pSamples.z()));
    Vector3d d = pSamples - Point3d(pi.x(), pi.y(), pi.z());

    double d00 = lerp(d.x(), D(pi), D(pi + Vector3i(1, 0, 0)));
    double d01 = lerp(d.x(), D(pi + Vector3i(0, 0, 1)), D(pi + Vector3i(1, 0, 1)));
    double d10 = lerp(d.x(), D(pi + Vector3i(0, 1, 0)), D(pi + Vector3i(1, 1, 0)));
    double d11 = lerp(d.x(), D(pi + Vector3i(0, 1, 1)), D(pi + Vector3i(1, 1, 1)));
    double d0 = lerp(d.y(), d00, d10);
    double d1 = lerp(d.y(), d01, d11);
    return lerp(d.z(), d0, d1);
}

double GridDensityMedium::D(const Point3i& p) const {
    if (p.x() < 0 || p.x() >= nx_ || p.y() < 0 || p.y() >= ny_ || p.z() < 0 || p.z() >= nz_) {
        return 0.0;
    }
    return density_[(p.z() * ny_ + p.y()) * nx_ * p.x()];
}

Spectrum GridDensityMedium::sample(const Ray& rWorld, Sampler& sampler,
                                   MemoryArena& arena,
                                   MediumInteraction* mi) const {
    Point3d  rOrg = worldToMedium_.apply(rWorld.org());
    Vector3d rDir = worldToMedium_.apply(rWorld.dir());
    Ray ray(rOrg, rDir.normalized(), rWorld.maxDist());

    Bounds3d b(Point3d(0.0, 0.0, 0.0), Point3d(1.0, 1.0, 1.0));
    double tMin, tMax;
    if (!b.intersect(ray, &tMin, &tMax)) {
        return Spectrum(1.0);
    }

    // Delta tracking iteration.
    double t = tMin;
    while (true) {
        t -= std::log(1.0 - sampler.get1D()) * invMaxDensity_ / sigmaExtinct_;
        if (t >= tMax) break;
        if (density(ray.proceeded(t)) * invMaxDensity_ > sampler.get1D()) {
            PhaseFunction* phase = arena.allocate<HenyeyGreenstein>(g_);
            *mi = MediumInteraction(rWorld.proceeded(t), -rWorld.dir(), this, phase);
            return sigmaScatter_ / sigmaExtinct_;
        }
    }
    return Spectrum(1.0);
}

Spectrum GridDensityMedium::Tr(const Ray& rWorld, Sampler& sampler) const {
    Point3d  rOrg = worldToMedium_.apply(rWorld.org());
    Vector3d rDir = worldToMedium_.apply(rWorld.dir());
    Ray ray(rOrg, rDir.normalized(), rWorld.maxDist());

    Bounds3d b(Point3d(0.0, 0.0, 0.0), Point3d(1.0, 1.0, 1.0));
    double tMin, tMax;
    if (!b.intersect(ray, &tMin, &tMax)) {
        return Spectrum(1.0);
    }

    double tr = 1.0, t = tMin;
    while (true) {
        t -= std::log(1.0 - sampler.get1D()) * invMaxDensity_ / sigmaExtinct_;
        if (t >= tMax) break;
        double dense = density(ray.proceeded(t));
        tr *= 1.0 - std::max(0.0, dense * invMaxDensity_);
    }
    return Spectrum(tr);
}

}  // namespace spica
