#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_MIRROR_H_
#define _SPICA_MIRROR_H_

#include "core/core.hpp"
#include "core/common.h"
#include "core/spectrum.h"

#include "render/render.hpp"
#include "render/material.h"

namespace spica {

class SPICA_EXPORTS MirrorMaterial : public Material {
public:
    // Public methods
    MirrorMaterial(const std::shared_ptr<Texture<Spectrum>>& Kr,
                   const std::shared_ptr<Texture<double>>& bump = nullptr);

    void setScatterFuncs(SurfaceInteraction* intr,
                         MemoryArena& arena) const override;

private:
    // Private fields
    std::shared_ptr<Texture<Spectrum>> Kr_;
    std::shared_ptr<Texture<double>> bumpMap_;
};

}  // namespace spica

#endif  // _SPICA_MIRROR_H_
