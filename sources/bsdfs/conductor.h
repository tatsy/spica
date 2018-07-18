#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_MIRROR_H_
#define _SPICA_MIRROR_H_

#include "core/core.hpp"
#include "core/common.h"
#include "core/spectrum.h"
#include "core/render.hpp"
#include "core/material.h"

namespace spica {

class SPICA_EXPORTS Conductor : public SurfaceMaterial {
public:
    // Public methods
    Conductor(const std::shared_ptr<Texture<Spectrum>> &Ks,
              const std::shared_ptr<Texture<Spectrum>> &eta,
              const std::shared_ptr<Texture<Spectrum>> &k,
              const std::shared_ptr<Texture<Spectrum>> &bump = nullptr);

    Conductor(RenderParams &params);

    void setScatterFuncs(SurfaceInteraction* intr,
                         MemoryArena& arena) const override;

private:
    // Private fields
    std::shared_ptr<Texture<Spectrum>> Ks_;
    std::shared_ptr<Texture<Spectrum>> eta_;
    std::shared_ptr<Texture<Spectrum>> k_;
    std::shared_ptr<Texture<Spectrum>> bumpMap_;
};

SPICA_EXPORT_PLUGIN(Conductor, "Conductor surface");

}  // namespace spica

#endif  // _SPICA_MIRROR_H_
