#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_DIELECTRIC_H_
#define _SPICA_DIELECTRIC_H_

#include <memory>

#include "core/common.h"
#include "core/spectrum.h"
#include "core/render.hpp"
#include "core/material.h"

namespace spica {

class SPICA_EXPORTS Dielectric : public SurfaceMaterial {
public:
    // Public methods
    Dielectric(const std::shared_ptr<Texture<Spectrum>>& Kr,
               const std::shared_ptr<Texture<Spectrum>>& Kt,
               const std::shared_ptr<Texture<Spectrum>>& index,
               const std::shared_ptr<Texture<Spectrum>>& bumpMap = nullptr);

    explicit Dielectric(RenderParams &params);

    void setScatterFuncs(SurfaceInteraction* isect,
                         MemoryArena& arena) const override;

private:
    // Private fields
    std::shared_ptr<Texture<Spectrum>> Kr_, Kt_;
    std::shared_ptr<Texture<Spectrum>> index_;
    std::shared_ptr<Texture<Spectrum>> bumpMap_;
};

SPICA_EXPORT_PLUGIN(Dielectric, "Dielectic surface");

}  // namespace spica

#endif  // _SPICA_DIELECTRIC_H_
