#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_DIPOLE_H_
#define _SPICA_DIPOLE_H_

#include "core/common.h"
#include "core/spectrum.h"
#include "core/interpolation.h"
#include "core/render.hpp"
#include "core/material.h"

namespace spica {

class SPICA_EXPORTS Dipole : public SubsurfaceMaterial {
public:
    // Public methods
    Dipole(double scale,
           const std::shared_ptr<Texture<Spectrum>>& sigmaAbsorb,
           const std::shared_ptr<Texture<Spectrum>>& sigmaScatter,
           double g, double eta);

    explicit Dipole(RenderParams &params);

    void setScatterFuncs(SurfaceInteraction* isect,
                         MemoryArena& arena) const override;

private:
    // Private fields
    double scale_;
    std::shared_ptr<Texture<Spectrum>> sigmaAbsorb_, sigmaScatter_;
    double eta_;
    CatmullRom2D table_;

};

SPICA_EXPORT_PLUGIN(Dipole, "Dipole subsurface");

}  // namespace spica

#endif  // _SPICA_SUBSURFACE_H_
